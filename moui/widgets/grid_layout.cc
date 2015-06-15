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
  const int kNumberOfColumns = number_of_columns_;
  float width_of_columns[kNumberOfColumns];
  for (int i = 0; i < kNumberOfColumns; ++i)
    width_of_columns[i] = 0;

  std::vector<Size> child_sizes;
  for (ManagedWidget& child : managed_widgets_) {
    width_of_columns[column] = std::max(width_of_columns[column],
                                        child.size.width);
    if (++column == kNumberOfColumns)
      column = 0;
  }

  // Arranges child widgets.
  column = -1;
  float column_offset = left_padding();
  float row_height = 0;
  float row_offset = top_padding();
  for (ManagedWidget& child : managed_widgets_) {
    if (++column == kNumberOfColumns) {
      column = 0;
      column_offset = left_padding();
      row_offset += row_height + spacing();
      row_height = 0;
    } else if (column > 0) {
      column_offset += spacing();
    }

    Widget* widget = child.widget;
    widget->SetX(Widget::Alignment::kLeft, Widget::Unit::kPoint, column_offset);
    widget->SetY(Widget::Alignment::kTop, Widget::Unit::kPoint, row_offset);

    row_height = std::max(row_height, child.size.height);
    column_offset += width_of_columns[column];
  }

  // Updates the size of the content view.
  const float kContentHeight = row_offset + row_height + bottom_padding();
  float content_width = \
      left_padding() + right_padding() + spacing() * (kNumberOfColumns - 1);
  for (int i = 0; i < kNumberOfColumns; ++i)
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
