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

#include <stack>
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

  // Attaches a widget to the widget view for rendering.
  void AddWidget(Widget* widget);

  // Sets the bounds for the view and its managed widget.
  void SetBounds(const int x, const int y, const int width,
                 const int height);

  // Accessors and setters.
  bool is_opaque() const { return is_opaque_; }
  void set_is_opaque(const bool is_opaque);

 private:
  // A widget item is a wrapper for a widget object and keeps some information
  // to render the widget.
  struct WidgetItem {
    // The pointer to the widget to render.
    Widget* widget;
    // The origin of the widget that related to its parent widget.
    Point origin;
    // The widget's width in points.
    float width;
    // The widget's height in points.
    float height;
    // The hierarchy level of the widget. 0 indicates the toppest level.
    int level;
    // The pointer to the widget item that corresponds to the current widget's
    // parent widget.
    WidgetItem* parent_item;
    // The rendering scale of the widget.
    float scale;
    // The origin of the widget that related to the current coordinate system.
    // This value is used as the origin for rendering the widget and
    // determining the related position of its child widgets.
    Point translated_origin;
    // The scissor's origin of the widget that related to the current
    // coordinate system. This value and `scissor_width` and `scissor_height`
    // forms the visible area of the widget.
    Point scissor_origin;
    // The scissor's width in points of the widget.
    float scissor_width;
    // The scissor's height in points of the widget.
    float scissor_height;
  };

  // Keeps a stack of widget items in the rendering hierarchy.
  typedef std::stack<WidgetItem*> WidgetItemStack;

  // Keeps a list of widget items to render in order.
  typedef std::vector<WidgetItem*> WidgetList;

  // This method gets called right after `context_` is created in Rneder().
  virtual void ContextDidCreate(NVGcontext* context) {};

  // Inherited from BaseView class.
  virtual void HandleEvent(std::unique_ptr<Event> event) override final;

  // Pops widget items from the stack and finalizes each popped widget until
  // reaching the passed level.
  void PopAndFinalizeWidgetItems(const int level, WidgetItemStack* stack);

  // Populates a list of widgets to render on screen in order. This method
  // itertates all children widgets and filters invisible onces.
  void PopulateWidgetList(const int level, const float scale,
                          WidgetList* widget_list, Widget* widget,
                          WidgetItem* parent_item);

  // Inherited from BaseView class. Renders belonged widgets recursively.
  virtual void Render() override final;

  // Inherited from BaseView class.
  virtual bool ShouldHandleEvent(const Point location) override final;

  // Updates the `event_responders_` instance variable based on the passed
  // location. This is a recursive method, both the returned value and
  // the `widget` parameter are reserved for the recursion purpose.
  // When calling this method, specify `nullptr` for the `widget` parameter.
  bool UpdateEventResponders(const Point location, Widget* widget);

  // This method gets called when the view did finish rendering all visible
  // widgets.
  virtual void ViewDidRender(NVGcontext* context) {}

  // This method gets called before the view started rendering widgets.
  virtual void ViewWillRender(NVGcontext* context) {}

  // Calls the `Widget::WidgetViewDidRender()` method of the passed widget
  // and all of its children recursively. If the passed widget is the
  // `root_widget_`, `WidgetView::ViewDidRender()` is called instead.
  void WidgetViewDidRender(Widget* widget);

  // Calls the `Widget::WidgetViewWillRender()` method of the passed widget
  // and all of its children recursively. If the passed widget is the
  // `root_widget_`, `WidgetView::ViewWillRender()` is called instead.
  void WidgetViewWillRender(Widget* widget);

  // The nanovg context for drawing.
  NVGcontext* context_;

  // Keeps a list of widgets to handle events passed to the `HandleEvent()`
  // method. The list could be updated by `UpdateEventResponders()`.
  std::vector<Widget*> event_responders_;

  // Indicates whether the view is opaque.
  bool is_opaque_;

  // The root widget for rendering. All its children will be rendered as well.
  Widget* root_widget_;

  DISALLOW_COPY_AND_ASSIGN(WidgetView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_WIDGET_VIEW_H_
