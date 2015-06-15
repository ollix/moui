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

#include <functional>
#include <string>
#include <vector>

#include "moui/base.h"
#include "moui/nanovg_hook.h"

namespace moui {

class Event;
class WidgetView;

// The `Widget` class represents a graphical element that can be displayed on
// the screen.
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

  // Binds a function or class method for rendering the widget. Calling this
  // method repeatedly will replace the previous binded one.
  //
  // Examples:
  // BindRenderFunction(Function)  // function
  // BindRenderFunction(&Class::Method)  // class method
  template<class Callback>
  void BindRenderFunction(Callback&& callback) {
    render_function_ = std::bind(callback, std::placeholders::_1);
  }

  // Binds an instance method for rendering the widget. Calling this method
  // repeatedly will replace the previous binded one.
  //
  // Example: BindRenderFunction(&Class::Method, instance)
  template<class Callback, class TargetType>
  void BindRenderFunction(Callback&& callback, TargetType&& target) {
    render_function_ = std::bind(callback, target, std::placeholders::_1);
  }

  // Moves the specified child widget so that it appears on top of its siblings.
  // Returns `false` if the specified child is not one of its children.
  bool BringChildToFront(Widget* child);

  // Returns `true` if the passed point is within the region of the widget's
  // bounding box plus the passed padding at every direction.
  bool CollidePoint(const Point point, const float padding);

  // Returns `true` if the passed point is within the region of the widget's
  // bounding box plus the paddings at various direction.
  bool CollidePoint(const Point point, const float top_padding,
                    const float right_padding, const float bottom_padding,
                    const float left_padding);

  // Returns the height in points.
  float GetHeight() const;

  // Determines the widget's origin and size that related to the corresponded
  // widget view's coordinate system. `nullptr` could be passed as parameter
  // if the value doesn't matter.
  void GetMeasuredBounds(Point* origin, Size* size);

  // Returns the widget's scale that related to the corresponded widget view's
  // coordinate system.
  float GetMeasuredScale();

  // Returns the scaled height in points.
  float GetScaledHeight() const;

  // Returns the scaled width in points.
  float GetScaledWidth() const;

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

  // Unlinks the widget from its real parent and removes it from the responder
  // chain. Returns `false` on failure.
  bool RemoveFromParent();

  // Returns `true` if the render function is binded.
  bool RenderFunctionIsBinded() const;

  // Sets the bounds of the view in points.
  void SetBounds(const float x, const float y, const float width,
                 const float height);

  // Sets the height in points.
  void SetHeight(const float height);

  // Sets the height with the specified unit.
  void SetHeight(const Unit unit, const float height);

  // Sets whether the widget should be visible.
  void SetHidden(const bool hidden);

  // Sets the width in points.
  void SetWidth(const float width);

  // Sets the width with the specified unit.
  void SetWidth(const Unit unit, const float width);

  // Sets the horizontal position. The alignment could only be one of `kLeft`,
  // `kCenter`, and `kRight`.
  void SetX(const Alignment alignment, const Unit unit, const float x);

  // Sets the horizontal position to the parent's origin in points.
  void SetX(const float x);

  // Sets the vertical position. The alignment could only be one of `kTop`,
  // `kMiddle`, and `kBottom`.
  void SetY(const Alignment alignment, const Unit unit, const float y);

  // Sets the vertical position to the parent's origin in points.
  void SetY(const float y);

  // Starts updating the widget synchronized to the refresh rate of the display
  // continuously. `StopAnimation()` must be called after every
  // `StartAnimation()` call . If the widget is not yet attached to a
  // `WidgetView` object, nothing happened.
  void StartAnimation();

  // Ends previous `StartAnimation()` call. The animation will actually stop if
  // all `StartAnimation()` calls are stopped. If the widget is not yet
  // attached to a `WidgetView` object, nothing happened.
  void StopAnimation();

  // Unbinds the render function.
  void UnbindRenderFunction();

  // Setters and accessors.
  float alpha() const { return alpha_; }
  void set_alpha(const float alpha);
  NVGcolor background_color() const { return background_color_; }
  void set_background_color(const NVGcolor background_color);
  std::vector<Widget*>& children() { return children_; }
  bool frees_descendants_on_destruction() const {
    return frees_descendants_on_destruction_;
  }
  void set_frees_descendants_on_destruction(const bool value) {
    frees_descendants_on_destruction_ = value;
  }
  bool is_opaque() const { return is_opaque_; }
  void set_is_opaque(const bool is_opaque) { is_opaque_ = is_opaque; }
  Widget* parent() const { return parent_; }
  Point rendering_offset() const { return rendering_offset_; }
  void set_rendering_offset(const Point offset);
  float scale() const { return scale_; }
  void set_scale(const float scale);
  int tag() const { return tag_; }
  void set_tag(const int tag) { tag_ = tag; }
  WidgetView* widget_view() const { return widget_view_; }

