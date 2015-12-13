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
#include "moui/widgets/widget.h"

namespace moui {

GridLayout::GridLayout(const int number_of_columns)
    : Layout(), number_of_columns_(number_of_columns) {
}

GridLayout::~GridLayout() {
}

void GridLayout::ArrangeCells(const ManagedWidgetVector managed_widgets) {
  // Determines the width of each column and the height of each row.
  const int kNumberOfColumns = number_of_columns_;
  const int kNumberOfRows = \
      std::ceil(1.0 * managed_widgets.size() / kNumberOfColumns);

  float width_of_columns[kNumberOfColumns];
  for (int i = 0; i < kNumberOfColumns; ++i)
    width_of_columns[i] = 0;

  float height_of_rows[kNumberOfRows];
  for (int i = 0; i < kNumberOfRows; ++i)
    height_of_rows[i] = 0;

  int column = 0;
  int row = 0;
  for (ManagedWidget managed_widget : managed_widgets) {
    width_of_columns[column] = std::max(width_of_columns[column],
                                        managed_widget.occupied_size.width);
    height_of_rows[row] = std::max(height_of_rows[row],
                                   managed_widget.occupied_size.height);
    if (++column == kNumberOfColumns) {
      column = 0;
      ++row;
    }
  }

  // Updates the bounds of every cell.
  column = -1;
  row = 0;
  float column_offset = left_padding();
  float row_offset = top_padding();
  float row_height = 0;
  for (ManagedWidget managed_widget : managed_widgets) {
    if (++column == kNumberOfColumns) {
      column = 0;
      column_offset = left_padding();
      row_offset += height_of_rows[++row] + spacing();
    } else if (column > 0) {
      column_offset += spacing();
    }

    Widget* cell = managed_widget.cell;
    if (row < kNumberOfRows)
      row_height = height_of_rows[row];

    cell->SetX(column_offset);
    cell->SetY(row_offset);
    cell->SetWidth(width_of_columns[column]);
    cell->SetHeight(row_height);

    column_offset += width_of_columns[column];
  }

  // Updates the size of content view.
  const float kContentHeight = row_offset + row_height + bottom_padding();
  float content_width = \
      left_padding() + right_padding() + spacing() * (kNumberOfColumns - 1);
  for (int i = 0; i < kNumberOfColumns; ++i)
    content_width += width_of_columns[i];
  UpdateContentSize(content_width, kContentHeight);
}

void GridLayout::set_number_of_columns(const int number_of_columns) {
  if (number_of_columns != number_of_columns_) {
    number_of_columns_ = number_of_columns;
    RearrangeCells();
  }
}

}  // namespace moui
