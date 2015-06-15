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

#ifndef MOUI_WIDGETS_PROGRESS_VIEW_H_
#define MOUI_WIDGETS_PROGRESS_VIEW_H_

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace moui {

// The `ProgressView` widget depicts the progress of a task over time.
class ProgressView : public Widget {
 public:
  // The style of the rendered progress track and progress bar.
  enum class Style {
    // The default style of the progress view. Treats whole progress view's
    // bounding rectangle as progress track but with round corners.
    // The progress bar is drawn horizontally with round corners, too.
    kRoundHorizontalBar,
    // Treats whole progress view's bounding rectangle as progress track.
    // The progress bar is drawn horizontally with square corner.
    kSquareHorizontalBar,
  };

  ProgressView();
  ~ProgressView();

  // Accessors and setters.
  NVGcolor bar_color() const { return bar_color_; }
  void set_bar_color(const NVGcolor bar_color);
  float progress() const { return progress_; }
  void set_progress(const float progress);
  Style style() const { return style_; }
  void set_style(const Style style) { style_ = style; }
  NVGcolor track_color() const { return track_color_; }
  void set_track_color(const NVGcolor bar_color);

 private:
  // Inherited from `Widget` class. Renders the progress view according to the
  // configured `style_`.
  void Render(NVGcontext* context) final;

  // Renders the progress view for the `kRoundHorizontalBar` style.
  void RenderRoundHorizontalBar(NVGcontext* context) const;

  // Renders the progress view for the `kSquareHorizontalBar` style.
  void RenderSquareHorizontalBar(NVGcontext* context) const;

  // The color of the progress bar. The default color is dark gray.
  NVGcolor bar_color_;

  // The progress that calculates the percentage of the progress bar. The valid
  // range is between 0 and 1. The default value is 0.
  float progress_;

  // The style of the progress view to render. The default style is
  // `kRoundHorizontalBar`.
  Style style_;

  // The color of the progress track. The default color is light gray.
  NVGcolor track_color_;

  DISALLOW_COPY_AND_ASSIGN(ProgressView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_PROGRESS_VIEW_H_
