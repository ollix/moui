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

#include <mutex>
#include <vector>

#include "moui/base.h"
#include "moui/nanovg_hook.h"

namespace moui {

class Event;
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
    kPoint,
  };

  Widget();
  explicit Widget(const bool caches_rendering);
  ~Widget();

  // Adds child widget.
  void AddChild(Widget* child);

  // Returns true if the passed point is within the region of the widget's
  // bounding box plus the passed padding at every direction.
  bool CollidePoint(const Point point, const int padding);

  // Returns true if the passed point is within the region of the widget's
  // bounding box plus the paddings at various direction.
  bool CollidePoint(const Point point, const int top_padding,
                    const int right_padding, const int bottom_padding,
                    const int left_padding);

  // Returns the height in points.
  float GetHeight() const;

  // Determines the widget's origin and size that related to the corresponded
  // widget view's coordinate system. `nullptr` could be passed as parameter
  // if it doesn't matter.
  void GetMeasuredBounds(Point* origin, Size* size);

  // Returns the width in points.
  float GetWidth() const;

  // Returns the horizontal position in points related to its parent's left.
  float GetX() const;

  // Returns the vertical position in points related to its parent's top.
  float GetY() const;

  // Returns true if the widget is animating.
  bool IsAnimating() const;

  // Returns true if the widget is hidden.
  bool IsHidden() const;

  // Redraws the widget view containing this widget. Note that all widgets
  // belonged to the same widget view will be drawn by calling this method.
  // If the current widget doesn't belong to any widget view, nothing happened.
  void Redraw();

  // Sets the bounds of the view in points.
  void SetBounds(const int x, const int y, const int width, const int height);

  // Sets the height with the specified unit.
  void SetHeight(const Unit unit, const float height);

  // Sets whether the widget should be visible.
  void SetHidden(const bool hidden);

  // Sets the width with the specified unit.
  void SetWidth(const Unit unit, const float width);

  // Sets the horizontal position. The alignment could only be one of kLeft,
  // kCenter, and kRight.
  void SetX(const Alignment alignment, const Unit unit, const float x);

  // Sets the vertical position. The alignment could only be one of kTop,
  // kMiddle, and kBottom.
  void SetY(const Alignment alignment, const Unit unit, const float y);

  // Starts updating the widget synchronized to the refresh rate of the display
  // continuously. StopAnimation() must be called for each StartAnimation().
  // If the widget is not yet attached to a WidgetView object, nothing happened.
  void StartAnimation();

  // Ends previous StartAnimation() call. The animation will actually stop if
  // all StartAnimation() calls are ended. If the widget is not yet attached
  // to a WidgetView object, nothing happened.
  void StopAnimation();

  // Setters and accessors.
  NVGcolor background_color() const { return background_color_; }
  void set_background_color(const float red, const float green,
                            const float blue, const float alpha) {
    background_color_ = nvgRGBA(red, green, blue, alpha);
  }
  std::vector<Widget*>& children() { return children_; }
  bool is_opaque() const { return is_opaque_; }
  void set_is_opaque(const bool is_opaque) { is_opaque_ = is_opaque; }
  Widget* parent() const { return parent_; }
  float scale() const { return scale_; }
  void set_scale(const float scale) { scale_ = scale; }
  WidgetView* widget_view() const { return widget_view_; }

 protected:
  // Initializes the environemnt for rendering in the passed framebuffer.
  // A new framebuffer will be created automatically if *framebuffer is nullptr.
  // EndRenderbufferUpdates() should be called when updates finished.
  void BeginRenderbufferUpdates(NVGcontext* context,
                                NVGLUframebuffer** framebuffer);

  // Ends the framebuffer environment previously created by
  // BeginRenderbufferUpdates()
  void EndRenderbufferUpdates();

  // The weak reference to nanovg context that will be updated by WidgetView
  // and is guaranteed to be available in all render method variants.
  NVGcontext* context_;

 private:
  friend class WidgetView;

  // This method will get called when context_ is about to change. Subclasses
  // can implmenet this method to free nanovg objects binded to it.
  virtual void ContextWillChange(NVGcontext* context) {};

  // This method gets called when the widget received an event. In order to
  // receive an event, the ShouldHandleEvent() method must return true.
  // The actual implmentation should be done in subclass and the passed event
  // object will be deallocated automatically. If the returned value is false,
  // the event stops propagating to the next resonder.
  //
  // Note that this method should only be called in the
  // WidgetView::HandleEvent() method.
  virtual bool HandleEvent(Event* event) { return false; }

  // Implements the logic for rendering the widget. The actual implementation
  // should be done in subclass.
  //
  // Note that this method should only be called in the
  // WidgetView::RenderWidget() method.
  virtual void Render(NVGcontext* context) {}

  // Renders the background color before executing Render() if the widget
  // is opaque.
  void RenderBackgroundColor(NVGcontext* context);

  // Renders Render() in default_framebuffer_ if caches_rendering_ is true.
  void RenderDefaultFramebuffer(NVGcontext* context);

  // Either renders Render() directly or renders default_framebuffer_ if
  // caches_rendering_ is true.
  void RenderOnDemand(NVGcontext* context);

  // This method gets called when an event is about to occur. The returned
  // boolean indicates whether the widget should handle the event. By default
  // it returns false and simply ignores any event. This method could be
  // implemented in the subclass to change the default behavior.
  virtual bool ShouldHandleEvent(const Point location);

  // Updates inherited attributes recursively from the passed widget to all of
  // its children.
  void UpdateChildrenRecursively(Widget* widget);

  // Updates the internal context_.
  void UpdateContext(NVGcontext* context);

  // This method gets called when the widget itself and all of its child
  // widgets did finish rendering in a refresh cycle. It's a good place to
  // restore the context state if changed in `WidgetWillRender()`. However,
  // this method is not called if the widget is not visible on screen in a
  // refresh cycle.
  virtual void WidgetDidRender(NVGcontext* context) {};

  // This method gets called when the corresponded widget view finished
  // rendering all visible widgets in a refresh cycle. This method is always
  // called as long as the widget is attached to a widget view. Because this
  // method is called after all necessary widgets are rendered, which makes
  // it a good place to take snapshot of the rendered stuff.
  virtual void WidgetViewDidRender(NVGcontext* context) {};

  // This method gets called when the corresponded widget view is about to
  // render widgets but has not started the rendering process yet. It's a good
  // place to change the widget's position and dimenstion. This method is
  // always called as long as the widget is attached to a widget view.
  //
  // This method also provides an opportunity for offscreen rendering.
  // A typical use is to create a framebuffer and rendering there. Once done,
  // the framebuffer can be drawn in Render().
  //
  // An example to implement this method for offscreen rendering:
  //    NVGLUframebuffer* framebuffer;  // usually defined as a class member
  //    BeginRenderbufferUpdates(context, &framebuffer);
  //    nvgBeginFrame(context, GetWidth(), GetHeight(), ScreenScaleFactor);
  //    ...
  //    nvgEndFrame(context);
  //    EndRenderbufferUpdates();
  virtual void WidgetViewWillRender(NVGcontext* context) {}

  // This method gets called right before the corresponded widget view calling
  // the render() method. Transformation made in this method not only applies
  // to the widget itself but also applies to its child widgets. However, this
  // method is not called if the widget is not visible on screen in a refresh
  // cycle.
  virtual void WidgetWillRender(NVGcontext* context) {};

  //  Sets the parent.
  void set_parent(Widget* parent) { parent_ = parent; }

  // This setter should only be called by the WidgetView firend class.
  void set_widget_view(WidgetView* widget_view) { widget_view_ = widget_view; }

  // The number of animation reuqests that updated by StartAnimation() and
  // StopAnimation(). The widget is animating if this value is greater than 0.
  int animation_count_;

  // The background color of the widget that will be rendered automatically if
  // is_opaque_ is true. The default color is white.
  NVGcolor background_color_;

  // Indicates whether the rendering in Render() should be rendered in
  // `default_framebuffer_` to cache the rendering result. If true, Render()
  // won't be executed every time the corresponded WidgetView redraws. This
  // value should be decided to meet the expected rendering design.
  const bool caches_rendering_;

  // Holds a list of child widgets.
  std::vector<Widget*> children_;

  // The default framebuffer for rendering Render() if caches_rendering_ is
  // true.
  NVGLUframebuffer* default_framebuffer_;

  // The mutex for making the mechanism of updating default_framebuffer_
  // thread-safe.
  std::mutex* default_framebuffer_mutex_;

  // The NVGpaint object corresonded to the default_framebuffer_.
  NVGpaint default_framebuffer_paint_;

  // The unit of the height_value_.
  Unit height_unit_;

  // The height value represented as height_unit_.
  float height_value_;

  // Indicates whether the widget is hidden.
  bool hidden_;

  // Indicates whether the widget is opaque. If true, the background color
  // will be filled to the entire bounding rectangle. The default value is true.
  bool is_opaque_;

  // The parent widget of the current widget.
  Widget* parent_;

  // The scale of the widget to render. This value should always be positive.
  // The default value is 1.
  float scale_;

  // Indicates whether the default_framebuffer_ should be drawn.
  bool should_redraw_default_framebuffer_;

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
