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

// Returns the height of the widget or zero if the widget is nullptr.
int GetHeightOrZero(const moui::Widget* widget) {
  if (widget == nullptr)
    return 0;
  return widget->GetHeight();
}

// Returns the pixels of height.
float GetHeightPixels(const moui::Widget* parent, const moui::Widget::Unit unit,
                      const int value) {
  float pixels;
  switch (unit) {
    case moui::Widget::Unit::kPercent:
      pixels = value / 100.0 * GetHeightOrZero(parent);
      break;
    case moui::Widget::Unit::kPixel:
      pixels = value;
      break;
    default:
      assert(false);
  }
  return pixels;
}

// Returns the width of the widget or zero if the widget is nullptr.
int GetWidthOrZero(const moui::Widget* widget) {
  if (widget == nullptr)
    return 0;
  return widget->GetWidth();
}

// Returns the pixels of width.
float GetWidthPixels(const moui::Widget* parent, const moui::Widget::Unit unit,
                     const int value) {
  float pixels;
  switch (unit) {
    case moui::Widget::Unit::kPercent:
      pixels = value / 100.0 * GetWidthOrZero(parent);
      break;
    case moui::Widget::Unit::kPixel:
      pixels = value;
      break;
    default:
      assert(false);
  }
  return pixels;
}

}  // namespace

namespace moui {

Widget::Widget() : Widget(true) {
}

Widget::Widget(const bool caches_rendering)
    : caches_rendering_(caches_rendering), context_(nullptr),
      default_framebuffer_(nullptr), default_framebuffer_mutex_(nullptr),
      height_unit_(Unit::kPixel), height_value_(0), hidden_(false),
      parent_(nullptr), should_redraw_default_framebuffer_(true),
      widget_view_(nullptr), width_unit_(Unit::kPixel), width_value_(0),
      x_alignment_(Alignment::kLeft), x_unit_(Unit::kPixel), x_value_(0),
      y_alignment_(Alignment::kTop), y_unit_(Unit::kPixel), y_value_(0) {
  if (caches_rendering)
    default_framebuffer_mutex_ = new std::mutex;
}

Widget::~Widget() {
  UpdateContext(nullptr);
}

void Widget::AddChild(Widget* child) {
  children_.push_back(child);
  child->set_parent(this);
  UpdateWidgetViewRecursively(child);
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
  const int kWidgetX = GetX();
  if (point.x < (kWidgetX - padding))
    return false;
  const int kWidgetY = GetY();
  if (point.y < (kWidgetY - padding))
    return false;
  const int kWidgetWidth = GetWidth();
  if (point.x >= (kWidgetX + kWidgetWidth + padding))
    return false;
  const int kWidgetHeight = GetHeight();
  if (point.y >= (kWidgetY + kWidgetHeight + padding))
    return false;
  return true;
}

void Widget::EndRenderbufferUpdates() {
  nvgluBindFramebuffer(NULL);
}

int Widget::GetHeight() const {
  return GetHeightPixels(parent_, height_unit_, height_value_) + 0.5;
}

int Widget::GetWidth() const {
  return GetWidthPixels(parent_, width_unit_, width_value_) + 0.5;
}

int Widget::GetX() const {
  float x;
  float offset = GetWidthPixels(parent_, x_unit_, x_value_);
  switch (x_alignment_) {
    case Alignment::kLeft:
      x = offset;
      break;
    case Alignment::kCenter:
      x = (parent_->GetWidth() - GetWidth()) / 2 + offset;
      break;
    case Alignment::kRight:
      x = parent_->GetWidth() - GetWidth() - offset;
      break;
    default:
      assert(false);
  }
  return x + 0.5;
}

int Widget::GetY() const {
  float y;
  float offset = GetHeightPixels(parent_, y_unit_, y_value_);
  switch (y_alignment_) {
    case Alignment::kTop:
      y = offset;
      break;
    case Alignment::kMiddle:
      y = (parent_->GetHeight() - GetHeight()) / 2 + offset;
      break;
    case Alignment::kBottom:
      y = parent_->GetHeight() - GetHeight() - offset;
      break;
    default:
      assert(false);
  }
  return y + 0.5;
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

void Widget::RenderDefaultFramebuffer(NVGcontext* context) {
  if (!caches_rendering_)
    return;

  default_framebuffer_mutex_->lock();
  if (!should_redraw_default_framebuffer_) {
    default_framebuffer_mutex_->unlock();
    return;
  }
  should_redraw_default_framebuffer_ = false;

  const int kWidth = GetWidth();
  const int kHeight = GetHeight();

  BeginRenderbufferUpdates(context, &default_framebuffer_);
  nvgBeginFrame(context, kWidth, kHeight, Device::GetScreenScaleFactor());
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
    Render(context);
  }
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
  return CollidePoint(location, 0);
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
}

void Widget::UpdateWidgetViewRecursively(Widget* widget) {
  widget->set_widget_view(widget_view_);
  for (Widget* child_widget : widget->children()) {
    UpdateWidgetViewRecursively(child_widget);
  }
}

}  // namespace moui
