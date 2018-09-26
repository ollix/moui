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
#include <cmath>
#include <cstdlib>
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

}  // namespace

namespace moui {

Widget::Widget(const bool caches_rendering)
    : alpha_(1), animation_count_(0),
      background_color_(nvgRGBA(255, 255, 255, 255)), bottom_padding_(0),
      box_sizing_(BoxSizing::kContentBox), caches_rendering_(caches_rendering),
      default_framebuffer_(nullptr), frees_children_on_destruction_(false),
      height_unit_(Unit::kPoint), height_value_(0), hidden_(false),
      is_opaque_(true), is_visible_(false), left_padding_(0),
      measured_scale_(-1), parent_(nullptr), paused_animation_(false),
      real_parent_(nullptr), render_function_(NULL), rendering_offset_({0, 0}),
      rendering_scale_(1), right_padding_(0), scale_(1),
      should_redraw_default_framebuffer_(false), tag_(0), top_padding_(0),
      widget_view_(nullptr), width_unit_(Unit::kPoint), width_value_(0),
      x_alignment_(Alignment::kLeft), x_unit_(Unit::kPoint), x_value_(0),
      y_alignment_(Alignment::kTop), y_unit_(Unit::kPoint), y_value_(0) {
}

Widget::Widget() : Widget(false) {
}

Widget::~Widget() {
  StopAnimation(true);
  RemoveFromParent();
  set_widget_view(nullptr);
  if (frees_children_on_destruction_) {
    for (Widget* child : children_) {
      delete child;
    }
  }
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
                                     NVGframebuffer** framebuffer,
                                     const float width, const float height,
                                     float* scale_factor) {
  const float kScaleFactor = \
      Device::GetScreenScaleFactor() * GetMeasuredScale();
  const int kWidth = static_cast<int>(width * kScaleFactor);
  const int kHeight = static_cast<int>(height * kScaleFactor);
  if (kWidth < 0 || kHeight < 0)
    return false;

  // Deletes the `framebuffer` if its size is not matched to the expected size.
  if (*framebuffer != nullptr) {
    int framebuffer_width = 0;
    int framebuffer_height = 0;
    nvgImageSize((*framebuffer)->ctx, (*framebuffer)->image,
                 &framebuffer_width, &framebuffer_height);
    if (kWidth != framebuffer_width || kHeight != framebuffer_height) {
      nvgDeleteFramebuffer(*framebuffer);
      *framebuffer = nullptr;
    }
  }

  if (*framebuffer == nullptr)
    *framebuffer = nvgCreateFramebuffer(context, kWidth, kHeight, 0);
  if (*framebuffer == NULL) {
    *framebuffer = nullptr;
    return false;
  }
  if (scale_factor != nullptr)
    *scale_factor = kScaleFactor;
  nvgBindFramebuffer(*framebuffer);
  nvgClearColor(context, kWidth, kHeight,
                is_opaque_ ? background_color_ : nvgRGBAf(0, 0, 0, 0));
  return true;
}

bool Widget::BeginFramebufferUpdates(NVGcontext* context,
                                    NVGframebuffer** framebuffer,
                                    float* scale_factor) {
  return BeginFramebufferUpdates(context, framebuffer, GetWidth(), GetHeight(),
                                 scale_factor);
}

bool Widget::BringChildToFront(Widget* child) {
  if (!children_.empty()) {
    auto iterator = std::find(children_.begin(), children_.end(), child);
    if (iterator == children_.end())
      return false;

    if (iterator == (children_.end() - 1))
      return true;

    children_.erase(iterator);
  }

  children_.push_back(child);
  if (widget_view_ != nullptr && !child->IsHidden())
    widget_view_->Redraw();
  return true;
}

void Widget::NotifyContextChange(NVGcontext* old_context,
                                 NVGcontext* new_context) {
  if (old_context == new_context)
    return;

  if (old_context != nullptr)
    ContextWillChange(old_context);
  ContextDidChange(new_context);
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
  nvgDeleteFramebuffer(default_framebuffer_);
  default_framebuffer_ = nullptr;
}

void Widget::EndFramebufferUpdates() {
  nvgBindFramebuffer(NULL);
}

void Widget::ExecuteRenderFunction(NVGcontext* context) {
  // Fills the background color.
  if (!caches_rendering_ && is_opaque_ && background_color_.a > 0) {
    nvgBeginPath(context);
    nvgRect(context, 0, 0, GetWidth(), GetHeight());
    nvgFillColor(context, background_color_);
    nvgFill(context);
  }

  nvgTranslate(context, rendering_offset_.x, rendering_offset_.y);
  if (render_function_ == NULL) {
    Render(context);
  } else {
    nvgScale(context, rendering_scale_, rendering_scale_);
    render_function_(this, context);
  }
}

float Widget::GetHeight() const {
  float parent_height = parent_ == nullptr ? 0 : parent_->GetHeight();
  if (parent_ != nullptr && box_sizing_ == BoxSizing::kBorderBox) {
    parent_height -= (parent_->top_padding() + parent_->bottom_padding());
  }
  return CalculatePoints(height_unit_, height_value_, parent_height);
}

float Widget::GetMeasuredAlpha() {
  float measure_alpha = alpha_;
  Widget* parent = parent_;
  while (parent != nullptr) {
    measure_alpha *= parent->alpha();
    parent = parent->parent();
  }
  return measure_alpha;
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

void Widget::GetOccupiedSpace(Size* size) const {
  size->width = GetWidth();
  size->height = GetHeight();

  const float kParentWidth = parent_ == nullptr ? 0 : parent_->GetWidth();
  const float kOffsetX = CalculatePoints(x_unit_, x_value_, kParentWidth);
  if (x_alignment_ == Alignment::kLeft || x_alignment_ == Alignment::kRight)
    size->width += kOffsetX;
  else if (x_alignment_ == Alignment::kCenter)
    size->width += std::abs(kOffsetX);

  const float kParentHeight = parent_ == nullptr ? 0 : parent_->GetHeight();
  const float kOffsetY = CalculatePoints(y_unit_, y_value_, kParentHeight);
  if (y_alignment_ == Alignment::kTop || y_alignment_ == Alignment::kBottom)
    size->height += kOffsetY;
  else if (x_alignment_ == Alignment::kMiddle)
    size->height += std::abs(kOffsetY);

  size->width *= scale_;
  size->height *= scale_;
}

float Widget::GetScaledHeight() const {
  return GetHeight() * scale_;
}

float Widget::GetScaledWidth() const {
  return GetWidth() * scale_;
}

// Asks the corresponded `widget_view_` to render the current widget in a
// newly created framebuffer. Once done, read the pixels out from that
// framebuffer and we can delete the framebuffer safely at this point.
unsigned char* Widget::GetSnapshot() {
  if (widget_view_ == nullptr)
    return nullptr;

  NVGcontext* context = widget_view_->context();
  if (context == nullptr)
    return nullptr;

  NVGframebuffer* framebuffer = nullptr;
  if (!BeginFramebufferUpdates(context, &framebuffer, nullptr))
    return nullptr;
  widget_view_->Render(this, framebuffer);

  int framebuffer_width = 0;
  int framebuffer_height = 0;
  nvgImageSize(framebuffer->ctx, framebuffer->image, &framebuffer_width,
               &framebuffer_height);

  unsigned char* snapshot = reinterpret_cast<unsigned char*>(
      std::malloc(framebuffer_width * framebuffer_height * 4));
  if (snapshot != nullptr) {
    nvgReadPixels(context, framebuffer->image, 0, 0, framebuffer_width,
                  framebuffer_height, snapshot);
  }
  EndFramebufferUpdates();
  nvgDeleteFramebuffer(framebuffer);
  return snapshot;
}

float Widget::GetWidth() const {
  float parent_width = parent_ == nullptr ? 0 : parent_->GetWidth();
  if (parent_ != nullptr && box_sizing_ == BoxSizing::kBorderBox) {
    parent_width -= (parent_->left_padding() + parent_->right_padding());
  }
  return CalculatePoints(width_unit_, width_value_, parent_width);
}

float Widget::GetX() const {
  const float kParentWidth = real_parent_ == nullptr ?
                             0 : real_parent_->GetWidth();
  const float kOffset = CalculatePoints(x_unit_, x_value_, kParentWidth);
  float parent_left_padding = 0;
  float parent_right_padding = 0;
  if (real_parent_ != nullptr &&
      real_parent_->box_sizing() == BoxSizing::kContentBox) {
    parent_left_padding = real_parent_ == nullptr ?
                          0 : real_parent_->left_padding();
    parent_right_padding = real_parent_ == nullptr ?
                           0 : real_parent_->right_padding();
  }
  float x;
  switch (x_alignment_) {
    case Alignment::kLeft:
      x = parent_left_padding + kOffset;
      break;
    case Alignment::kCenter:
      x = parent_left_padding + (kParentWidth - GetWidth() * scale_) / 2
          + kOffset;
      break;
    case Alignment::kRight:
      x = kParentWidth - parent_right_padding - GetWidth() * scale_ - kOffset;
      break;
    default:
      x = 0;
  }
  return x;
}

float Widget::GetY() const {
  const float kParentHeight = real_parent_ == nullptr ?
                              0 : real_parent_->GetHeight();
  const float kOffset = CalculatePoints(y_unit_, y_value_, kParentHeight);
  float parent_top_padding = 0;
  if (real_parent_ != nullptr &&
      real_parent_->box_sizing() == BoxSizing::kContentBox) {
    parent_top_padding = real_parent_ == nullptr ?
                         0 : real_parent_->top_padding();
  }
  float y;
  switch (y_alignment_) {
    case Alignment::kTop:
      y = parent_top_padding + kOffset;
      break;
    case Alignment::kMiddle:
      y = parent_top_padding + (kParentHeight - GetHeight() * scale_) / 2
          + kOffset;
      break;
    case Alignment::kBottom:
      y = kParentHeight - parent_top_padding - GetHeight() * scale_ - kOffset;
      break;
    default:
      y = 0;
  }
  return y;
}

void Widget::HandleMemoryWarning(NVGcontext* context) {
  nvgDeleteFramebuffer(default_framebuffer_);
  default_framebuffer_ = nullptr;
}

bool Widget::InsertChildAboveSibling(Widget* child, Widget* sibling) {
  if (child->real_parent_ == this)
    return false;

  auto iterator = std::find(children_.begin(), children_.end(), sibling);
  if (iterator == children_.end())
    return false;

  child->parent_ = this;
  child->real_parent_ = this;
  child->set_widget_view(widget_view_);
  children_.insert(iterator + 1, child);
  if (widget_view_ != nullptr && !child->IsHidden())
    widget_view_->Redraw();
  return true;
}

bool Widget::InsertChildBelowSibling(Widget* child, Widget* sibling) {
  if (child->real_parent_ == this)
    return false;

  auto iterator = std::find(children_.begin(), children_.end(), sibling);
  if (iterator == children_.end())
    return false;

  child->parent_ = this;
  child->real_parent_ = this;
  child->set_widget_view(widget_view_);
  children_.insert(iterator, child);
  if (widget_view_ != nullptr && !child->IsHidden())
    widget_view_->Redraw();
  return true;
}

bool Widget::IsAnimating() const {
  return animation_count_ > 0;
}

bool Widget::IsChild(Widget* parent) {
  if (parent == nullptr || real_parent_ == nullptr)
    return false;

  for (Widget* candidate : *parent->children()) {
    if (this == candidate)
      return true;
  }
  return false;
}

bool Widget::IsHidden() const {
  return hidden_;
}

void Widget::Redraw() {
  if (caches_rendering_) {
    should_redraw_default_framebuffer_ = true;
  }
  if (widget_view_ != nullptr)
    widget_view_->Redraw(this);
}

bool Widget::RemoveChild(Widget* child) {
  auto iterator = std::find(children_.begin(), children_.end(), child);
  if (iterator == children_.end())
    return false;
  children_.erase(iterator);
  Redraw();
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

  // Resets the default framebuffer if the widget's size has been changed.
  const float kWidth = GetWidth();
  const float kHeight = GetHeight();
  if (default_framebuffer_ != nullptr) {
    const float kScaleFactor = \
        Device::GetScreenScaleFactor() * GetMeasuredScale();
    const int kFramebufferWidth = static_cast<int>(kWidth * kScaleFactor);
    const int kFramebufferHeight = static_cast<int>(kHeight * kScaleFactor);
    int framebuffer_width = 0;
    int framebuffer_height = 0;
    nvgImageSize(default_framebuffer_->ctx, default_framebuffer_->image,
                 &framebuffer_width, &framebuffer_height);
    if (kFramebufferWidth != framebuffer_width ||
        kFramebufferHeight != framebuffer_height) {
      nvgDeleteFramebuffer(default_framebuffer_);
      default_framebuffer_ = nullptr;
      should_redraw_default_framebuffer_ = true;
    }
  }

  if (default_framebuffer_ != nullptr && !should_redraw_default_framebuffer_ &&
      !IsAnimating()) {
    return;
  }
  should_redraw_default_framebuffer_ = false;

  float scale_factor;
  if (BeginFramebufferUpdates(context, &default_framebuffer_, &scale_factor)) {
    nvgBeginFrame(context, kWidth, kHeight, scale_factor);
    ExecuteRenderFunction(context);
    nvgEndFrame(context);
    EndFramebufferUpdates();
    default_framebuffer_paint_ = nvgImagePattern(context, 0, 0, kWidth,
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
  for (Widget* child : *widget->children())
    ResetMeasuredScaleRecursively(child);
}

bool Widget::SendChildToBack(Widget* child) {
  if (!children_.empty()) {
    auto iterator = std::find(children_.begin(), children_.end(), child);
    if (iterator == children_.end()) {
      return false;
    }
    if (iterator == children_.begin()) {
      return true;
    }
    children_.erase(iterator);
  }
  children_.insert(children_.begin(), child);
  if (widget_view_ != nullptr && !child->IsHidden())
    widget_view_->Redraw();
  return true;
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

void Widget::SetPadding(const float vertical_padding,
                        const float horizontal_padding) {
  set_left_padding(horizontal_padding);
  set_right_padding(horizontal_padding);
  set_top_padding(vertical_padding);
  set_bottom_padding(vertical_padding);
}

void Widget::SetPadding(const float padding) {
  SetPadding(padding, padding);
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
    widget_view_->Redraw(this);
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
    widget_view_->Redraw(this);
}

void Widget::SetY(const float y) {
  SetY(Alignment::kTop, Unit::kPoint, y);
}

bool Widget::ShouldHandleEvent(const Point location) {
  return false;
}

void Widget::StartAnimation() {
  if (animation_count_++ == 0 && !paused_animation_) {
    if (is_visible_ && widget_view_ != nullptr)
      widget_view_->StartAnimation();
    else
      paused_animation_ = true;
  }
}

void Widget::StopAnimation(const bool force) {
  const bool kIsAnimating = IsAnimating();
  animation_count_ = force ? 0 : std::max(0, animation_count_ - 1);
  if (kIsAnimating && animation_count_ == 0) {
    if (paused_animation_)
      paused_animation_ = false;
    else
      widget_view_->StopAnimation();
  }
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
  if (widget_view_ != nullptr)
    widget_view_->Redraw(this);
}

void Widget::set_background_color(const NVGcolor background_color) {
  if (!moui::nvgCompareColor(background_color, background_color_)) {
    background_color_ = background_color;
    Redraw();
  }
}

void Widget::set_bottom_padding(const float padding) {
  if (padding != bottom_padding_) {
    bottom_padding_ = padding;
    if (widget_view_ != nullptr)
      widget_view_->Redraw(this);
  }
}

void Widget::set_box_sizing(const BoxSizing box_sizing) {
  if (box_sizing != box_sizing_) {
    box_sizing_ = box_sizing;
    if (widget_view_ != nullptr)
      widget_view_->Redraw(this);
  }
}

void Widget::set_left_padding(const float padding) {
  if (padding != left_padding_) {
    left_padding_ = padding;
    if (widget_view_ != nullptr)
      widget_view_->Redraw(this);
  }
}

// Updates the animation state of the corresponded widget view.
void Widget::set_is_visible(const bool is_visible) {
  if (is_visible == is_visible_)
    return;

  if (is_visible && paused_animation_ && widget_view_ != nullptr) {
    paused_animation_ = false;
    widget_view_->StartAnimation();
  } else if (!is_visible && !paused_animation_ && IsAnimating()) {
    paused_animation_ = true;
    widget_view_->StopAnimation();
  }
  is_visible_ = is_visible;
}

void Widget::set_rendering_offset(const Point offset) {
  if (offset.x == rendering_offset_.x && offset.y == rendering_offset_.y)
    return;

  rendering_offset_ = offset;
  Redraw();
}

void Widget::set_rendering_scale(const float rendering_scale) {
  if (rendering_scale == rendering_scale_)
    return;

  rendering_scale_ = rendering_scale;
  Redraw();
}

void Widget::set_right_padding(const float padding) {
  if (padding != right_padding_) {
    right_padding_ = padding;
    if (widget_view_ != nullptr)
      widget_view_->Redraw(this);
  }
}

void Widget::set_scale(const float scale) {
  if (scale == scale_)
    return;

  scale_ = scale;
  ResetMeasuredScaleRecursively(this);
  Redraw();
}

void Widget::set_top_padding(const float padding) {
  if (padding != top_padding_) {
    top_padding_ = padding;
    if (widget_view_ != nullptr)
      widget_view_->Redraw(this);
  }
}

void Widget::set_widget_view(WidgetView* widget_view) {
  if (widget_view_ == widget_view)
    return;

  NVGcontext* old_context = nullptr;
  if (widget_view_ != nullptr) {
    widget_view_->RemoveResponder(this);
    old_context = widget_view_->context();
  }
  set_is_visible(false);
  widget_view_ = widget_view;
  NVGcontext* new_context = (widget_view == nullptr) ? nullptr :
                                                       widget_view->context();
  NotifyContextChange(old_context, new_context);

  // Updates the widget view of all its child widgets as well.
  for (Widget* child_widget : children_)
    child_widget->set_widget_view(widget_view);
}

}  // namespace moui
