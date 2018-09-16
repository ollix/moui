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

#ifndef MOUI_WIDGETS_TABLE_VIEW_H_
#define MOUI_WIDGETS_TABLE_VIEW_H_

#include <cstdint>
#include <map>
#include <string>
#include <queue>
#include <vector>

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/scroll_view.h"

namespace moui {

// Forward declaration.
class TableViewCell;
class TableViewDataSource;
class TableViewDelegate;

// The `TableView` widget is a means for displaying and editing hierachical
// lists of information.
class TableView : public ScrollView {
 public:
  // Indicates the default value for a given dimension.
  static constexpr float kAutomaticDimenstion = INT_MIN;

  // A cell index representing the row and section of the cell.
  struct CellIndex {
    int section_index;
    int row_index;
  };

  // The position in the table view (top, middle, bottom) to which a given row
  // is scrolled.
  enum class ScrollPosition {
    // The table view scrolls the row of interest to be fully visible with a
    // minimum of movement.
    kNone,
    // The table view scrolls the row of interest to the top of the visible
    // table view.
    kTop,
    // The table view scrolls the row of interest to the middle of the visible
    // table view.
    kMiddle,
    // The table view scrolls the row of interest to the bottom of the visible
    // table view.
    kBottom,
  };

  TableView();
  ~TableView();

  // Deselects a row in the table view identified by `cell_index`.
  void DeselectRow(const CellIndex cell_index);

  // Returns a reusable table-view cell object located by its identifier.
  TableViewCell* DequeueReusableCell(const std::string identifier);

  // Returns the cell object at the specified cell index, or `nullptr` if the
  // cell is not visible or `cell_index` is out of range.
  TableViewCell* GetCell(const CellIndex cell_index) const;

  // Returns a cell index representing the row and section of the given
  // table-view cell. `nullptr` is returned if the cell index is invalid.
  CellIndex GetCellIndex(TableViewCell* cell) const;

  // Reloads the rows and sections of the table view.
  void ReloadData();

  // Refreshes the layout.
  void RefreshLayout();

  // Scrolls through the table view until a row identified by cell index is at
  // a particular location on the screen.
  void ScrollToCellIndex(const CellIndex cell_index,
                         const ScrollPosition scroll_position,
                         const bool animating);

  // Selects a row in the table view identified by `cell_index`.
  void SelectRow(const CellIndex cell_index);

  // Returns `true` if the specified cell index is valid.
  bool ValidateCellIndex(const CellIndex cell_index);

  // Setters and accessors.
  std::vector<CellIndex>* cell_indexes_for_selected_rows() {
    return &cell_indexes_for_selected_rows_;
  }
  std::vector<CellIndex>* cell_indexes_for_visible_rows() {
    return &cell_indexes_for_visible_rows_;
  }
  TableViewDataSource* data_source() const { return data_source_; }
  void set_data_source(TableViewDataSource* data_source);
  TableViewDelegate* delegate() const { return delegate_; }
  void set_delegate(TableViewDelegate* delegate) { delegate_ = delegate; }
  float height_between_sections() const { return height_between_sections_; }
  void set_height_between_sections(const float height_between_sections);
  float row_height() const { return row_height_; }
  void set_row_height(const float row_height);
  NVGcolor separator_color() const { return separator_color_; }
  void set_separator_color(const NVGcolor separator_color);
  EdgeInsets separator_insets() const { return separator_insets_; }
  void set_separator_insets(const EdgeInsets separator_insets);
  moui::Widget* table_footer_view() const { return table_footer_view_; }
  void set_table_footer_view(moui::Widget* table_footer_view);
  moui::Widget* table_header_view() const { return table_header_view_; }
  void set_table_header_view(moui::Widget* table_header_view);
  std::vector<TableViewCell*>* visible_cells() { return &visible_cells_; }

 protected:
  // Inherited from `Widget` class.
  bool WidgetViewWillRender(NVGcontext* context) override;

 private:
  // Returns the height to use for a row in a specified location.
  float GetRowHeight(const CellIndex cell_index);

  // Inherited from `Widget` class.
  bool HandleEvent(Event* event) final;

  // Highlights the `down_event_cell_` if the cell object is not `nullptr`.
  // This method exists for the purpose of delay highlighting.
  void HighlightDownEventCell();

  // Adds the specified cell to the `reusable_cells_` vector if the cell's
  // reusable identifier is not empty.
  void ReuseCell(TableViewCell* cell);

  // Reuses visible cells.
  void ReuseVisibleCells(const int begin, const int last);

  // Renders the `layout_view_`.
  void RenderLayoutView(moui::Widget* widget, NVGcontext* context);

  // Sets the highlighted state of a table-view cell.
  void SetCellHighlighted(TableViewCell* cell, const bool highlighted);

  // Inherited from `Widget` class.
  bool ShouldHandleEvent(const Point location) final;

  // Updates the layout of displayed cells. Returns `true` if the layout did
  // update.
  bool UpdateLayout();

  // The cell indexes representing the selected rows.
  std::vector<CellIndex> cell_indexes_for_selected_rows_;

  // Indicates a list of cell indexes each identifying a visible row in the
  // table view.
  std::vector<CellIndex> cell_indexes_for_visible_rows_;

  // The weak reference to the `TableViewDataSource` delegate instance.
  TableViewDataSource* data_source_;

  // The weak reference to the `TableViewDelegate` delegate instance.
  TableViewDelegate* delegate_;

