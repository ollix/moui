// Copyright (c) 2014 Ollix. All rights reserved.
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

#include "moui/widgets/layout.h"

#include <vector>

#include "moui/widgets/scroll_view.h"
#include "moui/widgets/widget.h"

namespace moui {

Layout::Layout() : adjusts_size_to_fit_contents_(false),
                   should_rearrange_cells_(false), spacing_(0) {
  set_is_opaque(false);
}

Layout::~Layout() {
  ResetCells();
}

// When adding a child widget, the child is actually added to a newly created
// cell widget.
void Layout::AddChild(Widget* child) {
  auto cell = new Widget;
  cell->set_is_opaque(false);
  cell->AddChild(child);
  ScrollView::AddChild(cell);
  child->set_parent(this);
  all_cells_.push_back(cell);
}

// Populates and returns a list of valid cells. Cells are actually the children
// of the content view's children of the inherited `ScrollView` class. Also,
// if `RemoveFromParent()` was called from one of the actual managed widgets,
// its corresponded cell will contain no child. And it's a good timing to
// free and remove the cell when this situation is detected.
std::vector<Widget*> Layout::GetCells() {
  std::vector<Widget*> valid_cells;
  std::vector<Widget*> stale_cells;
  for (Widget* cell : *reinterpret_cast<ScrollView*>(this)->children()) {
    if (cell->children()->size() == 1)
      valid_cells.push_back(cell);
    else
      stale_cells.push_back(cell);
  }

  // Frees stale cells.
  for (auto it = stale_cells.begin(); it != stale_cells.end(); ++it) {
    Widget* cell = reinterpret_cast<Widget*>(*it);
    moui::Widget::SmartRelease(cell);
  }

  return valid_cells;
}

void Layout::Redraw() {
  should_rearrange_cells_ = true;
  Widget::Redraw();
}

void Layout::ResetCells() {
  for (Widget* cell : all_cells_) {
    cell->RemoveFromParent();
    moui::Widget::SmartRelease(cell);
  }
  all_cells_.clear();

  for (auto child : *children()) {
    child->RemoveFromParent();
    moui::Widget::SmartRelease(child);
  }
}

// Checks if there is any difference between the current child widgets and the
// managed widgets. If it is, the child widgets should be rearranged.
bool Layout::ShouldRearrangeCells() {
  if (should_rearrange_cells_)
    return true;

  std::vector<Widget*> cells = GetCells();
  if (cells.size() != managed_widgets_.size()) {
    should_rearrange_cells_ = true;
    return true;
  }

  int index = 0;
  for (Widget* cell : cells) {
    auto widget = reinterpret_cast<Widget*>(cell->children()->front());
    Size occupied_size;
    widget->GetOccupiedSpace(&occupied_size);

    ManagedWidget managed_widget = managed_widgets_[index++];
    if (managed_widget.widget != widget ||
        managed_widget.occupied_size.width != occupied_size.width ||
        managed_widget.occupied_size.height != occupied_size.height) {
      should_rearrange_cells_ = true;
      return true;
    }
  }
  return false;
}

void Layout::UpdateContentSize(const float width, const float height) {
  SetContentViewSize(width, height);

  if (adjusts_size_to_fit_contents_) {
    SetWidth(width);
    SetHeight(height);
  }
}

bool Layout::WidgetViewWillRender(NVGcontext* context) {
  const bool kResult = ScrollView::WidgetViewWillRender(context);
  if (!ShouldRearrangeCells()) {
    return kResult;
  }
  should_rearrange_cells_ = false;

  // Updates managed widgets.
  managed_widgets_.clear();
  for (Widget* cell : GetCells()) {
    auto widget = reinterpret_cast<Widget*>(cell->children()->front());
    Size occupied_size;
    widget->GetOccupiedSpace(&occupied_size);
    managed_widgets_.push_back({widget, occupied_size, cell});
  }
  ArrangeCells(managed_widgets_);
  return should_rearrange_cells_ ? false : kResult;
}

std::vector<Widget*>* Layout::children() {
  managed_children_.clear();
  for (Widget* cell : GetCells()) {
    if (cell->children()->size() == 1)
      managed_children_.push_back(cell->children()->front());
  }
  return &managed_children_;
}

void Layout::set_spacing(const float spacing) {
  if (spacing == spacing_)
    return;
  spacing_ = spacing;
  Redraw();
}

}  // namespace moui
