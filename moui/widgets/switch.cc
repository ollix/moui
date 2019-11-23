// Copyright (c) 2016 Ollix. All rights reserved.
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

#include "moui/widgets/switch.h"

#include <algorithm>

#include "moui/core/clock.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/control.h"

namespace {

const NVGcolor kDefaultTintColor = nvgRGB(76, 216, 100);
const float kSwitchWidth = 51;
const float kSwitchHeight = 31;
const float kOutlineWidth = 2;
const float kKnobRadius = (kSwitchHeight - kOutlineWidth * 2) / 2;
const float kTrackWidth = kSwitchWidth - (kOutlineWidth + kKnobRadius) * 2;

const float kAnimationDuration = 0.15;

}  // namespace

namespace moui {

Switch::Switch() : Control(true), is_on_(false),
                   tint_color_(kDefaultTintColor) {
  set_is_opaque(false);
  BindAction(ControlEvents::kTouchUpInside, &Switch::ToggleState, this);
}

Switch::~Switch() {
}

bool Switch::IsOn() const {
  return is_on_;
}

void Switch::Render(NVGcontext* context) {
  float knob_position = is_on_ ? 1 : 0;
  if (IsAnimating()) {
    const float kElapsedTime = \
        Clock::GetTimestamp() - animation_start_timestamp_;
    const float kAnimationProgress = \
        std::min(1.0f, kElapsedTime / kAnimationDuration);
    knob_position = is_on_ ? kAnimationProgress : 1 - kAnimationProgress;
    if (kAnimationProgress == 1)
      StopAnimation(true);
  }

  // Draws the outline.
  nvgBeginPath(context);
  nvgMoveTo(context, 35.5, 0);
  nvgLineTo(context, 15.5, 0);
  nvgBezierTo(context, 6.939, 0, 0, 6.939, 0, 15.5);
  nvgBezierTo(context, 0, 24.061, 6.939, 31, 15.5, 31);
  nvgLineTo(context, 35.5, 31);
  nvgBezierTo(context, 44.061, 31, 51, 24.061, 51, 15.5);
  nvgBezierTo(context, 51, 6.939, 44.061, 0, 35.5, 0);
  nvgClosePath(context);
  nvgFillColor(context, nvgRGBA(175, 175, 175, 40));
  nvgFill(context);

  // Draws the background.
  nvgBeginPath(context);
  nvgMoveTo(context, 35.5, 0);
  nvgLineTo(context, 15.5, 0);
  nvgBezierTo(context, 6.939, 0, 0, 6.939, 0, 15.5);
  nvgBezierTo(context, 0.0, 24.061, 6.939, 31, 15.5, 31);
  nvgLineTo(context, 35.5, 31);
  nvgBezierTo(context, 44.061, 31, 51, 24.061, 51, 15.5);
  nvgBezierTo(context, 51.0, 6.939, 44.061, 0, 35.5, 0);
  nvgFillColor(context, nvgTransRGBA(tint_color_, 255 * knob_position));
  nvgFill(context);

  // Draws the shadow of knob.
  const float kKnobCenterX = kOutlineWidth + kKnobRadius \
                             + kTrackWidth * knob_position;
  const float kKnobCenterY = kOutlineWidth + kKnobRadius;
  const float kShadowOuterRadius = kKnobRadius + kOutlineWidth * 1.5;
  const NVGpaint kShadow = nvgRadialGradient(context, kKnobCenterX,
                                             kKnobCenterY, kKnobRadius,
                                             kShadowOuterRadius,
                                             nvgRGBA(0, 0, 0, 10),
                                             nvgRGBA(0, 0, 0, 0));
  nvgBeginPath(context);
  nvgRect(context, kKnobCenterX - kShadowOuterRadius,
          kKnobCenterY - kShadowOuterRadius, kShadowOuterRadius * 2,
          kShadowOuterRadius * 2);
  nvgFillPaint(context, kShadow);
  nvgFill(context);

  // Draws the knob.
  nvgBeginPath(context);
  nvgCircle(context, kKnobCenterX, kKnobCenterY, kKnobRadius);
  nvgFillColor(context, nvgRGB(255, 255, 255));
  nvgFill(context);
}

void Switch::SetOn(const bool is_on, const bool animated) {
  if (is_on == is_on_)
    return;
  if (animated)
    ToggleState(nullptr);
  else
    is_on_ = is_on;
}

void Switch::ToggleState(Control* control) {
  if (!IsAnimating()) {
    is_on_ = !is_on_;
    animation_start_timestamp_ = Clock::GetTimestamp();
    StartAnimation();
  }
}

bool Switch::WidgetViewWillRender(NVGcontext* context) {
  SetWidth(kSwitchWidth);
  SetHeight(kSwitchHeight);
  return true;
}

void Switch::set_tint_color(const NVGcolor tint_color) {
  if (!nvgCompareColor(tint_color, tint_color_)) {
    tint_color_ = tint_color;
    Redraw();
  }
}

}  // namespace moui