 protected:
  // Initializes the environment for rendering in the passed framebuffer.
  // Returns false on failure. If successful, a new framebuffer will be created
  // automatically if `*framebuffer` is `nullptr`, and
  // `EndFramebufferUpdates()` must be called when finished rendering.
  //
  // This method returns the `scale_factor` that used to create the framebufer.
  // It is calculated based on the screen's scale factor and the widget's
  // current scale that related to the corresponded widget view's coordinate
  // system. Passing this value to the `nvgBeginFrame()` function can
  // generate the best quality. If this value doesn't matter, simply passing
  // `nullptr` can as the `scale_factor` parameter.
  bool BeginFramebufferUpdates(NVGcontext* context,
                               NVGLUframebuffer** framebuffer,
                               float* scale_factor);

  // This method will get called when the corresponed nanovg context is about
  // to change. Subclasses should implement this method to free related
  // nanovg objects if there is any. Besides, the overriding method should
  // always call the same method defined in its super class as there are some
  // default behaviors implemented in this base class.
  virtual void ContextWillChange(NVGcontext* context);

  // Ends the framebuffer environment previously created by
  // `BeginFramebufferUpdates()`.
  void EndFramebufferUpdates();

 private:
  friend class ScrollView;
  friend class WidgetView;

  // Executes either the binded `render_function_` or `Render()` if no render
  // function is binded. This method respects the `rendering_offset_` and it
  // also fills the background color if the widget is opaque.
  void ExecuteRenderFunction(NVGcontext* context);

  // This method gets called when the widget received an event. In order to
  // receive an event, the `ShouldHandleEvent()` method must return true.
  // The actual implementation should be done in subclass and the passed event
  // object will be deallocated automatically. If the returned value is false,
  // the event stops propagating to the next resonder.
  //
  // Note that this method should only be called in the
  // `WidgetView::HandleEvent()` method.
  virtual bool HandleEvent(Event* event) { return false; }

  // Returns `true` if the passed widget is removed from children. This method
  // is designed for internal use. To remove a child from a parent widget.
  // Calls the child widget's `RemoveFromParent()` method instead.
  bool RemoveChild(Widget* child);

  // Implements the logic for rendering the widget. The actual implementation
  // should be done in subclass. Note that this method should only be called by
  // `WidgetView::RenderWidget()`.
  virtual void Render(NVGcontext* context) {}

  // Renders `Render()` in `default_framebuffer_` if `caches_rendering_` is
  // true. Note that this method should only be called by
  // `WidgetView::RenderWidget()`.
  void RenderDefaultFramebuffer(NVGcontext* context);

  // Allows subclasses to render offscreen stuff into custom framebuffers. This
  // method gets called before executing `RenderDefaultFramebuffer()`. Note
  // that this method should only be called by `WidgetView::RenderWidget()`.
  //
  // An example to implement this method for offscreen rendering:
  //
  //    NVGLUframebuffer* framebuffer;  // usually defined as a class member
  //    float scale_factor;
  //    BeginFramebufferUpdates(context, &framebuffer, &scale_factor);
  //    nvgBeginFrame(context, GetWidth(), GetHeight(), scale_factor);
  //    ...
  //    nvgEndFrame(context);
  //    EndFramebufferUpdates();
  virtual void RenderFramebuffer(NVGcontext* context) {}

  // Either renders `Render()` directly or renders `default_framebuffer_` if
  // `caches_rendering_` is true.
  void RenderOnDemand(NVGcontext* context);

  // Resets the `measured_scale_` property so the value will be re-calculated
  // the next time calling `GetMeasuredScale()`.
  void ResetMeasuredScale();

  // Resets the measured scale for the passed widget and all of its descendants
  // recursively.
  void ResetMeasuredScaleRecursively(Widget* widget);

  // This method gets called when an event is about to occur. The returned
  // boolean indicates whether the widget should handle the event. By default
  // it returns `false` and simply ignores any event. This method could be
  // implemented in the subclass to change the default behavior.
  virtual bool ShouldHandleEvent(const Point location);

  // This method gets called when the widget itself and all of its child
  // widgets did finish rendering in a refresh cycle. It's a good place to
  // restore the context state if changed in `WidgetWillRender()`. However,
  // this method is not called if the widget is not visible on screen in a
  // refresh cycle.
  virtual void WidgetDidRender(NVGcontext* context) {}

  // This method gets called when the corresponded widget view finished
  // rendering all visible widgets in a refresh cycle. This method is always
  // called as long as the widget is attached to a widget view. Because this
  // method is called after all necessary widgets are rendered, which makes
  // it a good place to take snapshot of the rendered stuff.
  virtual void WidgetViewDidRender(NVGcontext* context) {}

  // This method gets called when the corresponded widget view is about to
  // render widgets but has not started the rendering process yet. It's a good
  // place to change the widget's position and dimensions. This method is
  // always called as long as the widget is attached to a widget view and is
  // not hidden.
  //
  // This method requires a boolean value to be returned. The returned value
  // indicates whether the widget is ready to render. If `false`, the
  // corresponded widget view will iterate all of this widget's children
  // to call their `WidgetViewWillRender()` method repeatedly until this
  // widget returns `true`.
  virtual bool WidgetViewWillRender(NVGcontext* context) { return true; }

