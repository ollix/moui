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
  // The type of standard accessory control used by a cell.
  enum class AccessoryType {
    // The cell does not have any accessory view. This is the default value.
    kNone,
    // The cell has a check mark on its right side. This control does not
    // track touches.
    kCheckmark,
    // The cell has an accessory control shaped like a chevron. This control
    // indicates that tapping the cell triggers a push action. The control
    // does not track touches.
    kDisclosureIndicator,
  };

  // An enumeration for the various styles of cells.
  enum class Style {
    // A simple style for a cell with a text label (balck and left-aligned) and
    // an optional image view.
    kDefault,
    // A style for a cell with label on the left side of the cell with
    // left-aligned and black text; on the right side is a label that has
    // smaller gray text and is right-aligned.
    kValue1,
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
  AccessoryType accessory_type() const { return accessory_type_; }
  void set_accessory_type(const AccessoryType accessory_type);
  Widget* content_view() const { return content_view_; }
  Label* detail_text_label() const { return detail_text_label_; }
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
  // Renders the content view.
  void RenderContentView(moui::Widget* widget, NVGcontext* context);

  // Updates the cell's layout with the configured style.
  void UpdateLayout(NVGcontext* context);

  // Indicates the type of standard accessory view the cell should use.
  AccessoryType accessory_type_;

  // The strong reference to the default superview for content displayed by the
  // cell.
  Widget* content_view_;

  // The strong reference to the secondary label of the table cell if one
  // exists.
  Label* detail_text_label_;

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

  // Indicates the style applied to the cell.
  Style style_;

  // The strong reference to the label used for the main textual content of
  // the table cell.
  Label* text_label_;

  DISALLOW_COPY_AND_ASSIGN(TableViewCell);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_TABLE_VIEW_CELL_H_
