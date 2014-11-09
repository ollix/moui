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

#include "moui/widgets/grid_layout.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "moui/widgets/layout.h"
#include "moui/widgets/scroll_view.h"
#include "moui/widgets/widget.h"

namespace moui {

GridLayout::GridLayout(const int number_of_columns)
    : Layout(), number_of_columns_(number_of_columns) {
}

GridLayout::~GridLayout() {
}

void GridLayout::ArrangeChildren() {
  // Determines the width of each column.
  int column = 0;
  float width_of_columns[number_of_columns_];
  for (int i = 0; i < number_of_columns_; ++i)
    width_of_columns[i] = 0;
  for (Widget* child : children()) {
    width_of_columns[column] = std::max(width_of_columns[column],
                                        child->GetWidth());
    if (++column == number_of_columns_)
      column = 0;
  }

  // Arranges child widgets.
  column = -1;
  float column_offset = 0;
  float row_height = 0;
  float row_offset = 0;
  for (Widget* child : children()) {
    if (++column == number_of_columns_) {
      column = 0;
      column_offset = 0;
      row_offset += row_height;
      row_height = 0;
    }
    child->SetX(Widget::Alignment::kLeft, Widget::Unit::kPoint, column_offset);
    child->SetY(Widget::Alignment::kTop, Widget::Unit::kPoint, row_offset);

    row_height = std::max(row_height, child->GetHeight());
    column_offset += width_of_columns[column];
  }

  // Updates the size of the content view.
  const float kContentHeight = row_offset + row_height;
  float content_width;
  for (int i = 0; i < number_of_columns_; ++i)
    content_width += width_of_columns[i];
  SetContentViewSize(content_width, kContentHeight);
}

void GridLayout::set_number_of_columns(const int number_of_columns) {
  if (number_of_columns != number_of_columns_) {
    number_of_columns_ = number_of_columns;
    Redraw();
  }
}

}  // namespace moui
