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

  // The constants to alter the default way to calculate width and height of
  // the widgets.
  enum class BoxSizing {
    // The width and height are measureed in consideration of only the content,
    // but not the padding. This is the default value.
    kContentBox,
    // The width and height are measured in consideratino of the padding.
    kBorderBox,
  };

  Widget();
  explicit Widget(const bool caches_rendering);
  virtual ~Widget();

  // Adds child widget.
  virtual void AddChild(Widget* child);

  // Binds a function or class method for rendering the widget. Calling this
  // method repeatedly will replace the previous binded one.
  //
  // Examples:
  // BindRenderFunction(Function)  // function
  // BindRenderFunction(&Class::Method)  // class method
  template<class Callback>
  void BindRenderFunction(Callback&& callback) {
    render_function_ = std::bind(callback, std::placeholders::_1,
                                 std::placeholders::_2);
    Redraw();
  }

  // Binds an instance method for rendering the widget. Calling this method
  // repeatedly will replace the previous binded one.
  //
  // Example: BindRenderFunction(&Class::Method, instance)
  template<class Callback, class TargetType>
  void BindRenderFunction(Callback&& callback, TargetType&& target) {
    render_function_ = std::bind(callback, target, std::placeholders::_1,
                                 std::placeholders::_2);
    Redraw();
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

  // Returns the widget's alpha value related to the corresponded widget view.
  float GetMeasuredAlpha();

  // Determines the widget's origin and size that related to the corresponded
  // widget view's coordinate system. `nullptr` could be passed as parameter
  // if the value doesn't matter.
  void GetMeasuredBounds(Point* origin, Size* size);

  // Returns the widget's scale that related to the corresponded widget view's
  // coordinate system.
  float GetMeasuredScale();

  // Gets the minimum space required to render the enitre widget including the
  // configured horizontal and vertical offsets.
  void GetOccupiedSpace(Size* size) const;

  // Returns the scaled height in points.
  float GetScaledHeight() const;

  // Returns the scaled width in points.
  float GetScaledWidth() const;

  // Returns the snapshot of the widget including all of its descendants
  // that respect both the screen sacle factor and the `scale_` property,
  // and each pixel is represented by 4 consective bytes in the RGBA format.
  // The returned data needs to be freed manually by `std::free()`.
  virtual unsigned char* GetSnapshot();

  // Returns the width in points.
  float GetWidth() const;

  // Returns the horizontal position in points related to its parent's left.
  float GetX() const;

  // Returns the vertical position in points related to its parent's top.
  float GetY() const;

  // This methods gets called when the application receives a memory warning.
  // The overriding method in subclasses should always call the same method
  // defined in its super class as there are some default behaviors
  // implemented in this base class.
  virtual void HandleMemoryWarning(NVGcontext* context);

  // Inserts a child view above another view in the view hierarchy.
  // Returns `false` on failure.
  bool InsertChildAboveSibling(Widget* child, Widget* sibling);

  // Inserts a child view below another view in the view hierarchy.
  // Returns `false` on failure.
  bool InsertChildBelowSibling(Widget* child, Widget* sibling);

  // Returns true if the widget is animating.
  bool IsAnimating() const;

  // Returns `true` if the widget is a child of a given `parent` widget.
  bool IsChild(Widget* parent);

  // Returns true if the widget is hidden.
  bool IsHidden() const;

  // Resets the `default_framebuffer_` and asks the corresponded `widget_view_`
  // to redraw. This widget will actually be drawn immediately if it's
  // currently visible.
  virtual void Redraw();

  // Unlinks the widget from its real parent and removes it from the responder
  // chain. Returns `false` on failure.
  bool RemoveFromParent();

  // Returns `true` if the render function is binded.
  bool RenderFunctionIsBinded() const;

  // Resets the context for the widget and its descendants.
  void ResetContext(NVGcontext* context);

  // Moves the specified child so that it appears beind its siblings.
  bool SendChildToBack(Widget* child);

  // Sets the bounds of the view in points.
  void SetBounds(const float x, const float y, const float width,
                 const float height);

  // Sets the height in points.
  void SetHeight(const float height);

  // Sets the height with the specified unit.
  void SetHeight(const Unit unit, const float height);

  // Sets whether the widget should be visible.
  void SetHidden(const bool hidden);

  // Sets the padding for horizontal and vertical sides.
  void SetPadding(const float vertical_padding, const float horizontal_padding);

  // Sets the padding at every side.
  void SetPadding(const float padding);

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

  // Releases the specified widget and its children if `auto_release_children`
  // is `true`.
  static void SmartRelease(moui::Widget* widget);

  // Starts updating the widget synchronized to the refresh rate of the display
  // continuously. If the widget is not yet attached to a `WidgetView` object,
  // nothing happened.
  void StartAnimation();

  // Stops animation. If `force` is `true`, animation stops immediately no
  // matter how many times `StartAnimation()` is called. Otherwise, the same
  // number of
  void StopAnimation(const bool force);

  // Unbinds the render function.
  void UnbindRenderFunction();

  // Setters and accessors.
  float alpha() const { return alpha_; }
  void set_alpha(const float alpha);
  bool auto_release_children() const {
    return auto_release_children_;
  }
  void set_auto_release_children(const bool value) {
    auto_release_children_ = value;
  }
  NVGcolor background_color() const { return background_color_; }
  void set_background_color(const NVGcolor background_color);
  virtual float bottom_padding() const { return bottom_padding_; }
  virtual void set_bottom_padding(const float padding);
  virtual BoxSizing box_sizing() const { return box_sizing_; }
  virtual void set_box_sizing(const BoxSizing box_sizing);
  std::vector<Widget*>* children() { return &children_; }
  virtual float left_padding() const { return left_padding_; }
  virtual void set_left_padding(const float padding);
  bool is_opaque() const { return is_opaque_; }
  void set_is_opaque(const bool is_opaque) { is_opaque_ = is_opaque; }
  bool is_visible() const { return is_visible_; }
  Widget* parent() const { return parent_; }
  void set_parent(Widget* parent) { parent_ = parent; }
  Point rendering_offset() const { return rendering_offset_; }
  void set_rendering_offset(const Point offset);
  float rendering_scale() const { return rendering_scale_; }
  void set_rendering_scale(const float rendering_scale);
  virtual float right_padding() const { return right_padding_; }
  virtual void set_right_padding(const float padding);
  float scale() const { return scale_; }
  void set_scale(const float scale);
  int tag() const { return tag_; }
  void set_tag(const int tag) { tag_ = tag; }
  virtual float top_padding() const { return top_padding_; }
  virtual void set_top_padding(const float padding);
  WidgetView* widget_view() const { return widget_view_; }

 protected:
  // Initializes the environment for rendering in the passed framebuffer.
  // Returns `false` on failure. If successful, a new framebuffer will be
  // created automatically if `*framebuffer` is `nullptr`, and
  // `EndFramebufferUpdates()` must be called when finished rendering.
  //
  // This method returns the `scale_factor` that used to create the framebufer.
  // It is calculated based on the screen's scale factor and the widget's
  // current scale that related to the corresponded widget view's coordinate
  // system. Passing this value to the `nvgBeginFrame()` function can
  // generate the best quality. If this value doesn't matter, simply passing
  // `nullptr` as the `scale_factor` parameter.
  bool BeginFramebufferUpdates(NVGcontext* context,
                               NVGframebuffer** framebuffer,
                               const float width, const float height,
                               float* scale_factor);

  // Initializes the environment for rendering in the passed framebuffer based
  // on the widget's current size.
  bool BeginFramebufferUpdates(NVGcontext* context,
                               NVGframebuffer** framebuffer,
                               float* scale_factor);

  // This method will get called when a new context is assigned to the widget.
  // It's a good place to allocate context-related resources in subclasses.
  virtual void ContextDidChange(NVGcontext* context) {}

  // This method will get called when the corresponed nanovg context is about
  // to change. Subclasses should implement this method to free related
  // nanovg objects if there is any. Besides, the overriding method should
  // always calls the same method defined in its super class as there are some
  // default behaviors implemented in this base class.
  virtual void ContextWillChange(NVGcontext* context);

  // Ends the framebuffer environment previously created by
  // `BeginFramebufferUpdates()`.
  void EndFramebufferUpdates();

  // This method gets called when the widget received an event. In order to
  // receive an event, the `ShouldHandleEvent()` method must return `true`.
  // The actual implementation should be done in subclass and the passed event
  // object will be deallocated automatically. If the returned value is
  // `false`, the event will stop propagating to the next responder.
  //
  // Note that this method should only be called in the
  // `WidgetView::HandleEvent()` method.
  virtual bool HandleEvent(Event* event) { return false; }

  // Releases the widget ifself and its direct children on demand.
  void ReleaseSelfAndChildrenOnDemand();

  // Implements the logic for rendering the widget. The actual implementation
  // should be done in subclass. Note that this method should only be called by
  // `WidgetView::RenderWidget()`.
  virtual void Render(NVGcontext* context) {}

  // Allows subclasses to render offscreen stuff into custom framebuffers. This
  // method gets called before executing `RenderDefaultFramebuffer()`. Note
  // that this method should only be called by `WidgetView::RenderWidget()`.
  //
  // An example to implement this method for offscreen rendering:
  //
  //    NVGframebuffer* framebuffer;  // usually defined as a class member
  //    float scale_factor;
  //    BeginFramebufferUpdates(context, &framebuffer, &scale_factor);
  //    nvgBeginFrame(context, GetWidth(), GetHeight(), scale_factor);
  //    ...
  //    nvgEndFrame(context);
  //    EndFramebufferUpdates();
  virtual void RenderFramebuffer(NVGcontext* context) {}

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
  // always called as long as the widget is attached to a widget view.
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

 private:
  friend class WidgetView;

  // Executes either the binded `render_function_` or `Render()` if no render
  // function is binded. This method respects the `rendering_offset_` and it
  // also fills the background color if the widget is opaque.
  void ExecuteRenderFunction(NVGcontext* context);

  // Notifies that the corresponded context has been changed. This method
  // would call `ContextWillChange()` and `ContextDidChange()` on demand.
  void NotifyContextChange(NVGcontext* old_context, NVGcontext* new_context);

  // Returns `true` if the passed widget is removed from children. This method
  // is designed for internal use. To remove a child from a parent widget.
  // Calls the child widget's `RemoveFromParent()` method instead.
  bool RemoveChild(Widget* child);

  // Renders `Render()` in `default_framebuffer_` if `caches_rendering_` is
  // true. Note that this method should only be called by
  // `WidgetView::RenderWidget()`.
  void RenderDefaultFramebuffer(NVGcontext* context);

  // Either renders `Render()` directly or renders `default_framebuffer_` if
  // `caches_rendering_` is true.
  void RenderOnDemand(NVGcontext* context);

  // Resets the `measured_scale_` property so the value will be re-calculated
  // the next time calling `GetMeasuredScale()`.
  void ResetMeasuredScale();

  // Resets the measured scale for the passed widget and all of its descendants
  // recursively.
  void ResetMeasuredScaleRecursively(Widget* widget);

  // This setters that should only be called by the `WidgetView` class.
  void set_is_visible(const bool is_visible);
  void set_widget_view(WidgetView* widget_view);

  // The opacity value of the widget, specified as a value from 0.0 to 1.0.
  // Values below 0.0 are interpreted as 0.0, and values above 1.0 are
  // interpreted as 1.0. The default value is 1.0.
  float alpha_;

  // The number of animation reuqests that updated by `StartAnimation()` and
  // `StopAnimation()`. The widget is animating if this value is greater than 0.
  int animation_count_;

  // Indicates whether release children when calling `SmartRelease()`.
  // The default value is `false`.
  bool auto_release_children_;

  // The background color of the widget that will be rendered automatically if
  // `is_opaque_` is true. The default color is white.
  NVGcolor background_color_;

  // The padding in points on the bottom side of the widget.
  float bottom_padding_;

  // Indicators the behavior used when calculating the width and height of the
  // widget.
  BoxSizing box_sizing_;

  // Indicates whether the rendering logic implemented in `Render()` should be
  // rendered in `default_framebuffer_` to cache the rendering result. If
  // `true`, `Render()` won't be executed every time the corresponded
  // `WidgetView` redraws.
  const bool caches_rendering_;

  // Holds a list of child widgets.
  std::vector<Widget*> children_;

  // The framebuffer to save the rendering result of `Render()` when
  // `caches_rendering_` is set to `true`.
  NVGframebuffer* default_framebuffer_;

  // The `NVGpaint` object corresonded to the `default_framebuffer_`.
  NVGpaint default_framebuffer_paint_;

  // The unit of the `height_value_`.
  Unit height_unit_;

  // The height value represented as `height_unit_`.
  float height_value_;

  // Indicates whether the widget is hidden.
  bool hidden_;

  // Indicates whether the widget is opaque. If `true`, the background color
  // will be filled to the entire bounding rectangle. The default value is
  // `true`.
  bool is_opaque_;

  // Indicates if the widget is visible to the corresponded widget view.
  bool is_visible_;

  // The padding in points on the left side of the widget.
  float left_padding_;

  // Keeps the calculated scale related to the corresponded widget view's
  // coordinate system. This property should never be accessed directly.
  // Instead, calling the `GetMeasuredScale()` method to retrieve this value
  // and calling `ResetMeasuredScale()` to reset this value.
  float measured_scale_;

  // Keeps the pointer to the logical parent widget of the current widget. The
  // logical parent can be changed through `set_parent()` in inherited widgets
  // whenever needed.
  Widget* parent_;

  // Indicates if the animation has been paused for the widget due to its
  // currently invisible to the corresponded widget view..
  bool paused_animation_;

  // Keeps the real parent widget of the current widget. Unlike the logical
  // `parent_` property. This value is always pointing to the real parent
  // widget and cannot be changed manually.
  Widget* real_parent_;

  // Keeps the binded render function to replace `Render()`. This value can
  // be set through `BindRenderFunction()` and `UnbindRenderFunction()`.
  std::function<void(Widget*, NVGcontext*)> render_function_;

  // The offset related to the widget's origin as the real origin for rendering
  // the binded render function or the `Render() method.
  Point rendering_offset_;

  // The scale applied when rendering the binded render function or the
  // `Render()` method. The default value is 1.
  float rendering_scale_;

  // The padding in points on the right side of the widget.
  float right_padding_;

  // The scale of the widget to render. This value should always be positive.
  // The default value is 1.
  float scale_;

  // Indicates whether the `default_framebuffer_` should be drawn.
  bool should_redraw_default_framebuffer_;

  // This property can be set to an arbitrary integer and use that number to
  // identify the widget later. The default value is 0.
  int tag_;

  // The padding in points on the top side of the widget.
  float top_padding_;

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
