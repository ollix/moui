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

#include "moui/widgets/widget.h"

#include "moui/nanovg_hook.h"
#include "moui/widgets/widget_view.h"

namespace moui {

Widget::Widget() : height_(0), hidden_(false), widget_view_(nullptr), width_(0),
                   x_(0), y_(0) {
}

Widget::~Widget() {
}

void Widget::AddChild(Widget* child) {
  children_.push_back(child);
  UpdateWidgetViewRecursively(child);
}

void Widget::Redraw() const {
  widget_view_->Redraw();
}

void Widget::SetBounds(const int x, const int y, const int width,
                       const int height) {
  x_ = x;
  y_ = y;
  width_ = width;
  height_ = height;
}

void Widget::UpdateWidgetViewRecursively(Widget* widget) {
  widget->set_widget_view(widget_view_);
  for (Widget* child_widget : widget->children()) {
    UpdateWidgetViewRecursively(child_widget);
  }
}

}  // namespace moui
