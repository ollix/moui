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

#include "moui/widgets/scroller.h"

#include <algorithm>
#include <cassert>

#include "moui/core/clock.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace {

// The duration measured in seconds for hiding the scroller in animation.
const double kAnimatingHideDuration = 0.2;

// The default width of the scroller's knob.
const float kKnobWidth = 5;

// The default padding on both sides of the scroller's track .
const float kTrackPadding = 3;

}  // namespace

namespace moui {

Scroller::Scroller(const Direction direction)
    : Widget(false), animation_progress_(-1), direction_(direction),
      shows_scrollers_on_both_directions_(false) {
  set_is_opaque(false);
  SetX(Widget::Alignment::kRight, Widget::Unit::kPoint, 0);
  SetY(Widget::Alignment::kBottom, Widget::Unit::kPoint, 0);
  if (direction == Direction::kHorizontal) {
    SetWidth(Widget::Unit::kPercent, 100);
    SetHeight(Widget::Unit::kPoint, kKnobWidth + kTrackPadding * 2);
  } else if (direction == Direction::kVertical) {
    SetWidth(Widget::Unit::kPoint, kKnobWidth + kTrackPadding * 2);
    SetHeight(Widget::Unit::kPercent, 100);
  }
}

Scroller::~Scroller() {
}

void Scroller::HideInAnimation() {
  if (hiding_in_animation_ || IsHidden())
    return;

  hiding_in_animation_ = true;
  animation_initial_timestamp_ = Clock::GetTimestamp();
  StartAnimation();
}

void Scroller::Render(NVGcontext* context) {
  if (hiding_in_animation_) {
    nvgGlobalAlpha(context, 1 - animation_progress_);
  }

  nvgLineCap(context, NVG_ROUND);
  nvgStrokeColor(context, nvgRGBA(0, 0, 0, 132));
  nvgStrokeWidth(context, kKnobWidth);

  const int kWidgetLength = direction_ == Direction::kHorizontal ?
                            GetWidth() : GetHeight();
  const float kTrackLength = \
      kWidgetLength - kTrackPadding * 2 - \
      (shows_scrollers_on_both_directions_ ? kKnobWidth : 0);
  const float kKnobLength = std::max(
      kKnobWidth, static_cast<float>(kTrackLength * knob_proportion_));
  const float kKnobPosition = kTrackPadding + kTrackLength * knob_position_;
  RenderKnob(context, kKnobPosition, kKnobLength);
}

// Stops animation immediately before redrawing.
void Scroller::Redraw() {
  if (hiding_in_animation_) {
    StopAnimation(true);
    hiding_in_animation_ = false;
  }
  Widget::Redraw();
}

void Scroller::RenderKnob(NVGcontext* context, const float position,
                          const float length) const {
  const float kKnobDotRadius = kKnobWidth / 2;
  const float kKnobOffset = position + kKnobDotRadius;
  const int kWidgetWidth = direction_ == Direction::kHorizontal ?
                           GetHeight() : GetWidth();
  const float kKnobSideOffset = kWidgetWidth - kTrackPadding - kKnobDotRadius;
  // The minimum knob length of 0.1 makes the knob looks like a circle.
  const float kKnobLength = std::max(0.1f, length - kKnobWidth);

  nvgBeginPath(context);
  if (direction_ == Direction::kHorizontal) {
    nvgMoveTo(context, kKnobOffset, kKnobSideOffset);
    nvgLineTo(context, kKnobOffset + kKnobLength, kKnobSideOffset);
  } else if (direction_ == Direction::kVertical) {
    nvgMoveTo(context, kKnobSideOffset, kKnobOffset);
    nvgLineTo(context, kKnobSideOffset, kKnobOffset + kKnobLength);
  } else {
    assert(false);
  }
  nvgStroke(context);
}

void Scroller::SetHidden(const bool hidden) {
  Widget::SetHidden(hidden);
  if (!hidden && hiding_in_animation_)
    Redraw();
}

// Stops animation if reaching the animation duration.
void Scroller::WidgetViewDidRender(NVGcontext* context) {
  if (animation_progress_ < 1)
    return;
  StopAnimation(true);

  if (hiding_in_animation_) {
    SetHidden(true);
    hiding_in_animation_ = false;
  }
  animation_progress_ = -1;
}

// Calculates the animation progress.
bool Scroller::WidgetViewWillRender(NVGcontext* context) {
  if (!IsAnimating())
    return true;

  const double kElapsedTime = \
      Clock::GetTimestamp() - animation_initial_timestamp_;

  if (hiding_in_animation_)
    animation_progress_ = kElapsedTime / kAnimatingHideDuration;

  animation_progress_ = std::min(1.0f, animation_progress_);
  return true;
}

void Scroller::set_knob_position(const double value) {
  double knob_position = value;
  if (value < 0)
    knob_position = 0;
  else if (value > 1)
    knob_position = 1;

  if (knob_position != knob_position_) {
    knob_position_ = knob_position;
    Redraw();
  }
}

void Scroller::set_knob_proportion(const double value) {
  double knob_proportion = value;
  if (value < 0)
    knob_proportion = 0;
  else if (value > 1)
    knob_proportion = 1;

  if (knob_proportion != knob_proportion_) {
    knob_proportion_ = knob_proportion;
    Redraw();
  }
}

void Scroller::set_shows_scrollers_on_both_directions(const bool value) {
  if (value != shows_scrollers_on_both_directions_) {
    shows_scrollers_on_both_directions_ = value;
    Redraw();
  }
}

}  // namespace moui
