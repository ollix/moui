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

#ifndef MOUI_WIDGETS_TABLE_VIEW_CELL_H_
#define MOUI_WIDGETS_TABLE_VIEW_CELL_H_

#include <string>

#include "moui/base.h"
#include "moui/widgets/widget.h"

namespace moui {

// Forward declaration.
class Label;

// The `TableViewCell` class defines the attributes and behavior of the cells
// that appear in `TableView` objects.
class TableViewCell : public Widget {
 public:
  // An enumeration for the various styles of cells.
  enum class Style {
    // A simple style for a cell with a text label (balck and left-aligned) and
    // an optional image view.
    kDefault,
  };

  TableViewCell(const Style style, const std::string& reuse_identifier);
  explicit TableViewCell(const Style style);
  ~TableViewCell();

  // Prepares a reusable cell for reuse by the table view's delegate. This
  // method is invoked just before the object is returned from the
  // `TableView::DequeueReusableCell()` method. If the cell object does not
  // have an associated reuse identifier, this method is not called.
  virtual void PrepareForReuse();

  // Setters and accessors.
  Widget* content_view() const { return content_view_; }
  bool highlighted() const { return highlighted_; }
  void set_highlighted(const bool highlighted);
  Widget* image_view() const { return image_view_; }
  std::string reuse_identifier() const { return reuse_identifier_; }
  bool selected() const { return selected_; }
  void set_selected(const bool selected);
  Style style() const { return style_; }
  Label* text_label() const { return text_label_; }

 protected:
  // Inherited from `Widget` class.
  bool WidgetViewWillRender(NVGcontext* context) override;

 private:
  // Updates the cell's layout with the configured style.
  void UpdateLayout();

  // The strong reference to the default superview for content displayed by the
  // cell.
  Widget* content_view_;

  // Indicates whether the cell is highlighted.
  bool highlighted_;

  // The strong reference to the image view object of the table view, which
  // initially sets to hidden. If the view is set not hidden, it appears on
  // the left side of the cell, before any label.
  Widget* image_view_;

  // Indicates a string used to identify the cell object if it is to be reused
  // for drawing multiple rows of a table view. Pass an empty string if the
  // cell object is not to be reused.
  std::string reuse_identifier_;

  // Indicates whether the cell is selected.
  bool selected_;

  // The strong reference to the label used for the main textual content of
  // the table cell.
  Label* text_label_;

  // Indicates the style applied to the cell.
  Style style_;

  DISALLOW_COPY_AND_ASSIGN(TableViewCell);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_TABLE_VIEW_CELL_H_