  // The weak reference to the cell that receives the last down event.
  TableViewCell* down_event_cell_;

  // Records the cell's origin related to the corresponded widget view's
  // coordinate system when receiving the `Event::Type::kDown` event.
  Point down_event_origin_;

  // Indicates the height in points between sections.
  float height_between_sections_;

  // Keeps the bottommost content view offset last time updated layout.
  float last_bottommost_content_view_offset_;

  // Keeps the topmost content view offset last time updated layout.
  float last_topmost_content_view_offset_;

  // The strong reference to the view sitting above the content view to display
  // the layout such as separators.
  moui::Widget* layout_view_;

  // Keeps strong reference to the cell objects that are marked as reusable.
  // The key indicates the cells' `reuse_identifier` property.
  std::map<std::string, std::queue<TableViewCell*>> reusable_cells_;

  // Indicates the height of each row in the table view.
  float row_height_;

  // Indicates whether the layout should update.
  bool should_update_layout_;

  // The color of separator rows in the table view.
  NVGcolor separator_color_;

  // Indicates the default inset of cell separators. Note that only left and
  // right insets are honered.
  EdgeInsets separator_insets_;

  // The weak reference to the accessory view that is displayed below the table.
  // The default value is `nullptr`. The table view is different from a section
  // footer.
  moui::Widget* table_footer_view_;

  // The weak reference to the accessory view that is displayed above the table.
  // The default value is `nullptr`. The table view is different from a section
  // header.
  moui::Widget* table_header_view_;

  // The table cells that are visible in the table view.
  std::vector<TableViewCell*> visible_cells_;

  DISALLOW_COPY_AND_ASSIGN(TableView);
};

// The `TableViewDataSource` class is adopted by an object that mediates the
// application's data model for a `TableView` object. The data source provides
// the table-view object with the information it needs to construct and modify
// a table view.
class TableViewDataSource {
 public:
  TableViewDataSource() {}
  ~TableViewDataSource() {}

  // Asks the data source for a cell to insert in a particular location of
  // the table view.
  virtual TableViewCell* GetTableViewCell(TableView* table_view,
                                          const int section_index,
                                          const int row_index) = 0;

  // Tells the data source to return the number of rows in a given section of
  // a table view.
  virtual int GetNumberOfRowsInSection(TableView* table_view,
                                       const int section_index) = 0;

  // Asks the data source to return the number of sections in the table view.
  // The default value is 1.
  virtual int GetNumberOfSections(TableView* table_view) { return 1; }

 private:
  DISALLOW_COPY_AND_ASSIGN(TableViewDataSource);
};

// The `TableViewDelegate` class allows the table view delegate to manage
// selections, configure section headings and footers, and perform other
// actions.
class TableViewDelegate {
 public:
  TableViewDelegate() {}
  virtual ~TableViewDelegate() {}

  // Asks the delegate for a footer widget to display in the footer of the
  // specified section of the table view. This method only works correctly
  // when `GetTableViewSectionFooterHeight()` is also implemented.
  virtual moui::Widget* GetTableViewSectionFooter(TableView* table_view,
                                                  const int section_index) {
    return nullptr;
  }

  // Asks the delegate for the height to use for the footer of a particular
  // section.
  virtual float GetTableViewSectionFooterHeight(TableView* table_view,
                                                const int section_index) {
    return 0;
  }

  // Asks the delegate for a header widget to display in the header of the
  // specified section of the table view. This method only works correctly
  // when `GetTableViewSectionHeaderHeight()` is also implemented.
  virtual moui::Widget* GetTableViewSectionHeader(TableView* table_view,
                                                  const int section_index) {
    return nullptr;
  }

  // Asks the delegate for the height to use for the header of a particular
  // section.
  virtual float GetTableViewSectionHeaderHeight(TableView* table_view,
                                                const int section_index) {
    return 0;
  }

  // Asks the delegate for the height to use for a row in a specified location.
  virtual float GetTableViewRowHeight(TableView* table_view,
                                      const TableView::CellIndex cell_index) {
    return TableView::kAutomaticDimenstion;
  }

  // Tells the delegate that the specified row is now deselected.
  virtual void TableViewDidDeselectRow(TableView* table_view,
                                       const TableView::CellIndex cell_index) {}

  // Tells the delegate that the specified row is now highlighted.
  virtual void TableViewDidHighlightRow(
      TableView* table_view, const TableView::CellIndex cell_index) {}

  // Tells the delegate that the specified row is now selected.
  virtual void TableViewDidSelectRow(TableView* table_view,
                                     const TableView::CellIndex cell_index) {}

  // Tells the delegate that the highlight was removed from the row at the
  // specified cell index.
  virtual void TableViewDidUnhighlightRow(
      TableView* table_view, const TableView::CellIndex cell_index) {}

  // Asks the delegate if the specified row should be deselected.
  virtual bool TableViewShouldDeselectRow(
      TableView* table_view, const TableView::CellIndex cell_index) {
    return true;
  }

  // Asks the delegate if the specified row should be highlighted.
  virtual bool TableViewShouldHighlightRow(
      TableView* table_view, const TableView::CellIndex cell_index) {
    return true;
  }

  // Asks the delegate if the specified row should be selected.
  virtual bool TableViewShouldSelectRow(
      TableView* table_view, const TableView::CellIndex cell_index) {
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(TableViewDelegate);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_TABLE_VIEW_H_
