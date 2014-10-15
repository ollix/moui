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

#ifndef MOUI_WIDGETS_WIDGET_VIEW_H_
#define MOUI_WIDGETS_WIDGET_VIEW_H_

#include <vector>

#include "moui/base.h"
#include "moui/core/event.h"
#include "moui/nanovg_hook.h"
#include "moui/ui/view.h"

namespace moui {

class Widget;

// The WidgetView class is designed specifically for rendering Widget instances.
// It sets up the nanovg context for rendering widgets, and comes with a
// managed widget as the root widget. All other widgets should be added to the
// managed widget for rendering.
class WidgetView : public View {
 public:
  WidgetView();
  ~WidgetView();

  // Sets the bounds for the view and its managed widget.
  void SetBounds(const int x, const int y, const int width,
                 const int height);

  // Accessor.
  Widget* widget() { return widget_; }

 private:
  // A widget item is a wrapper for a widget object and keeps some information
  // to render the widget.
  struct WidgetItem {
    // The pointer to the widget to render.
    Widget* widget;
    // The pointer to the widget item that corresponds to the current widget's
    // parent widget.
    WidgetItem* parent_item;
    // The origin of the widget that related to the current coordinate system.
    // This value is used as the origin for rendering the widget and
    // determining the related position of its child widgets.
    Point translate_origin;
    // The scissor's origin of the widget that related to the current
    // coordinate system. This value and `scissor_width` and `scissor_height`
    // forms the visible area of the widget.
    Point scissor_origin;
    // The scissor's width in points of the widget.
    float scissor_width;
    // The scissor's height in points of the widget.
    float scissor_height;
  };

  // Keeps a list of widget items.
  typedef std::vector<WidgetItem*> WidgetList;

  // This method gets called right after `context_` is created in Rneder().
  virtual void ContextDidCreate(NVGcontext* context) {};

  // Inherited from BaseView class.
  virtual void HandleEvent(std::unique_ptr<Event> event) override final;

  // Populates a list of widgets to render on screen in order. This method
  // itertates all children widgets and filters invisible onces.
  void PopulateWidgetList(WidgetList* widget_list, Widget* widget,
                          WidgetItem* parent_item);

  // Inherited from BaseView class. Renders belonged widgets recursively.
  virtual void Render() override final;

  // Inherited from BaseView class.
  virtual bool ShouldHandleEvent(const Point location) override final;

  // Returns true if the passed widget of any of its children should handle
  // coming events.
  bool ShouldHandleEvent(const Point location, Widget* widget);

  // The nanovg context for drawing.
  NVGcontext* context_;

  // The widget that is responsible to respond events passed to the
  // HandleEvent() method. The value is updated in the ShouldHandleEvent()
  // method.
  Widget* event_responder_;

  // The root widget for rendering. All its children will be rendered as well.
  Widget* widget_;

  DISALLOW_COPY_AND_ASSIGN(WidgetView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_WIDGET_VIEW_H_
