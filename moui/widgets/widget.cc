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

// Returns the integral value that is nearest to x.
int RoundToInteger(float value) {
  return value > 0 ? value + 0.5 : value - 0.5;
}

}  // namespace

namespace moui {

Widget::Widget() : Widget(true) {
}

Widget::Widget(const bool caches_rendering)
    : animation_count_(0), caches_rendering_(caches_rendering),
      context_(nullptr), default_framebuffer_(nullptr),
      default_framebuffer_mutex_(nullptr), height_unit_(Unit::kPoint),
      height_value_(0), hidden_(false), is_opaque_(true), parent_(nullptr),
      should_redraw_default_framebuffer_(true), widget_view_(nullptr),
      width_unit_(Unit::kPoint), width_value_(0),
      x_alignment_(Alignment::kLeft), x_unit_(Unit::kPoint), x_value_(0),
      y_alignment_(Alignment::kTop), y_unit_(Unit::kPoint), y_value_(0) {
  if (caches_rendering)
    default_framebuffer_mutex_ = new std::mutex;

  // Sets white as default background color.
  set_background_color(255, 255, 255, 255);
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
  if (!child->IsHidden())
    Redraw();
}

void Widget::BeginRenderbufferUpdates(NVGcontext* context,
                                      NVGLUframebuffer** framebuffer) {
  const int kScreenScaleFactor = Device::GetScreenScaleFactor();
  const int kWidth = GetWidth() * kScreenScaleFactor;
  const int kHeight = GetHeight() * kScreenScaleFactor;

  if (*framebuffer == nullptr)
    *framebuffer = nvgluCreateFramebuffer(context, kWidth, kHeight, 0);
  nvgluBindFramebuffer(*framebuffer);
  glViewport(0, 0, kWidth, kHeight);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

bool Widget::CollidePoint(const Point point, const int padding) const {
  // Determines the widget's origin related to the root widget.
  int origin_x = GetX();
  int origin_y = GetY();
  Widget* parent = this->parent();
  while (parent != nullptr) {
    origin_x += parent->GetX();
    origin_y += parent->GetY();
    parent = parent->parent();
  }
  return (point.x >= (origin_x - padding) &&
          point.y >= (origin_y - padding) &&
          point.x <= (origin_x + GetWidth() + padding) &&
          point.y <= (origin_y + GetHeight() + padding));
}

void Widget::EndRenderbufferUpdates() {
  nvgluBindFramebuffer(NULL);
}

int Widget::GetHeight() const {
  const int kParentHeight = parent_ == nullptr ? 0 : parent_->GetHeight();
  const float kHeight = CalculatePoints(height_unit_, height_value_,
                                        kParentHeight);
  return RoundToInteger(kHeight);
}

int Widget::GetWidth() const {
  const int kParentWidth = parent_ == nullptr ? 0 : parent_->GetWidth();
  const float kWidth = CalculatePoints(width_unit_, width_value_, kParentWidth);
  return RoundToInteger(kWidth);
}

int Widget::GetX() const {
  const int kParentWidth = parent_ == nullptr ? 0 : parent_->GetWidth();
  const float kOffset = CalculatePoints(x_unit_, x_value_, kParentWidth);
  float x;
  switch (x_alignment_) {
    case Alignment::kLeft:
      x = kOffset;
      break;
    case Alignment::kCenter:
      x = (kParentWidth - GetWidth()) / 2 + kOffset;
      break;
    case Alignment::kRight:
      x = kParentWidth - GetWidth() - kOffset;
      break;
    default:
      assert(false);
  }
  return RoundToInteger(x);
}

int Widget::GetY() const {
  const int kParentHeight = parent_ == nullptr ? 0 : parent_->GetHeight();
  const float kOffset = CalculatePoints(y_unit_, y_value_, kParentHeight);
  float y;
  switch (y_alignment_) {
    case Alignment::kTop:
      y = kOffset;
      break;
    case Alignment::kMiddle:
      y = (kParentHeight - GetHeight()) / 2 + kOffset;
      break;
    case Alignment::kBottom:
      y = kParentHeight - GetHeight() - kOffset;
      break;
    default:
      assert(false);
  }
  return RoundToInteger(y);
}

bool Widget::IsAnimating() const {
  return animation_count_ > 0;
}

bool Widget::IsHidden() const {
  return hidden_;
}

void Widget::Redraw() {
  if (widget_view_ == nullptr)
    return;

  if (caches_rendering_) {
    default_framebuffer_mutex_->lock();
    should_redraw_default_framebuffer_ = true;
    default_framebuffer_mutex_->unlock();
  }
  widget_view_->Redraw();
}

void Widget::RenderBackgroundColor(NVGcontext* context) {
  // Do nothing if the widget is not opaque or the color's alpha value is 0.
  if (!is_opaque_ || background_color_.rgba[3] == 0)
    return;

  nvgBeginPath(context);
  nvgRect(context, 0, 0, GetWidth(), GetHeight());
  nvgFillColor(context, background_color_);
  nvgFill(context);
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

  const int kWidth = GetWidth();
  const int kHeight = GetHeight();

  BeginRenderbufferUpdates(context, &default_framebuffer_);
  nvgBeginFrame(context, kWidth, kHeight, Device::GetScreenScaleFactor());
  RenderBackgroundColor(context);
  Render(context);
  nvgEndFrame(context);
  EndRenderbufferUpdates();

  default_framebuffer_paint_ = nvgImagePattern(
      context, 0, kHeight, kWidth, kHeight, 0, default_framebuffer_->image, 1);
  default_framebuffer_mutex_->unlock();
}

void Widget::RenderOnDemand(NVGcontext* context) {
  if (caches_rendering_) {
    nvgBeginPath(context);
    nvgRect(context, 0, 0, GetWidth(), GetHeight());
    nvgFillPaint(context, default_framebuffer_paint_);
    nvgFill(context);
  } else {
    RenderBackgroundColor(context);
    Render(context);
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
  height_unit_ = unit;
  height_value_ = height;
}

void Widget::SetHidden(bool hidden) {
  if (hidden != hidden_) {
    hidden_ = hidden;
    Redraw();
  }
}

void Widget::SetWidth(const Unit unit, const float width) {
  width_unit_ = unit;
  width_value_ = width;
}

void Widget::SetX(const Alignment alignment, const Unit unit, const float x) {
  x_alignment_ = alignment;
  x_unit_ = unit;
  x_value_ = x;
}

void Widget::SetY(const Alignment alignment, const Unit unit, const float y) {
  y_alignment_ = alignment;
  y_unit_ = unit;
  y_value_ = y;
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

}  // namespace moui
