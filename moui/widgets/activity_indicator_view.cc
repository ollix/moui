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

#include "moui/widgets/activity_indicator_view.h"

#include <algorithm>
#include <cmath>

#include "moui/core/clock.h"
#include "moui/nanovg_hook.h"

namespace {

const int kDefaultColorStartLineIndex = 9;
const int kDefaultNumberOfColoredLines = 6;
const int kDefaultNumberOfLines = 12;

}  // namespace

namespace moui {

ActivityIndicatorView::ActivityIndicatorView()
    : animation_start_timestamp_(-1),
      color_start_line_index_(kDefaultColorStartLineIndex),
      last_color_(nvgRGBAf(0, 0, 0, 0.2)),
      number_of_colored_lines_(kDefaultNumberOfColoredLines),
      number_of_lines_(kDefaultNumberOfLines),
      number_of_visible_lines_(number_of_lines_),
      start_color_(nvgRGBAf(0, 0, 0, 0.5)) {
  set_is_opaque(false);
}

ActivityIndicatorView::~ActivityIndicatorView() {
}

void ActivityIndicatorView::StartAnimating() {
  if (IsAnimating())
    return;
  animation_start_timestamp_ = Clock::GetTimestamp();
  animation_color_start_line_index_ = color_start_line_index_;
  StartAnimation();
}

void ActivityIndicatorView::StopAnimating() {
  animation_start_timestamp_ = -1;
  StopAnimation(true);
}

void ActivityIndicatorView::Render(NVGcontext* context) {
  const float kWidth = GetWidth();
  const float kHeight = GetHeight();
  const float kIndicatorRadius = std::min(kWidth, kHeight) * 0.5;
  const float kLineWidth = kIndicatorRadius / 6;
  const float kLineStartPoint = kIndicatorRadius * 0.5;
  const float kLineEndPoint = kIndicatorRadius - kLineWidth / 2 - 1;

  if (animation_start_timestamp_ > 0) {
    const double kElapsedTime = \
        Clock::GetTimestamp() - animation_start_timestamp_;
    color_start_line_index_ = \
        static_cast<int>(animation_color_start_line_index_ + kElapsedTime / 0.1)
        % number_of_lines_;
  }

  nvgLineCap(context, NVG_ROUND);
  nvgLineJoin(context, NVG_ROUND);
  nvgStrokeWidth(context, kLineWidth);
  nvgTranslate(context, kWidth / 2, kHeight / 2);

  for (int line_index = 0; line_index < number_of_lines_; ++line_index) {
    if (line_index >= number_of_visible_lines_)
      continue;

    const int kColorIndex =
        ((number_of_lines_ - line_index) + color_start_line_index_)
            % number_of_lines_;
    const NVGcolor kLineColor = nvgLerpRGBA(
        start_color_, last_color_,
        static_cast<float>(kColorIndex) / (number_of_colored_lines_ - 1));

    nvgSave(context);
    nvgBeginPath(context);
    nvgRotate(context, 2 * M_PI / number_of_lines_ * line_index - 0.5 * M_PI);
    nvgMoveTo(context, kLineStartPoint, 0);
    nvgLineTo(context, kLineEndPoint, 0);
    nvgStrokeColor(context, kLineColor);
    nvgStroke(context);
    nvgRestore(context);
  }
}

void ActivityIndicatorView::set_color_start_line_index(
    const int color_start_line_index) {
  if (color_start_line_index == color_start_line_index_)
    return;
  color_start_line_index_ = color_start_line_index;
  Redraw();
}

void ActivityIndicatorView::set_last_color(const NVGcolor last_color) {
  if (nvgCompareColor(last_color, last_color_))
    return;
  last_color_ = last_color;
  Redraw();
}

void ActivityIndicatorView::set_number_of_lines(const int number_of_lines) {
  if (number_of_lines == number_of_lines_)
    return;
  number_of_lines_ = number_of_lines;
  number_of_visible_lines_ = number_of_lines;
  Redraw();
}

void ActivityIndicatorView::set_number_of_visible_lines(
    const int number_of_visible_lines) {
  if (number_of_visible_lines == number_of_visible_lines_)
    return;
  number_of_visible_lines_ = number_of_visible_lines;
  Redraw();
}

void ActivityIndicatorView::set_start_color(const NVGcolor start_color) {
  if (nvgCompareColor(start_color, start_color_))
    return;
  start_color_ = start_color;
  Redraw();
}

}  // namespace moui
