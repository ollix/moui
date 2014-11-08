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

#include <cassert>
#include <mutex>
#include <stack>
#include <vector>

#include "moui/core/device.h"
#include "moui/core/event.h"
#include "moui/widgets/widget_view.h"

namespace {

// Returns the actual length in points of the specified value and unit.
float CalculatePoints(const moui::Widget::Unit unit, const float value,
                      const int parent_length) {
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

}  // namespace

namespace moui {

Widget::Widget() : Widget(true) {
}

Widget::Widget(const bool caches_rendering)
    : animation_count_(0), background_color_(nvgRGBA(255, 255, 255, 255)),
      caches_rendering_(caches_rendering), context_(nullptr),
      default_framebuffer_(nullptr), default_framebuffer_mutex_(nullptr),
      height_unit_(Unit::kPoint), height_value_(0), hidden_(false),
      is_opaque_(true), parent_(nullptr), render_function_(NULL),
      rendering_offset_({0, 0}), scale_(1),
      should_redraw_default_framebuffer_(true), widget_view_(nullptr),
      width_unit_(Unit::kPoint), width_value_(0),
      x_alignment_(Alignment::kLeft), x_unit_(Unit::kPoint), x_value_(0),
      y_alignment_(Alignment::kTop), y_unit_(Unit::kPoint), y_value_(0) {
  if (caches_rendering)
    default_framebuffer_mutex_ = new std::mutex;
}

Widget::~Widget() {
  UpdateContext(nullptr);

  if (caches_rendering_)
    delete default_framebuffer_mutex_;

  // Resets children's parent.
  for (Widget* child : children())
    child->set_parent(nullptr);
}

void Widget::AddChild(Widget* child) {
  child->set_parent(this);
  UpdateChildrenRecursively(child);
  children_.push_back(child);
  if (widget_view_ != nullptr && !child->IsHidden())
    widget_view_->Redraw();
}

bool Widget::BeginRenderbufferUpdates(NVGcontext* context,
                                      NVGLUframebuffer** framebuffer) {
  const float kScreenScaleFactor = Device::GetScreenScaleFactor();
  const float kWidth = GetWidth() * kScreenScaleFactor;
  const float kHeight = GetHeight() * kScreenScaleFactor;
  if (kWidth <= 0 || kHeight <= 0)
    return false;
  if (*framebuffer == nullptr)
    *framebuffer = nvgluCreateFramebuffer(context, kWidth, kHeight, 0);
  if (*framebuffer == NULL) {
    *framebuffer = nullptr;
    return false;
  }
  nvgluBindFramebuffer(*framebuffer);
  glViewport(0, 0, kWidth, kHeight);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  return true;
}

bool Widget::CollidePoint(const Point point, const int padding) {
  return CollidePoint(point, padding, padding, padding, padding);
}

// This method compares the passed point to the widget's actual origin and
// dimenstion related to the corresponded widget view's coordinate system.
bool Widget::CollidePoint(const Point point, const int top_padding,
                          const int right_padding, const int bottom_padding,
                          const int left_padding) {
  Point origin;
  Size size;
  GetMeasuredBounds(&origin, &size);
  return (point.x >= (origin.x - left_padding) &&
          point.y >= (origin.y - top_padding) &&
          point.x < (origin.x + size.width + right_padding) &&
          point.y < (origin.y + size.height + bottom_padding));
}

void Widget::EndRenderbufferUpdates() {
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
  if (render_function_ == NULL)
    Render(context);
  else
    render_function_();
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
  Widget* parent = parent_;
  while (parent != nullptr) {
    widget_chain.push(parent);
    parent = parent->parent();
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

float Widget::GetWidth() const {
  const float kParentWidth = parent_ == nullptr ? 0 : parent_->GetWidth();
  return CalculatePoints(width_unit_, width_value_, kParentWidth);
}

float Widget::GetX() const {
  const float kParentWidth = parent_ == nullptr ? 0 : parent_->GetWidth();
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
  const float kParentHeight = parent_ == nullptr ? 0 : parent_->GetHeight();
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
    default_framebuffer_mutex_->lock();
    should_redraw_default_framebuffer_ = true;
    default_framebuffer_mutex_->unlock();
  }
  if (widget_view_ != nullptr && !IsHidden())
    widget_view_->Redraw();
}

void Widget::RenderDefaultFramebuffer(NVGcontext* context) {
  if (!caches_rendering_)
    return;

  default_framebuffer_mutex_->lock();
  if (!should_redraw_default_framebuffer_ && !IsAnimating()) {
    default_framebuffer_mutex_->unlock();
    return;
  }
  should_redraw_default_framebuffer_ = false;
  nvgluDeleteFramebuffer(context_, default_framebuffer_);
  default_framebuffer_ = nullptr;

  const float kWidth = GetWidth();
  const float kHeight = GetHeight();

  if (BeginRenderbufferUpdates(context, &default_framebuffer_)) {
    nvgBeginFrame(context, kWidth, kHeight, Device::GetScreenScaleFactor());
    ExecuteRenderFunction(context);
    nvgEndFrame(context);
    EndRenderbufferUpdates();
    default_framebuffer_paint_ = nvgImagePattern(context, 0, kHeight, kWidth,
                                                 kHeight, 0,
                                                 default_framebuffer_->image,
                                                 1);
  }
  default_framebuffer_mutex_->unlock();
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

void Widget::SetBounds(const int x, const int y, const int width,
                       const int height) {
  SetX(Alignment::kLeft, Unit::kPoint, x);
  SetY(Alignment::kTop, Unit::kPoint, y);
  SetWidth(Unit::kPoint, width);
  SetHeight(Unit::kPoint, height);
}

void Widget::SetHeight(const Unit unit, const float height) {
  if (unit == height_unit_ && height == height_value_)
    return;

  height_unit_ = unit;
  height_value_ = height;
  Redraw();
}

void Widget::SetHidden(const bool hidden) {
  if (hidden != hidden_) {
    hidden_ = hidden;
    Redraw();
  }
}

void Widget::SetWidth(const Unit unit, const float width) {
  if (unit == width_unit_ && width == width_value_)
    return;

  width_unit_ = unit;
  width_value_ = width;
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

void Widget::SetY(const Alignment alignment, const Unit unit, const float y) {
  if (alignment == y_alignment_ && unit == y_unit_ && y == y_value_)
    return;

  y_alignment_ = alignment;
  y_unit_ = unit;
  y_value_ = y;
  if (widget_view_ != nullptr)
    widget_view_->Redraw();
}

bool Widget::ShouldHandleEvent(const Point location) {
  return false;
}

void Widget::StartAnimation() {
  if (widget_view_ == nullptr)
    return;
  ++animation_count_;
  widget_view_->StartAnimation();
}

void Widget::StopAnimation() {
  if (widget_view_ == nullptr)
    return;
  animation_count_ = std::max(0, animation_count_ - 1);
  widget_view_->StopAnimation();
}

void Widget::UnbindRenderFunction() {
  render_function_ = NULL;
}

void Widget::UpdateChildrenRecursively(Widget* widget) {
  widget->set_widget_view(widget_view_);
  widget->UpdateContext(context_);
  for (Widget* child_widget : widget->children())
    UpdateChildrenRecursively(child_widget);
}

void Widget::UpdateContext(NVGcontext* context) {
  if (context == context_)
    return;

  // Resets the default framebuffer if caches_rendering_ is true.
  if (context_ != nullptr && default_framebuffer_ != nullptr) {
    default_framebuffer_mutex_->lock();
    nvgluDeleteFramebuffer(context_, default_framebuffer_);
    default_framebuffer_ = nullptr;
    default_framebuffer_mutex_->unlock();
  }

  ContextWillChange(context_);
  context_ = context;
  UpdateChildrenRecursively(this);
}

void Widget::set_background_color(const NVGcolor background_color) {
  if (!nvgCompareColor(background_color, background_color_)) {
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

}  // namespace moui
