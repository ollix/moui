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

#include <algorithm>
#include <vector>

#include "moui/widgets/layout.h"
#include "moui/widgets/widget.h"

namespace moui {

LinearLayout::LinearLayout(const Orientation orientation)
    : Layout(), orientation_(orientation) {
}

LinearLayout::~LinearLayout() {
}

void LinearLayout::ArrangeCells(const ManagedWidgetVector managed_widgets) {
  // Determines the maximum cell length. For horizontal orientation, the length
  // represents the cell's height. For vertical orientation, the length
  // represents the cell's width.
  float cell_length = orientation_ == Layout::Orientation::kHorizontal ?
                      GetHeight() - top_padding() - bottom_padding():
                      GetWidth() - left_padding() - right_padding();
  for (ManagedWidget managed_widget : managed_widgets) {
    if (orientation_ == Layout::Orientation::kHorizontal) {
      cell_length = std::max(cell_length, managed_widget.occupied_size.height);
    } else if (orientation_ == Layout::Orientation::kVertical) {
      cell_length = std::max(cell_length, managed_widget.occupied_size.width);
    }
  }

  // Updates the bounds of every cell.
  float offset = 0;
  for (ManagedWidget managed_widget : managed_widgets) {
    if (offset > 0)
      offset += spacing();
    else if (orientation_ == Layout::Orientation::kHorizontal)
      offset += left_padding();
    else if (orientation_ == Layout::Orientation::kVertical)
      offset += top_padding();

    Widget* cell = managed_widget.cell;
    if (orientation_ == Layout::Orientation::kHorizontal) {
      cell->SetX(offset);
      cell->SetY(top_padding());
      cell->SetWidth(managed_widget.occupied_size.width);
      cell->SetHeight(cell_length);
      offset += managed_widget.occupied_size.width;
    } else if (orientation_ == Layout::Orientation::kVertical) {
      cell->SetX(left_padding());
      cell->SetY(offset);
      cell->SetWidth(cell_length);
      cell->SetHeight(managed_widget.occupied_size.height);
      offset += managed_widget.occupied_size.height;
    }
  }

  // Updates the size of content view.
  if (orientation_ == Layout::Orientation::kHorizontal) {
    UpdateContentSize(offset + right_padding(),
                      cell_length + top_padding() + bottom_padding());
  } else if (orientation_ == Layout::Orientation::kVertical) {
    UpdateContentSize(cell_length + left_padding() + right_padding(),
                      offset + bottom_padding());
  }
}

}  // namespace moui
