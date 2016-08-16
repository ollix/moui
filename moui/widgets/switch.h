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

#ifndef MOUI_WIDGETS_SWITCH_H_
#define MOUI_WIDGETS_SWITCH_H_

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/control.h"

namespace moui {

// The `Switch` widget creates and manages the On/Off button.
class Switch : public Control {
 public:
  Switch();
  ~Switch();

  // Returns a boolean value that determines the On/Off state of the switch.
  bool IsOn() const;

  // Sets the state of the switch to On or Off, optionally animating the
  // transition.
  void SetOn(const bool is_on, const bool animated);

  // Setters and accessors.
  NVGcolor tint_color() const { return tint_color_; }
  void set_tint_color(const NVGcolor tint_color);

 private:
  // Inherited from `Widget` class.
  void Render(NVGcontext* context) final;

  // Toggles the state of the switch.
  void ToggleState(Control* control);

  // Inherited from `Widget` class.
  bool WidgetViewWillRender(NVGcontext* context) final;

  double animation_start_timestamp_;

  // Indicates the On/Off state of the switch.
  bool is_on_;

  // Indicates the color used to tint the outline of the switch when it is
  // turned off.
  NVGcolor tint_color_;

  DISALLOW_COPY_AND_ASSIGN(Switch);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_SWITCH_H_
