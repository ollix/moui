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

Layout::Layout() : adjusts_size_to_fit_contents_(false), bottom_padding_(0),
                   left_padding_(0), right_padding_(0),
                   should_rearrange_cells_(false), spacing_(0),
                   top_padding_(0) {
  set_is_opaque(false);
}

Layout::~Layout() {
  for (Widget* cell : GetCells())
    delete cell;
}

// When adding a child widget, the child is actually added to a newly created
// cell widget.
void Layout::AddChild(Widget* child) {
  auto cell = new Widget(false);
  cell->set_is_opaque(false);
  cell->AddChild(child);
  ScrollView::AddChild(cell);
  child->set_parent(this);
}

// Populates and returns a list of valid cells. Cells are actually the children
// of the content view's children of the inherited `ScrollView` class. Also,
// if `RemoveFromParent()` was called from one of the actual managed widgets,
// its corresponded cell will contain no child. And it's a good timing to
// free and remove the cell when this situation is detected.
std::vector<Widget*> Layout::GetCells() {
  std::vector<Widget*> valid_cells;
  std::vector<Widget*> stale_cells;
  for (Widget* cell : reinterpret_cast<ScrollView*>(this)->children()) {
    if (cell->children().size() == 1)
      valid_cells.push_back(cell);
    else
      stale_cells.push_back(cell);
  }

  // Frees stale cells.
  for (auto it = stale_cells.begin(); it != stale_cells.end(); ++it) {
    Widget* cell = reinterpret_cast<Widget*>(*it);
    cell->RemoveFromParent();
    delete cell;
  }

  return valid_cells;
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
    Widget* widget = cell->children().at(0);
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
  if (!ShouldRearrangeCells()) {
    return ScrollView::WidgetViewWillRender(context);
  }
  should_rearrange_cells_ = false;

  // Updates managed widgets.
  managed_widgets_.clear();
  for (Widget* cell : GetCells()) {
    Widget* widget = cell->children().at(0);
    Size occupied_size;
    widget->GetOccupiedSpace(&occupied_size);
    managed_widgets_.push_back({widget, occupied_size, cell});
  }
  ArrangeCells(managed_widgets_);
  return false;
}

std::vector<Widget*>& Layout::children() {
  managed_children_.clear();
  for (Widget* cell : GetCells()) {
    if (cell->children().size() == 1)
      managed_children_.push_back(cell->children().at(0));
  }
  return managed_children_;
}

void Layout::set_bottom_padding(const float padding) {
  if (padding == bottom_padding_)
    return;
  bottom_padding_ = padding;
  Redraw();
}

void Layout::set_left_padding(const float padding) {
  if (padding == left_padding_)
    return;
  left_padding_ = padding;
  Redraw();
}

void Layout::set_right_padding(const float padding) {
  if (padding == right_padding_)
    return;
  right_padding_ = padding;
  Redraw();
}

void Layout::set_spacing(const float spacing) {
  if (spacing == spacing_)
    return;
  spacing_ = spacing;
  Redraw();
}

void Layout::set_top_padding(const float padding) {
  if (padding == top_padding_)
    return;
  top_padding_ = padding;
  Redraw();
}

}  // namespace moui