  // This method gets called right before the corresponded widget view calling
  // the `render()` method. Transformation made in this method not only applies
  // to the widget itself but also applies to all of its child widgets.
  // However, this method is not called if the widget is not visible on screen
  // in a refresh cycle.
  virtual void WidgetWillRender(NVGcontext* context) {}

  // Sets the logical parent widget.
  void set_parent(Widget* parent) { parent_ = parent; }

  // Setter and accessor for the responder chain. The accessor is created for
  // the `WidgetView` class.
  std::string responder_chain_identifier() const {
    return responder_chain_identifier_;
  }
  void set_responder_chain_identifier(const std::string& identifer) {
    responder_chain_identifier_ = identifer;
  }

  // This setter should only be called by the `WidgetView` class.
  void set_widget_view(WidgetView* widget_view);

  // The opacity value of the widget, specified as a value from 0.0 to 1.0.
  // Values below 0.0 are interpreted as 0.0, and values above 1.0 are
  // interpreted as 1.0. The default value is 1.0.
  float alpha_;

  // The number of animation reuqests that updated by `StartAnimation()` and
  // `StopAnimation()`. The widget is animating if this value is greater than 0.
  int animation_count_;

  // The background color of the widget that will be rendered automatically if
  // `is_opaque_` is true. The default color is white.
  NVGcolor background_color_;

  // Indicates whether the rendering logic implemented in `Render()` should be
  // rendered in `default_framebuffer_` to cache the rendering result. If
  // `true`, `Render()` won't be executed every time the corresponded
  // `WidgetView` redraws.
  const bool caches_rendering_;

  // Holds a list of child widgets.
  std::vector<Widget*> children_;

  // The framebuffer to save the rendering result of `Render()` when
  // `caches_rendering_` is set to `true`.
  NVGLUframebuffer* default_framebuffer_;

  // The `NVGpaint` object corresonded to the `default_framebuffer_`.
  NVGpaint default_framebuffer_paint_;

  // Indicates whether all the widget's descendants should be freed when
  // executing the widget's destructor. The default value is `false`.
  bool frees_descendants_on_destruction_;

  // The unit of the `height_value_`.
  Unit height_unit_;

  // The height value represented as `height_unit_`.
  float height_value_;

  // Indicates whether the widget is hidden.
  bool hidden_;

  // This property can be set to an arbitrary integer and use that number to
  // identify the widget later. The default value is 0.
  int tag_;

  // Indicates whether the widget is opaque. If `true`, the background color
  // will be filled to the entire bounding rectangle. The default value is
  // `true`.
  bool is_opaque_;

  // Keeps the calculated scale related to the corresponded widget view's
  // coordinate system. This property should never be accessed directly.
  // Instead, calling the `GetMeasuredScale()` method to retrieve this value
  // and calling `ResetMeasuredScale()` to reset this value.
  float measured_scale_;

  // Keeps the pointer to the logical parent widget of the current widget. The
  // logical parent can be changed through `set_parent()` in inherited widgets
  // whenever needed.
  Widget* parent_;

  // Keeps the real parent widget of the current widget. Unlike the logical
  // `parent_` property. This value is always pointing to the real parent
  // widget and cannot be changed manually.
  Widget* real_parent_;

  // Keeps the binded render function to replace `Render()`. This value can
  // be set through `BindRenderFunction()` and `UnbindRenderFunction()`.
  std::function<void(NVGcontext*)> render_function_;

  // The offset related to the widget's origin as the real origin for rendering
  // the binded render function or the `Render() method.
  Point rendering_offset_;

  // The identifier that is used to determine which widget in the responder
  // chain should be the next one to handle the received event. The actual
  // mechanism is implemented in the `WidgetView::HandleEvent()` method.
  std::string responder_chain_identifier_;

  // The scale of the widget to render. This value should always be positive.
  // The default value is 1.
  float scale_;

  // Indicates whether the `default_framebuffer_` should be drawn.
  bool should_redraw_default_framebuffer_;

  // The `WidgetView` that manages this widget instance.
  WidgetView* widget_view_;

  // The unit of the `width_value_`.
  Unit width_unit_;

  // The width value represented as `width_unit_`.
  float width_value_;

  // The alignment of the `x_value_`.
  Alignment x_alignment_;

  // The unit of the `x_value_`.
  Unit x_unit_;

  // The horizontal position related to its parent.
  float x_value_;

  // The alignment of the `y_value_`.
  Alignment y_alignment_;

  // The unit of the `y_value_`.
  Unit y_unit_;

  // The vertical position related to its parent.
  float y_value_;

  DISALLOW_COPY_AND_ASSIGN(Widget);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_WIDGET_H_
