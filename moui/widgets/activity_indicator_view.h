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

#ifndef MOUI_WIDGETS_ACTIVITY_INDICATOR_VIEW_H_
#define MOUI_WIDGETS_ACTIVITY_INDICATOR_VIEW_H_

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace moui {

// The `ActivityIndicatorView` widget depicts the progress of a task over time.
// An activity indicator draws a couple of lines in a clockwise direction with
// the first line always toward the top, and which indicates line index 0.
class ActivityIndicatorView : public Widget {
 public:
  ActivityIndicatorView();
  ~ActivityIndicatorView();

  // Starts the animation of the progress indicator.
  void StartAnimating();

  // Stops the animation of the progress indicator.
  void StopAnimating();

  // Setters and accessors.
  int color_start_line_index() const { return color_start_line_index_; }
  void set_color_start_line_index(const int color_start_line_index);
  NVGcolor last_color() const { return last_color_; }
  void set_last_color(const NVGcolor last_color);
  int number_of_lines() const { return number_of_lines_; }
  void set_number_of_lines(const int number_of_lines);
  int number_of_visible_lines() const { return number_of_visible_lines_; }
  void set_number_of_visible_lines(const int number_of_visible_lines);
  NVGcolor start_color() const { return start_color_; }
  void set_start_color(const NVGcolor start_color);

 private:
  // Inherited from `Widget` class.
  void Render(NVGcontext* context) final;

  // Keeps the value of `color_start_line_index_` when starting animating.
  int animation_color_start_line_index_;

  // Indicates the timestamp the animation starts.
  double animation_start_timestamp_;

  // Indicates the line index that shows the `start_color_`. The value is set
  // to 9 when instantiate a activity indicator view object.
  int color_start_line_index_;

  // Indicates the color applied to the last line.
  NVGcolor last_color_;

  // Indicates the number of lines that should apply graduated colors from the
  // `start_color_` to `last_color_`. All other lines are applied the
  // `last_color_`. The default number is 6.
  int number_of_colored_lines_;

  // Indicates the number of lines to display in the activity indicator. The
  // default number is 12.
  int number_of_lines_;

  // Indicates the number of visible lines started from index 0. The value is
  // set to same number of `number_of_lines_` when calling the
  // `set_number_of_lines()` method.
  int number_of_visible_lines_;

  // Indicates the color applied to the first line.
  NVGcolor start_color_;

  DISALLOW_COPY_AND_ASSIGN(ActivityIndicatorView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_ACTIVITY_INDICATOR_VIEW_H_
