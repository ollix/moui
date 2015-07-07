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

#ifndef MOUI_WIDGETS_SCROLLER_H_
#define MOUI_WIDGETS_SCROLLER_H_

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace moui {

// The `Scroller` widget controls scrolling of a document view within the clip
// view of a scroll view.
class Scroller : public Widget {
 public:
  // The direction of the scroller.
  enum class Direction {
    kHorizontal,
    kVertical,
  };

  explicit Scroller(const Direction direction);
  ~Scroller();

  // Hides the scroller in animation. The animation will be canceled
  // and return to the opaque state immediately if `Redraw()` is called.
  void HideInAnimation();

  // Inherited from `Widget` class.
  void Redraw();

  // Inherited from `Widget` class.
  void SetHidden(const bool hidden);

  // Accessors and setters.
  double knob_position() const { return knob_proportion_; }
  void set_knob_position(const double value);
  double knob_proportion() const { return knob_proportion_; }
  void set_knob_proportion(const double value);
  bool shows_scrollers_on_both_directions() const {
    return shows_scrollers_on_both_directions_;
  }
  void set_shows_scrollers_on_both_directions(const bool value);

 private:
  // Inherited from `Widget` class.
  void Render(NVGcontext* context) final;

  // Renders the knob.
  void RenderKnob(NVGcontext* context, const float position,
                  const float length) const;

  // Inherited from `Widget` class.
  void WidgetViewDidRender(NVGcontext* context) final;

  // Inherited from `Widget` class.
  bool WidgetViewWillRender(NVGcontext* context) final;

  // Keeps the timestamp when starting animation.
  double animation_initial_timestamp_;

  // Keeps the progress of the current animation.
  float animation_progress_;

  // Indicates the current direction of the scroller.
  Direction direction_;

  // Indicates whether the scroller is hiding in animation. The hiding
  // animation is triggered by the `HideInAnimation()` method.
  bool hiding_in_animation_;

  // The position of the knob represented from 0.0 (indicating the top or left
  // end) to 1.0 (the bottom or right end).
  double knob_position_;

  // The proportion of the knob slot the knob should fill.
  double knob_proportion_;

  // Indicates whether scrollers are displayed on both horizontal and vertical
  // directions. If `true`, the intersection corner of both tracks is reserved
  // as an empty space. The default value is `false`.
  bool shows_scrollers_on_both_directions_;

  DISALLOW_COPY_AND_ASSIGN(Scroller);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_SCROLLER_H_
