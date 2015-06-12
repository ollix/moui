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

Layout::Layout() : ScrollView(), bottom_padding_(0), left_padding_(0),
                   right_padding_(0), should_arrange_children_(false),
                   spacing_(0), top_padding_(0) {
  set_is_opaque(false);
}

Layout::~Layout() {
}

// Checks if there is any difference between the current child widgets and the
// managed widgets. If it is, the child widgets should be rearranged.
bool Layout::ShouldArrangeChildren() {
  if (should_arrange_children_)
    return true;

  if (children().size() != managed_widgets_.size()) {
    should_arrange_children_ = true;
    return true;
  }

  int i = 0;
  for (Widget* child : children()) {
    ManagedWidget managed_widget = managed_widgets_[i++];
    if (managed_widget.widget != child ||
        managed_widget.origin.x != child->GetX() ||
        managed_widget.origin.y != child->GetY() ||
        managed_widget.size.width != child->GetWidth() * child->scale() ||
        managed_widget.size.height != child->GetHeight() * child->scale()) {
      should_arrange_children_ = true;
      return true;
    }
  }
  return false;
}

bool Layout::WidgetViewWillRender(NVGcontext* context) {
  if (!ShouldArrangeChildren()) {
    return ScrollView::WidgetViewWillRender(context);
  }
  should_arrange_children_ = false;

  // Updates the state of managed widgets.
  managed_widgets_.clear();
  for (Widget* child : children()) {
    managed_widgets_.push_back({{child->GetX(), child->GetY()},
                                {child->GetWidth() * child->scale(),
                                 child->GetHeight() * child->scale()},
                                child});
  }
  ArrangeChildren();
  return false;
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
