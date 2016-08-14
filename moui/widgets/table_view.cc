// Copyright (c) 2016 Ollix. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ---
// Author: olliwang@ollix.com (Olli Wang)

#include "moui/widgets/table_view.h"

#include <algorithm>
#include <map>
#include <string>
#include <queue>
#include <vector>

#include "moui/base.h"
#include "moui/core/clock.h"
#include "moui/core/event.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/scroll_view.h"
#include "moui/widgets/table_view_cell.h"
#include "moui/widgets/widget_view.h"

namespace {

// The default height in points between sections.
const float kDefaultHeightBetweenSections = 35;
// The default row height in points.
const float kDefaultRowHeight = 44;

}  // namespace

namespace moui {

TableView::TableView() :
    ScrollView(), data_source_(nullptr), delegate_(nullptr),
    down_event_cell_(nullptr),
    height_between_sections_(kDefaultHeightBetweenSections),
    last_bottommost_content_view_offset_(-1),
    last_topmost_content_view_offset_(-1),
    row_height_(TableView::kAutomaticDimenstion),
    should_update_layout_(true),
    separator_color_(nvgRGB(234, 234, 234)),
    separator_insets_({0, 20, 0, 20}),
    table_footer_view_(nullptr),
    table_header_view_(nullptr) {
  set_always_bounce_vertical(true);
  set_always_scroll_both_directions(false);
  set_background_color(nvgRGBA(240, 239, 245, 255));

  layout_view_ = new moui::Widget();
  layout_view_->set_is_opaque(false);
  layout_view_->BindRenderFunction(&TableView::RenderLayoutView, this);
  layout_view_->SetWidth(Widget::Unit::kPercent, 100);
  layout_view_->SetHeight(Widget::Unit::kPercent, 100);
  InsertChildAboveContentView(layout_view_);
}

TableView::~TableView() {
  // Releases visible cells.
  for (TableViewCell* cell : visible_cells_) {
    cell->RemoveFromParent();
    delete cell;
  }
  // Releases reusable cells.
  for (auto iterator = reusable_cells_.begin();
       iterator != reusable_cells_.end();
       ++iterator) {
    std::queue<TableViewCell*>* table_view_cells = &(iterator->second);
    while (!table_view_cells->empty()) {
      TableViewCell* cell = table_view_cells->front();
      table_view_cells->pop();
      delete cell;
    }
  }
  // Releases managed widgets.
  delete layout_view_;
}

void TableView::DeselectRow(const CellIndex cell_index) {
  if (cell_index.section_index < 0 || cell_index.row_index < 0) {
    return;
  }

  if (delegate_ != nullptr &&
      !delegate_->TableViewShouldDeselectRow(this, cell_index)) {
    return;
  }

  auto iterator = cell_indexes_for_selected_rows_.begin();
  for (; iterator != cell_indexes_for_selected_rows_.end(); ++iterator) {
    const CellIndex kCachedCellIndex = static_cast<CellIndex>(*iterator);
    if (cell_index.section_index == kCachedCellIndex.section_index &&
        cell_index.row_index == kCachedCellIndex.row_index) {
      break;
    }
  }  // end of `cell_indexes_for_selected_rows_` iterator.

  // Updates the sates of the corresopnded cell.
  TableViewCell* cell = GetCell(cell_index);
  if (cell != nullptr) {
    cell->set_selected(false);
  }

  // Removes the cell index from `cell_indexes_for_selected_rows_`.
  if (iterator != cell_indexes_for_selected_rows_.end()) {
    cell_indexes_for_selected_rows_.erase(iterator);
    if (delegate_ != nullptr)
      delegate_->TableViewDidDeselectRow(this, cell_index);
  }
}

TableViewCell* TableView::DequeueReusableCell(const std::string identifier) {
  auto queue_match = reusable_cells_.find(identifier);
  if (queue_match == reusable_cells_.end())
    return nullptr;

  auto reusable_cells = &(queue_match->second);
  if (reusable_cells->empty())
    return nullptr;

  TableViewCell* cell = reusable_cells->front();
  reusable_cells->pop();
  cell->PrepareForReuse();
  return cell;
}

TableViewCell* TableView::GetCell(const CellIndex cell_index) const {
  int vector_index = 0;
  for (CellIndex visible_cell_index : cell_indexes_for_visible_rows_) {
    if (visible_cell_index.section_index == cell_index.section_index &&
        visible_cell_index.row_index == cell_index.row_index) {
      return visible_cells_[vector_index];
    }
    ++vector_index;
  }
  return nullptr;
}

TableView::CellIndex TableView::GetCellIndex(TableViewCell* cell) const {
  int vector_index = 0;
  for (TableViewCell* visible_cell : visible_cells_) {
    if (visible_cell == cell)
      return cell_indexes_for_visible_rows_[vector_index];
    ++vector_index;
  }
  return {-1, -1};
}

float TableView::GetRowHeight(const CellIndex cell_index) {
  float row_height = TableView::kAutomaticDimenstion;
  if (delegate_ != nullptr) {
    row_height = delegate_->GetTableViewRowHeight(this, cell_index);
  }
  if (row_height == TableView::kAutomaticDimenstion)
    row_height = row_height_;
  if (row_height == TableView::kAutomaticDimenstion)
    row_height = kDefaultRowHeight;
  return row_height;
}

bool TableView::HandleEvent(Event* event) {
  const bool kResult = ScrollView::HandleEvent(event);
  if (down_event_cell_ == nullptr)
    return kResult;

  const Point location = static_cast<Point>(event->locations()->front());
  if (event->type() == Event::Type::kDown) {
    down_event_location_ = location;
    // Delay highlights the `down_event_cell_`.
    Clock::ExecuteCallbackOnMainThread(
        0.01,  // delay in seconds
        std::bind(&TableView::HighlightDownEventCell, this));
  } else if (event->type() == Event::Type::kUp) {
    SetCellHighlighted(down_event_cell_, false);
    // Toggles the selected state of the cell.
    const CellIndex kCellIndex = GetCellIndex(down_event_cell_);
    if (!down_event_cell_->selected())
      SelectRow(kCellIndex);
    else
      DeselectRow(kCellIndex);
    down_event_cell_ = nullptr;
  } else if (event->type() == Event::Type::kMove &&
             (location.x != down_event_location_.x ||
              location.y != down_event_location_.y)) {
    SetCellHighlighted(down_event_cell_, false);
    down_event_cell_ = nullptr;
  } else if (event->type() == Event::Type::kCancel) {
    SetCellHighlighted(down_event_cell_, false);
    down_event_cell_ = nullptr;
  }
  return kResult;
}

void TableView::HighlightDownEventCell() {
  if (down_event_cell_ == nullptr)
    return;
  SetCellHighlighted(down_event_cell_, true);
}

void TableView::ReloadData() {
  down_event_cell_ = nullptr;
  for (TableViewCell* cell : visible_cells_) {
    cell->RemoveFromParent();
    ReuseCell(cell);
  }
  visible_cells_.clear();
  cell_indexes_for_visible_rows_.clear();
  cell_indexes_for_selected_rows_.clear();
  should_update_layout_ = true;
  SetContentViewOffset({0, 0});
  layout_view_->Redraw();
}

void TableView::ReuseCell(TableViewCell* cell) {
  cell->RemoveFromParent();
  const std::string kReuseIdentifier = cell->reuse_identifier();
  if (kReuseIdentifier.empty()) {
    delete cell;
    return;
  }

  std::queue<TableViewCell*>* reusable_cells = \
      &(reusable_cells_[kReuseIdentifier]);
  reusable_cells->push(cell);
}

void TableView::ReuseVisibleCells(const int begin, const int last) {
  if (begin >= last)
    return;

  for (auto iterator = visible_cells_.begin() + begin;
       iterator < visible_cells_.begin() + last;
       ++iterator) {
    auto cell = reinterpret_cast<TableViewCell*>(*iterator);
    ReuseCell(cell);
  }
  visible_cells_.erase(visible_cells_.begin() + begin,
                       visible_cells_.begin() + last);
  cell_indexes_for_visible_rows_.erase(
      cell_indexes_for_visible_rows_.begin() + begin,
      cell_indexes_for_visible_rows_.begin() + last);
}

void TableView::RenderLayoutView(moui::Widget* widget, NVGcontext* context) {
  if (data_source_ == nullptr)
    return;

  const float kCellWidth = GetWidth();
  const Point kContentViewOffset = GetContentViewOffset();

  int current_section = -1;
  int last_row_of_the_current_section = -1;
  int vector_index = 0;

  nvgBeginPath(context);
  for (CellIndex& cell_index : cell_indexes_for_visible_rows_) {
    if (cell_index.section_index != current_section) {
      current_section = cell_index.section_index;
      last_row_of_the_current_section = \
          data_source_->GetNumberOfRowsInSection(this, current_section) - 1;
    }

    TableViewCell* cell = visible_cells_[vector_index];
    const float kCellTopOffset = cell->GetY() - kContentViewOffset.y;

    // Draws the top separator.
    if (cell_index.row_index == 0) {
      nvgRect(context, 0, kCellTopOffset, kCellWidth, 1);
    }

    // Draws the bottom separator.
    const float kBttomSeparatorLeftOffset = \
        cell_index.row_index == last_row_of_the_current_section ?
        0 :
        separator_insets_.left;
    const float kBttomSeparatorRightOffset = \
        cell_index.row_index == last_row_of_the_current_section ?
        kCellWidth :
        kCellWidth - separator_insets_.right;
    const float kBottomSeparatorVerticalOffset = \
        kCellTopOffset + cell->GetHeight() - 1;
    nvgRect(context, kBttomSeparatorLeftOffset, kBottomSeparatorVerticalOffset,
            kBttomSeparatorRightOffset - kBttomSeparatorLeftOffset + 1, 1);
    ++vector_index;
  }
  nvgFillColor(context, separator_color_);
  nvgFill(context);
}

void TableView::ScrollToCellIndex(const CellIndex cell_index,
                                  const ScrollPosition scroll_position,
                                  const bool animating) {
  if (!ValidateCellIndex(cell_index))
    return;

  // Determines the offset and height of the cell of interest.
  const float kContentViewOffset = GetContentViewOffset().y;
  float cell_offset = -1;
  float cell_height = -1;

  TableViewCell* cell = GetCell(cell_index);
  if (cell != nullptr) {
    cell_offset = cell->GetY();
    cell_height = cell->GetHeight();
  } else {
    float content_view_offset = -1;
    bool finished = false;

    // Table header view.
    if (table_header_view_ != nullptr) {
      content_view_offset = table_header_view_->GetHeight();
    }

    const int kNumberOfSections = data_source_->GetNumberOfSections(this);
    for (int section_index = 0;
         section_index < kNumberOfSections;
         ++section_index) {
      const int kNumberOfRows = data_source_->GetNumberOfRowsInSection(
          this, section_index);
      if (kNumberOfRows == 0) {
        cell_height = 0;
        cell_offset = std::max(0.0f, content_view_offset);
        break;
      }
      if (section_index > 0) {
        content_view_offset += height_between_sections_;
      }

      // Section header.
      if (delegate_ != nullptr) {
        const float kHeaderHeight = delegate_->GetTableViewSectionHeaderHeight(
            this, section_index);
        content_view_offset = content_view_offset < 0 ?
                              kHeaderHeight :
                              content_view_offset + kHeaderHeight;
      }

      // Rows.
      for (int row_index = 0; row_index < kNumberOfRows; ++row_index) {
        cell_offset = content_view_offset < 0 ? 0 : content_view_offset - 1;
        cell_height = GetRowHeight({section_index, row_index});
        if (section_index == cell_index.section_index &&
            row_index == cell_index.row_index) {
          finished = true;
          break;
        }
        content_view_offset = cell_offset + cell_height;
      }
      if (finished)
        break;

      // Section footer.
      if (delegate_ != nullptr) {
        const float kFooterHeight = delegate_->GetTableViewSectionFooterHeight(
            this, section_index);
        content_view_offset += kFooterHeight;
      }
    }
  }
  if (cell_offset < 0 || cell_height < 0)
    return;

  const float kCellBottomOffset = cell_offset + cell_height - 1;
  const float kContentViewBottomOffset = kContentViewOffset + GetHeight() - 1;
  if (scroll_position == ScrollPosition::kNone &&
      cell_offset >= kContentViewOffset &&
      kCellBottomOffset <= kContentViewBottomOffset) {
    return;
  }

  float new_content_view_offset = 0;
  if (scroll_position == ScrollPosition::kTop ||
      (scroll_position == ScrollPosition::kNone &&
       cell_offset < kContentViewOffset)) {
    new_content_view_offset = cell_offset;
  } else if (scroll_position == ScrollPosition::kBottom ||
             (scroll_position == ScrollPosition::kNone &&
              kCellBottomOffset > kContentViewBottomOffset)) {
    new_content_view_offset = \
        kContentViewOffset + (kCellBottomOffset - kContentViewBottomOffset);
  } else if (scroll_position == ScrollPosition::kMiddle) {
    new_content_view_offset = \
        kContentViewOffset
        + (cell_offset + 0.5 * cell_height)
        - (kContentViewOffset + GetHeight() / 2);
  }

  const float kContentViewSize = GetContentViewSize().height;
  if (new_content_view_offset < 0)
    new_content_view_offset = 0;
  else if (new_content_view_offset > (kContentViewSize - GetHeight()))
    new_content_view_offset = kContentViewSize - GetHeight();

  if (animating)
    AnimateContentViewOffset({0, new_content_view_offset}, 0.2);
  else
    SetContentViewOffset({0, new_content_view_offset});
}

void TableView::SelectRow(const CellIndex cell_index) {
  if (cell_index.section_index < 0 || cell_index.row_index < 0) {
    return;
  }

  if (delegate_ != nullptr &&
      !delegate_->TableViewShouldSelectRow(this, cell_index)) {
    return;
  }

  int position = 0;
  for (auto iterator = cell_indexes_for_selected_rows_.begin();
       iterator != cell_indexes_for_selected_rows_.end();
       ++iterator) {
    const CellIndex kCachedCellIndex = static_cast<CellIndex>(*iterator);
    if (kCachedCellIndex.section_index < cell_index.section_index ||
        (kCachedCellIndex.section_index == cell_index.section_index &&
         kCachedCellIndex.row_index < cell_index.row_index)) {
      ++position;
    } else if (kCachedCellIndex.section_index > cell_index.section_index ||
               (cell_index.section_index == kCachedCellIndex.section_index &&
                cell_index.row_index == kCachedCellIndex.row_index)) {
      break;
    }
  }  // end of `cell_indexes_for_selected_rows_` iterator.

  // Updates the states of the corresponded cell.
  TableViewCell* cell = GetCell(cell_index);
  if (cell != nullptr) {
    cell->set_selected(true);
    BringChildToFront(cell);
  }

  // Inserts the cell index to `cell_indexes_for_selected_rows_`.
  cell_indexes_for_selected_rows_.insert(
      cell_indexes_for_selected_rows_.begin() + position, cell_index);
  if (delegate_ != nullptr)
    delegate_->TableViewDidSelectRow(this, cell_index);
}

void TableView::SetCellHighlighted(TableViewCell* cell,
                                   const bool highlighted) {
  if (cell->highlighted() == highlighted)
    return;

  const CellIndex kCellIndex = GetCellIndex(cell);
  if (kCellIndex.section_index < 0 || kCellIndex.row_index < 0) {
    return;
  }
  if (highlighted && delegate_ != nullptr &&
      !delegate_->TableViewShouldHighlightRow(this, kCellIndex)) {
    return;
  }
  cell->set_highlighted(highlighted);
  if (highlighted)
    BringChildToFront(cell);

  if (delegate_ == nullptr) {
    return;
  }
  if (highlighted) {
    delegate_->TableViewDidHighlightRow(this, kCellIndex);
  } else {
    delegate_->TableViewDidUnhighlightRow(this, kCellIndex);
  }
}

bool TableView::ShouldHandleEvent(const Point location) {
  if (!ScrollView::ShouldHandleEvent(location))
    return false;

  for (TableViewCell* cell : visible_cells_) {
    if (cell->CollidePoint(location, 0))
      down_event_cell_ = cell;
  }
  return true;
}

bool TableView::UpdateLayout() {
  if (data_source_ == nullptr || IsHidden() || widget_view() == nullptr ||
      (GetWidth() == 0 && GetHeight() == 0)) {
    return;
  }

  const Point kContentViewOffset = GetContentViewOffset();
  const float kTopmostContentViewOffset = kContentViewOffset.y;
  const float kBottommostContentViewOffset = \
      kTopmostContentViewOffset + GetHeight();

  if (!should_update_layout_ &&
      kTopmostContentViewOffset == last_topmost_content_view_offset_ &&
      kBottommostContentViewOffset == last_bottommost_content_view_offset_) {
    return false;
  }
  should_update_layout_ = false;
  last_topmost_content_view_offset_ = kTopmostContentViewOffset;
  last_bottommost_content_view_offset_ = kBottommostContentViewOffset;

  float content_view_offset = -1;
  int index_of_visible_cells = -1;
  bool previous_cell_is_visible = false;
  bool done_processing_visible_cells = false;
  const float kTableWidth = GetWidth();

  // Table header view.
  if (table_header_view_ != nullptr) {
    table_header_view_->SetX(0);
    table_header_view_->SetY(0);
    table_header_view_->SetWidth(kTableWidth);
    AddChild(table_header_view_);
    content_view_offset = table_header_view_->GetHeight();
  }

  const int kNumberOfSections = data_source_->GetNumberOfSections(this);
  for (int section_index = 0;
       section_index < kNumberOfSections;
       ++section_index) {
    const int kNumberOfRows = data_source_->GetNumberOfRowsInSection(
        this, section_index);
    if (kNumberOfRows == 0) {
      continue;
    }
    if (section_index > 0) {
      content_view_offset += height_between_sections_;
    }

    // Section header.
    if (delegate_ != nullptr) {
      const float kHeaderHeight = delegate_->GetTableViewSectionHeaderHeight(
          this, section_index);
      moui::Widget* header = delegate_->GetTableViewSectionHeader(
          this, section_index);
      if (header != nullptr) {
        header->SetBounds(0, content_view_offset, kTableWidth, kHeaderHeight);
        AddChild(header);
      }
      content_view_offset = content_view_offset < 0 ?
                            kHeaderHeight :
                            content_view_offset + kHeaderHeight;
    }

    // Rows.
    for (int row_index = 0; row_index < kNumberOfRows; ++row_index) {
      const float kRowHeight = GetRowHeight({section_index, row_index});
      const float kCellTopOffset = content_view_offset < 0 ?
                                   0 : content_view_offset - 1;
      const float kCellBottomOffset = kCellTopOffset + kRowHeight - 1;
      content_view_offset = kCellTopOffset + kRowHeight;

      if (done_processing_visible_cells)
        continue;

      const bool kCellIsVisible = \
          kCellBottomOffset >= kTopmostContentViewOffset &&
          kBottommostContentViewOffset > kCellTopOffset;

      // Skips if the both the current and previous cells are invisible.
      if (!kCellIsVisible && !previous_cell_is_visible) {
        continue;
      }

      // Deals with the first visible cell. Removes all previous visible cells
      // ahead of which.
      if (kCellIsVisible && !previous_cell_is_visible) {
        previous_cell_is_visible = kCellIsVisible;
        int erase_last_index = 0;
        for (CellIndex& cell_index : cell_indexes_for_visible_rows_) {
          if (cell_index.section_index < section_index ||
              (cell_index.section_index == section_index &&
               cell_index.row_index < row_index)) {
             ++erase_last_index;
             continue;
          }
          break;
        }
        ReuseVisibleCells(0, erase_last_index);
      }  // end of dealing with the first visible cell.

      ++index_of_visible_cells;
      bool cell_was_visible = false;
      if (index_of_visible_cells < cell_indexes_for_visible_rows_.size()) {
        const CellIndex kCellIndex = \
            cell_indexes_for_visible_rows_[index_of_visible_cells];
        cell_was_visible = kCellIndex.section_index == section_index &&
                           kCellIndex.row_index == row_index;
      }

      TableViewCell* cell = nullptr;
      if (cell_was_visible) {
        cell = visible_cells_[index_of_visible_cells];
      } else {
        cell = data_source_->GetTableViewCell(this, section_index, row_index);
        visible_cells_.insert(visible_cells_.begin() + index_of_visible_cells,
                              cell);
        cell_indexes_for_visible_rows_.insert(
            cell_indexes_for_visible_rows_.begin() + index_of_visible_cells,
            {section_index, row_index});

        // Updates the selected state of the cell.
        bool cell_is_selected = false;
        for (CellIndex& cell_index : cell_indexes_for_selected_rows_) {
          if (cell_index.section_index == section_index &&
              cell_index.row_index == row_index) {
             cell_is_selected = true;
             break;
          }
        }
        cell->set_selected(cell_is_selected);
        AddChild(cell);
      }
      if (cell->highlighted())
        BringChildToFront(cell);
      else
        SendChildToBack(cell);
      cell->SetBounds(0, kCellTopOffset, kTableWidth, kRowHeight);

      if (kCellBottomOffset >= kBottommostContentViewOffset) {
        ReuseVisibleCells(
            index_of_visible_cells + 1,
            static_cast<int>(cell_indexes_for_visible_rows_.size()));
        done_processing_visible_cells = true;
      }
    }  // end of row

    // Section footer.
    if (delegate_ != nullptr) {
      const float kFooterHeight = delegate_->GetTableViewSectionFooterHeight(
          this, section_index);
      moui::Widget* footer = delegate_->GetTableViewSectionFooter(
          this, section_index);
      if (footer != nullptr) {
        footer->SetBounds(0, content_view_offset, kTableWidth, kFooterHeight);
        AddChild(footer);
      }
      content_view_offset += kFooterHeight;
    }
  }  // end of section

  // Table footer view.
  if (table_footer_view_ != nullptr) {
    table_footer_view_->SetX(0);
    table_footer_view_->SetY(content_view_offset);
    table_footer_view_->SetWidth(kTableWidth);
    content_view_offset += table_footer_view_->GetHeight();
    AddChild(table_footer_view_);
  }

  SetContentViewSize(-1, content_view_offset);
  layout_view_->Redraw();
  return true;
}

bool TableView::ValidateCellIndex(const CellIndex cell_index) {
  if (data_source_ == nullptr) {
    return false;
  }
  const int kNumberOfSections = data_source_->GetNumberOfSections(this);
  if (cell_index.section_index >= kNumberOfSections) {
    return false;
  }
  const int kNumberOfRows = data_source_->GetNumberOfRowsInSection(
      this, cell_index.section_index);
  if ((kNumberOfRows > 0 && cell_index.row_index >= kNumberOfRows)) {
    return false;
  }
  return true;
}

bool TableView::WidgetViewWillRender(NVGcontext* context) {
  const bool kResult = ScrollView::WidgetViewWillRender(context);
  if (!kResult)
    return false;

  UpdateLayout();
  return true;
}

void TableView::set_data_source(TableViewDataSource* data_source) {
  if (data_source != data_source_) {
    data_source_ = data_source;
    ReloadData();
  }
}

void TableView::set_height_between_sections(
    const float height_between_sections) {
  if (height_between_sections != height_between_sections_) {
    height_between_sections_ = height_between_sections;
    ReloadData();
  }
}

void TableView::set_row_height(const float row_height) {
  if (row_height == row_height_) {
    return;
  }
  row_height_ = row_height;
  should_update_layout_ = true;
  if (widget_view() != nullptr)
    widget_view()->Redraw();
}

void TableView::set_separator_color(const NVGcolor separator_color) {
  if (!nvgCompareColor(separator_color, separator_color_)) {
    separator_color_ = separator_color;
    layout_view_->Redraw();
  }
}

void TableView::set_separator_insets(const EdgeInsets separator_insets) {
  if (separator_insets.top == separator_insets_.top &&
      separator_insets.left == separator_insets_.left &&
      separator_insets.bottom == separator_insets_.bottom &&
      separator_insets.right == separator_insets_.right) {
    return;
  }
  separator_insets_ = separator_insets;
  should_update_layout_ = true;
  UpdateLayout();
}

void TableView::set_table_footer_view(moui::Widget* table_footer_view) {
  if (table_footer_view != table_footer_view_) {
    table_footer_view_ = table_footer_view;
    should_update_layout_ = true;
    UpdateLayout();
  }
}

void TableView::set_table_header_view(moui::Widget* table_header_view) {
  if (table_header_view != table_header_view_) {
    table_header_view_ = table_header_view;
    should_update_layout_ = true;
    UpdateLayout();
  }
}

}  // namespace moui
