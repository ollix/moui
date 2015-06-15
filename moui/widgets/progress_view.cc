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

#include "moui/widgets/progress_view.h"

#include <algorithm>
#include <cassert>

#include "moui/nanovg_hook.h"

namespace moui {

ProgressView::ProgressView() : progress_(0),
                               style_(Style::kRoundHorizontalBar) {
  set_is_opaque(false);
  set_bar_color(nvgRGBA(100, 100, 100, 255));
  set_track_color(nvgRGBA(150, 150, 150, 255));
}

ProgressView::~ProgressView() {
}

void ProgressView::Render(NVGcontext* context) {
  switch (style_) {
    case Style::kRoundHorizontalBar:
      return RenderRoundHorizontalBar(context);
    case Style::kSquareHorizontalBar:
      return RenderSquareHorizontalBar(context);
    default:
      assert(false);
  }
}

void ProgressView::RenderRoundHorizontalBar(NVGcontext* context) const {
  const float kWidth = GetWidth();
  const float kHeight = GetHeight();
  nvgStrokeWidth(context, kHeight);
  nvgLineCap(context, NVG_ROUND);

  const float kBarCircleRadius = kHeight / 2;
  const float kLineY = kBarCircleRadius;
  const float kTrackXStart = kBarCircleRadius;
  const float kTrackXEnd = kWidth - kBarCircleRadius;

  // Renders the track.
  if (track_color_.a > 0) {
    nvgBeginPath(context);
    nvgMoveTo(context, kTrackXStart, kLineY);
    nvgLineTo(context, kTrackXEnd, kLineY);
    nvgStrokeColor(context, track_color_);
    nvgStroke(context);
  }

  // Renders the bar.
  if (progress_ == 0 || bar_color_.a == 0)
    return;
  const float kBarWidth = (kTrackXEnd - kTrackXStart) * progress_;
  if (kBarWidth < 1) {
    nvgBeginPath(context);
    nvgCircle(context, kBarCircleRadius, kBarCircleRadius, kBarCircleRadius);
    nvgFillColor(context, bar_color_);
    nvgFill(context);
  } else {
    nvgBeginPath(context);
    nvgMoveTo(context, kTrackXStart, kLineY);
    nvgLineTo(context, kTrackXStart + kBarWidth, kLineY);
    nvgStrokeColor(context, bar_color_);
    nvgStroke(context);
  }
}

void ProgressView::RenderSquareHorizontalBar(NVGcontext* context) const {
  const float kWidth = GetWidth();
  const float kHeight = GetHeight();

  // Renders the track.
  if (track_color_.a > 0) {
    nvgBeginPath(context);
    nvgRect(context, 0, 0, kWidth, kHeight);
    nvgFillColor(context, track_color_);
    nvgFill(context);
  }

  // Renders the bar.
  if (progress_ == 0 || bar_color_.a == 0)
    return;
  nvgBeginPath(context);
  nvgRect(context, 0, 0, std::max(1.0f, kWidth * progress_), kHeight);
  nvgFillColor(context, bar_color_);
  nvgFill(context);
}

void ProgressView::set_bar_color(const NVGcolor bar_color) {
  if (!moui::nvgCompareColor(bar_color, bar_color_)) {
    bar_color_ = bar_color;
    Redraw();
  }
}

void ProgressView::set_progress(const float progress) {
  float expected_progress = progress;
  if (progress_ < 0)
    expected_progress = 0;
  else if (progress_ > 1)
    expected_progress = 1;

  if (expected_progress != progress_) {
    progress_ = expected_progress;
    Redraw();
  }
}

void ProgressView::set_track_color(const NVGcolor track_color) {
  if (!moui::nvgCompareColor(track_color, track_color_)) {
    track_color_ = track_color;
    Redraw();
  }
}

}  // namespace moui
