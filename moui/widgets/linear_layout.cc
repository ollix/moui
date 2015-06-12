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

#include "moui/widgets/linear_layout.h"

#include <cassert>
#include <vector>

#include "moui/widgets/layout.h"
#include "moui/widgets/scroll_view.h"
#include "moui/widgets/widget.h"

namespace moui {

LinearLayout::LinearLayout(const Orientation orientation)
    : Layout(), orientation_(orientation) {
}

LinearLayout::~LinearLayout() {
}

void LinearLayout::ArrangeChildren() {
  float offset = 0;
  Size content_view_size = GetContentViewSize();

  for (ManagedWidget& child : managed_widgets_) {
    if (offset > 0)
      offset += spacing();
    else if (orientation_ == Layout::Orientation::kHorizontal)
      offset += left_padding();
    else if (orientation_ == Layout::Orientation::kVertical)
      offset += top_padding();

    Widget* widget = child.widget;
    if (orientation_ == Layout::Orientation::kHorizontal) {
      widget->SetX(offset);
      offset += child.size.width;
      content_view_size.height = std::max(
          content_view_size.height,
          widget->GetHeight() + top_padding() + bottom_padding());
    } else if (orientation_ == Layout::Orientation::kVertical) {
      widget->SetY(offset);
      offset += child.size.height;
      content_view_size.width = std::max(
          content_view_size.width,
          widget->GetWidth() + left_padding() + right_padding());
    } else {
      assert(false);
    }
  }

  if (orientation_ == Layout::Orientation::kHorizontal)
    SetContentViewSize(offset + right_padding(), content_view_size.height);
  else if (orientation_ == Layout::Orientation::kVertical)
    SetContentViewSize(content_view_size.width, offset + bottom_padding());
}

}  // namespace moui
