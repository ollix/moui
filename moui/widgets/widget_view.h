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

  // Inherited from `View` class. Calls the `HandleMemoryWarning()` method on
  // all managed widgets recursively.
  void HandleMemoryWarning() final;

  // Inherited from `View` class.
  void Redraw() final;

  // Inherited from `View` class. Redraws the specified `widget` only if it's
  // currently visible.
  void Redraw(Widget* widget);

  // Sets the bounds for the view and its managed widget.
  void SetBounds(const int x, const int y, const int width, const int height);

  // Accessors and setters.
  NVGcontext* context();
  Widget* root_widget() const { return root_widget_; }

 private:
  // Allows `Widget::GetSnapshot()` to call the `Render()` method.
  friend class Widget;

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
    // The opacity value of the widget.
    float alpha;
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

  // Inherited from `BaseView` class.
  void HandleEvent(std::unique_ptr<Event> event) final;

  // Calls the `HandleMemoryWarning()` method for the specified `widget` and
  // all of its descendants.
  void HandleMemoryWarningRecursively(moui::Widget* widget);

  // Pops widget items from the stack and finalizes each popped widget until
  // reaching the passed level.
  void PopAndFinalizeWidgetItems(const int level, WidgetItemStack* stack);

  // Populates a list of widgets to render on screen in order. This method
  // itertates all children widgets and filters invisible onces.
  void PopulateWidgetList(const int level, const float scale,
                          WidgetList* widget_list, Widget* widget,
                          WidgetItem* parent_item);

  // Inherited from `BaseView` class. Renders belonged widgets recursively.
  void Render() final;

  // Renders the specified `widget` and all of its descendant widgets to the
  // specified `framebuffer`. If the `framebuffer` is `nullptr`, the `widget`
  // will be rendered without binding the `framebuffer`. Note that the
  // specified `widget` will always be rendered even if its `hidden_` property
  // is set to `true`.
  void Render(Widget* widget, NVGLUframebuffer* framebuffer);

  // Inherited from `BaseView` class.
  bool ShouldHandleEvent(const Point location) final;

  // Updates the `event_responders_` instance variable based on the passed
  // location. This is a recursive method, both the returned value and
  // the `widget` parameter are reserved for the recursion purpose.
  // When calling this method, specify `nullptr` for the `widget` parameter.
  bool UpdateEventResponders(const Point location, Widget* widget);

  // Calls the `Widget::WidgetViewDidRender()` method on the passed widget
  // and all of its descendant widgets recursively.
  void WidgetViewDidRender(Widget* widget);

  // Calls the `Widget::WidgetViewWillRender()` method on the passed widget
  // and all of its descendant widgets recursively.
  void WidgetViewWillRender(Widget* widget);

  // The nanovg context for rendering.
  NVGcontext* context_;

  // Keeps a list of widgets to handle events passed to the `HandleEvent()`
  // method. The list could be updated by `UpdateEventResponders()`.
  std::vector<Widget*> event_responders_;

  // The root widget for rendering. All its children will be rendered as well.
  Widget* root_widget_;

  // Indicating whether the widget view is preparing for rendering in the
  // `Render()` method.
  bool preparing_for_rendering_;

  // Indicates whether receiving the redraw request while preparing for
  // rendering. The value is updated in the `Redraw()`. If this value and
  // `preparing_for_rendering_` are both true in the `Render()` method.
  // It won't start another round of the rendering process.
  bool requests_redraw_;

  // Keeps a list of currently visible widgets. The list will be updated
  // whenever executing the `Render()` method.
  std::vector<Widget*> visible_widgets_;

  DISALLOW_COPY_AND_ASSIGN(WidgetView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_WIDGET_VIEW_H_
