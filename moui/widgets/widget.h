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

#ifndef MOUI_WIDGETS_WIDGET_H_
#define MOUI_WIDGETS_WIDGET_H_

#include <vector>

#include "moui/base.h"

struct NVGcontext;

namespace moui {

class WidgetView;

// A widget is a graphical element that can rendering based on nanovg context.
// Widgets must be added to the managed widget of a WidgetView instance before
// it can be rendering.
class Widget {
 public:
  enum class Alignment {
    kLeft,
    kCenter,
    kRight,
    kTop,
    kMiddle,
    kBottom,
  };
  enum class Unit {
    kPercent,
    kPixel,
  };

  explicit Widget();
  ~Widget();

  // Adds child widget.
  void AddChild(Widget* child);

  // Returns the height in pixels.
  int GetHeight() const;

  // Returns the width in pixels.
  int GetWidth() const;

  // Returns the horizontal position in pixels related to its parent's left.
  int GetX() const;

  // Returns the vertical position in pixels related to its parent's top.
  int GetY() const;

  // Returns true if the widget is hidden.
  bool IsHidden() const;

  // Redraws the widget view containing this widget. Note that all widgets
  // belonged to the same widget view will be drawn by calling this method.
  // If the current widget doesn't belong to any widget view, nothing happened.
  void Redraw() const;

  // Implements the logic for rendering the widget.
  virtual void Render(struct NVGcontext* context) {}

  // Sets the height with the specified unit.
  void SetHeight(const Unit unit, const float height);

  // Sets whether the widget should be visible.
  void SetHidden(bool hidden);

  // Sets the width with the specified unit.
  void SetWidth(const Unit unit, const float width);

  // Sets the horizontal position. The alignment could only be one of kLeft,
  // kCenter, and kRight.
  void SetX(const Alignment alignment, const Unit unit, const float x);

  // Sets the vertical position. The alignment could only be one of kTop,
  // kMiddle, and kBottom.
  void SetY(const Alignment alignment, const Unit unit, const float y);

  // Setters and accessors.
  std::vector<Widget*>& children() { return children_; }

 private:
  friend class WidgetView;

  // Accessors.
  void set_parent(Widget* parent) { parent_ = parent; }
  // This setter that should only be called by the WidgetView firend class.
  void set_widget_view(WidgetView* widget_view) { widget_view_ = widget_view; }

  // Updates the widget view for the specified widget and all its children
  // recursively.
  void UpdateWidgetViewRecursively(Widget* widget);

  // Holds a list of child widgets.
  std::vector<Widget*> children_;

  // The unit of the height_value_.
  Unit height_unit_;

  // The height value represented as height_unit_.
  float height_value_;

  // Indicates whether the widget is hidden.
  bool hidden_;

  // The parent widget of the current widget.
  Widget* parent_;

  // The WidgetView that contains the this widget instance. This is dedicated
  // for the convenient Redraw() method.
  WidgetView* widget_view_;

  // The unit of the width_value_.
  Unit width_unit_;

  // The width value represented as width_unit_.
  float width_value_;

  // The alignment of the x_value_.
  Alignment x_alignment_;

  // The unit of the x_value_.
  Unit x_unit_;

  // The horizontal position related to its parent.
  float x_value_;

  // The alignment of the y_value_.
  Alignment y_alignment_;

  // The unit of the y_value_.
  Unit y_unit_;

  // The vertical position related to its parent.
  float y_value_;

  DISALLOW_COPY_AND_ASSIGN(Widget);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_WIDGET_H_
