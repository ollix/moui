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

#include "moui/widgets/widget.h"

#include <algorithm>
#include <cassert>
#include <stack>
#include <vector>

#include "moui/core/device.h"
#include "moui/core/event.h"
#include "moui/widgets/widget_view.h"

namespace {

// Returns the actual length in points of the specified value and unit.
float CalculatePoints(const moui::Widget::Unit unit, const float value,
                      const float parent_length) {
  float points;
  switch (unit) {
    case moui::Widget::Unit::kPercent:
      points = value / 100.0 * parent_length;
      break;
    case moui::Widget::Unit::kPoint:
      points = value;
      break;
    default:
      assert(false);
  }
  return points;
}

// Frees all descendant widgets of the passed widget recursively. This
// function is created for the widget's destructor. Note that the passed
// root widget itself is not freed.
void FreeDescendantsRecursively(moui::Widget* widget) {
  for (moui::Widget* child : widget->children()) {
    FreeDescendantsRecursively(child);
    delete child;
  }
}

}  // namespace

namespace moui {

Widget::Widget() : Widget(true) {
}

Widget::Widget(const bool caches_rendering)
    : alpha_(1), animation_count_(0),
      background_color_(nvgRGBA(255, 255, 255, 255)),
      caches_rendering_(caches_rendering), default_framebuffer_(nullptr),
      frees_descendants_on_destruction_(false), height_unit_(Unit::kPoint),
      height_value_(0), hidden_(false), is_opaque_(true), measured_scale_(-1),
      parent_(nullptr), real_parent_(nullptr), render_function_(NULL),
      rendering_offset_({0, 0}), scale_(1),
      should_redraw_default_framebuffer_(false), tag_(0),
      widget_view_(nullptr), width_unit_(Unit::kPoint), width_value_(0),
      x_alignment_(Alignment::kLeft), x_unit_(Unit::kPoint), x_value_(0),
      y_alignment_(Alignment::kTop), y_unit_(Unit::kPoint), y_value_(0) {
}

Widget::~Widget() {
  set_widget_view(nullptr);

  while (IsAnimating())
    StopAnimation();

  if (frees_descendants_on_destruction_)
    FreeDescendantsRecursively(this);
}

void Widget::AddChild(Widget* child) {
  if (child->real_parent_ == this)
    return;

  child->parent_ = this;
  child->real_parent_ = this;
  child->set_widget_view(widget_view_);
  children_.push_back(child);
  if (widget_view_ != nullptr && !child->IsHidden())
    widget_view_->Redraw();
}

bool Widget::BeginFramebufferUpdates(NVGcontext* context,
                                     NVGLUframebuffer** framebuffer,
                                     float* scale_factor) {
  const float kScaleFactor = \
      Device::GetScreenScaleFactor() * GetMeasuredScale();
  const float kWidth = GetWidth() * kScaleFactor;
  const float kHeight = GetHeight() * kScaleFactor;
  if (kWidth <= 0 || kHeight <= 0)
    return false;

  // Deletes the `framebuffer` if its size is not matched to the expected size.
  if (*framebuffer != nullptr) {
    int framebuffer_width = 0;
    int framebuffer_height = 0;
    nvgImageSize((*framebuffer)->ctx, (*framebuffer)->image,
                 &framebuffer_width, &framebuffer_height);
    if (kWidth != framebuffer_width || kHeight != framebuffer_height)
      moui::nvgDeleteFramebuffer(framebuffer);
  }

  if (*framebuffer == nullptr)
    *framebuffer = nvgluCreateFramebuffer(context, kWidth, kHeight, 0);
  if (*framebuffer == NULL) {
    *framebuffer = nullptr;
    return false;
  }
  if (scale_factor != nullptr)
    *scale_factor = kScaleFactor;
  nvgluBindFramebuffer(*framebuffer);
  glViewport(0, 0, kWidth, kHeight);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  return true;
}

bool Widget::BringChildToFront(Widget* child) {
  auto iterator = std::find(children_.begin(), children_.end(), child);
  if (iterator == children_.end())
    return false;
  children_.erase(iterator);
  children_.push_back(child);
  return true;
}

bool Widget::CollidePoint(const Point point, const float padding) {
  return CollidePoint(point, padding, padding, padding, padding);
}

// This method compares the passed point to the widget's actual origin and
// dimenstion related to the corresponded widget view's coordinate system.
bool Widget::CollidePoint(const Point point, const float top_padding,
                          const float right_padding, const float bottom_padding,
                          const float left_padding) {
  Point origin;
  Size size;
  GetMeasuredBounds(&origin, &size);
  return (point.x >= (origin.x - left_padding) &&
          point.y >= (origin.y - top_padding) &&
          point.x < (origin.x + size.width + right_padding) &&
          point.y < (origin.y + size.height + bottom_padding));
}

void Widget::ContextWillChange(NVGcontext* context) {
  moui::nvgDeleteFramebuffer(&default_framebuffer_);
}

void Widget::EndFramebufferUpdates() {
  nvgluBindFramebuffer(NULL);
}

void Widget::ExecuteRenderFunction(NVGcontext* context) {
  // Fills the background color if the widget is not opaque and the color's
  // alpha value is not 0.
  if (is_opaque_ && background_color_.a > 0) {
    nvgBeginPath(context);
    nvgRect(context, 0, 0, GetWidth(), GetHeight());
    nvgFillColor(context, background_color_);
    nvgFill(context);
  }

  nvgTranslate(context, rendering_offset_.x, rendering_offset_.y);
  render_function_ == NULL ? Render(context) : render_function_(context);
}

float Widget::GetHeight() const {
  const float kParentHeight = parent_ == nullptr ? 0 : parent_->GetHeight();
  return CalculatePoints(height_unit_, height_value_, kParentHeight);
}

void Widget::GetMeasuredBounds(Point* origin, Size* size) {
  if (origin == nullptr && size == nullptr)
    return;

  std::stack<Widget*> widget_chain;
  widget_chain.push(this);
  Widget* parent = real_parent_;
  while (parent != nullptr) {
    widget_chain.push(parent);
    parent = parent->real_parent_;
  }
  float scale = 1;
  if (origin != nullptr)
    *origin = {0.0f, 0.0f};
  while (!widget_chain.empty()) {
    Widget* widget = widget_chain.top();
    widget_chain.pop();
    if (origin != nullptr) {
      origin->x += widget->GetX() * scale;
      origin->y += widget->GetY() * scale;
    }
    scale *= widget->scale();
  }
  if (size != nullptr) {
    size->width = GetWidth() * scale;
    size->height = GetHeight() * scale;
  }
}

float Widget::GetMeasuredScale() {
  if (measured_scale_ < 0) {
    measured_scale_ = scale_;
    Widget* parent = parent_;
    while (parent != nullptr) {
      measured_scale_ *= parent->scale();
      parent = parent->parent();
    }
  }
  return measured_scale_;
}

float Widget::GetScaledHeight() const {
  return GetHeight() * scale_;
}

float Widget::GetScaledWidth() const {
  return GetWidth() * scale_;
}

float Widget::GetWidth() const {
  const float kParentWidth = parent_ == nullptr ? 0 : parent_->GetWidth();
  return CalculatePoints(width_unit_, width_value_, kParentWidth);
}

float Widget::GetX() const {
  const float kParentWidth = real_parent_ == nullptr ?
                             0 : real_parent_->GetWidth();
  const float kOffset = CalculatePoints(x_unit_, x_value_, kParentWidth);
  float x;
  switch (x_alignment_) {
    case Alignment::kLeft:
      x = kOffset;
      break;
    case Alignment::kCenter:
      x = (kParentWidth - GetWidth() * scale_) / 2 + kOffset;
      break;
    case Alignment::kRight:
      x = kParentWidth - GetWidth() * scale_ - kOffset;
      break;
    default:
      assert(false);
  }
  return x;
}

float Widget::GetY() const {
  const float kParentHeight = real_parent_ == nullptr ?
                              0 : real_parent_->GetHeight();
  const float kOffset = CalculatePoints(y_unit_, y_value_, kParentHeight);
  float y;
  switch (y_alignment_) {
    case Alignment::kTop:
      y = kOffset;
      break;
    case Alignment::kMiddle:
      y = (kParentHeight - GetHeight() * scale_) / 2 + kOffset;
      break;
    case Alignment::kBottom:
      y = kParentHeight - GetHeight() * scale_ - kOffset;
      break;
    default:
      assert(false);
  }
  return y;
}

bool Widget::IsAnimating() const {
  return animation_count_ > 0;
}

bool Widget::IsHidden() const {
  return hidden_;
}

void Widget::Redraw() {
  if (caches_rendering_) {
    should_redraw_default_framebuffer_ = true;
  }
  if (widget_view_ != nullptr && !IsHidden())
    widget_view_->Redraw();
}

bool Widget::RemoveChild(Widget* child) {
  auto iterator = std::find(children_.begin(), children_.end(), child);
  if (iterator == children_.end())
    return false;
  children_.erase(iterator);
  return true;
}

bool Widget::RemoveFromParent() {
  if (real_parent_ == nullptr || !real_parent_->RemoveChild(this))
    return false;

  parent_ = nullptr;
  real_parent_ = nullptr;
  set_widget_view(nullptr);
  return true;
}

void Widget::RenderDefaultFramebuffer(NVGcontext* context) {
  if (!caches_rendering_)
    return;

  if (default_framebuffer_ != nullptr && !should_redraw_default_framebuffer_ &&
      !IsAnimating()) {
    return;
  }
  should_redraw_default_framebuffer_ = false;

  float scale_factor;
  if (BeginFramebufferUpdates(context, &default_framebuffer_, &scale_factor)) {
    const float kWidth = GetWidth();
    const float kHeight = GetHeight();
    nvgBeginFrame(context, kWidth, kHeight, scale_factor);
    ExecuteRenderFunction(context);
    nvgEndFrame(context);
    EndFramebufferUpdates();
    default_framebuffer_paint_ = nvgImagePattern(context, 0, kHeight, kWidth,
                                                 kHeight, 0,
                                                 default_framebuffer_->image,
                                                 1);
  }
}

bool Widget::RenderFunctionIsBinded() const {
  return render_function_ != NULL;
}

void Widget::RenderOnDemand(NVGcontext* context) {
  if (caches_rendering_ && default_framebuffer_ != nullptr) {
    nvgBeginPath(context);
    nvgRect(context, 0, 0, GetWidth(), GetHeight());
    nvgFillPaint(context, default_framebuffer_paint_);
    nvgFill(context);
  } else {
    ExecuteRenderFunction(context);
  }
}

void Widget::ResetMeasuredScale() {
  measured_scale_ = -1;
  Redraw();
}

void Widget::ResetMeasuredScaleRecursively(Widget* widget) {
  widget->ResetMeasuredScale();
  for (Widget* child : widget->children())
    ResetMeasuredScaleRecursively(child);
}

void Widget::SetBounds(const float x, const float y, const float width,
                       const float height) {
  SetX(x);
  SetY(y);
  SetWidth(width);
  SetHeight(height);
}

void Widget::SetHeight(const float height) {
  SetHeight(Unit::kPoint, height);
}

void Widget::SetHeight(const Unit unit, const float height) {
  const float kHeight = std::max(0.0f, height);
  if (unit == height_unit_ && kHeight == height_value_)
    return;

  height_unit_ = unit;
  height_value_ = kHeight;
  Redraw();
}

void Widget::SetHidden(const bool hidden) {
  if (hidden != hidden_) {
    hidden_ = hidden;
    if (widget_view_ != nullptr)
      widget_view_->Redraw();
  }
}

void Widget::SetWidth(const float width) {
  SetWidth(Unit::kPoint, width);
}

void Widget::SetWidth(const Unit unit, const float width) {
  const float kWidth = std::max(0.0f, width);
  if (unit == width_unit_ && kWidth == width_value_)
    return;

  width_unit_ = unit;
  width_value_ = kWidth;
  Redraw();
}

void Widget::SetX(const Alignment alignment, const Unit unit, const float x) {
  if (alignment == x_alignment_ && unit == x_unit_ && x == x_value_)
    return;

  x_alignment_ = alignment;
  x_unit_ = unit;
  x_value_ = x;
  if (widget_view_ != nullptr)
    widget_view_->Redraw();
}

void Widget::SetX(const float x) {
  SetX(Alignment::kLeft, Unit::kPoint, x);
}

void Widget::SetY(const Alignment alignment, const Unit unit, const float y) {
  if (alignment == y_alignment_ && unit == y_unit_ && y == y_value_)
    return;

  y_alignment_ = alignment;
  y_unit_ = unit;
  y_value_ = y;
  if (widget_view_ != nullptr)
    widget_view_->Redraw();
}

void Widget::SetY(const float y) {
  SetY(Alignment::kTop, Unit::kPoint, y);
}

bool Widget::ShouldHandleEvent(const Point location) {
  return false;
}

void Widget::StartAnimation() {
  if (widget_view_ == nullptr)
    return;

  if (animation_count_ == 0)
    widget_view_->StartAnimation();
  ++animation_count_;
}

void Widget::StopAnimation() {
  if (widget_view_ == nullptr)
    return;

  if (--animation_count_ == 0)
    widget_view_->StopAnimation();
  animation_count_ = std::max(0, animation_count_);
}

void Widget::UnbindRenderFunction() {
  render_function_ = NULL;
}

void Widget::set_alpha(const float alpha) {
  float revised_alpha = alpha;
  if (alpha > 1)
    revised_alpha = 1;
  else if (alpha < 0)
    revised_alpha = 0;

  if (revised_alpha == alpha_)
    return;

  alpha_ = revised_alpha;
  Redraw();
}

void Widget::set_background_color(const NVGcolor background_color) {
  if (!moui::nvgCompareColor(background_color, background_color_)) {
    background_color_ = background_color;
    Redraw();
  }
}

void Widget::set_rendering_offset(const Point offset) {
  if (offset.x == rendering_offset_.x && offset.y == rendering_offset_.y)
    return;

  rendering_offset_ = offset;
  if (render_function_ != NULL)
    Redraw();
}

void Widget::set_scale(const float scale) {
  if (scale == scale_)
    return;

  scale_ = scale;
  ResetMeasuredScaleRecursively(this);
  Redraw();
}

void Widget::set_widget_view(WidgetView* widget_view) {
  if (widget_view_ == widget_view)
    return;

  if (widget_view_ != nullptr) {
    NVGcontext* context = widget_view_->context();
    if (context != nullptr)
      ContextWillChange(context);
  }
  widget_view_ = widget_view;

  // Updates the widget view of all its child widgets as well.
  for (Widget* child_widget : children_)
    child_widget->set_widget_view(widget_view);
}

}  // namespace moui
