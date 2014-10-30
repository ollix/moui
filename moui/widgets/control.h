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

#ifndef MOUI_WIDGETS_CONTROL_H_
#define MOUI_WIDGETS_CONTROL_H_

#include <functional>
#include <vector>

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace moui {

// The bitmask type for all kinds of events possible for control objects.
enum ControlEvents {
  // A system event canceling the current touches for the control.
  kTouchCancel = 1 << 0,
  // A touch-down event in the control.
  kTouchDown = 1 << 1,
  // An event where a finger is dragged into the bounds of the control.
  kTouchDragEnter = 1 << 2,
  // An event where a finger is dragged from within a control to outside its
  // bounds.v
  kTouchDragExit = 1 << 3,
  // An event where a finger is dragged inside the bounds of the control.
  kTouchDragInside = 1 << 4,
  // An event where a finger is dragged just outside the bounds of the control.
  kTouchDragOutside = 1 << 5,
  // A touch-up event in the control where the finger is inside the bounds of
  // the control.
  kTouchUpInside = 1 << 6,
  // A touch-up event in the control where the finger is outside the bounds of
  // the control.
  kTouchUpOutside = 1 << 7,
  // All touch events.
  kAllTouchEvents = 0xFF,
};

// The bitmask type for control-state constants. A control object should always
// have at least one state.
enum ControlState {
  // The normal, or default state of a control. Enabled but neither selected
  // nor highlighted.
  kNormal = 0x01 << 0,
  // Highlighted state of a control. By default, a control is not highlighted.
  // A control sets and clears this state automatically when a touch enters and
  // exits during tracking and when there is a touch up.
  kHighlighted = 0x01 << 1,
  // Disabled state of a control. This state indicates that the control is
  // currently disabled.
  kDisabled = 0x01 << 2,
  // Selected state of a control. Can be use to display different appearance
  // in subclasses.
  kSelected = 0x01 << 3,
};

// This is the base class for control objects such as buttons that convey user
// intent to the application.
class Control : public Widget {
 public:
  Control();
  ~Control();

  // Binds a function or class method to handle particular control events.
  // The signature of the callback function must be `void(Control)`.
  //
  // Examples:
  // BindAction(events, Function)  // function
  // BindAction(events, &Class::Method)  // class method
  template<class Callback>
  void BindAction(const ControlEvents events, Callback&& callback) {
    auto action = new Action;
    action->callback = std::bind(callback, this);
    action->control_events = events;
    action->target = nullptr;
    actions_.push_back(action);
  }

  // Binds an instance method to handle particular control events.
  // The signature of the callback function must be `void(Control)`.
  //
  // Example: BindAction(events, &Class::Method, instance)
  template<class Callback, class TargetType>
  void BindAction(const ControlEvents events, Callback&& callback,
                  TargetType&& target) {
    auto action = new Action;
    action->callback = std::bind(callback, target, this);
    action->control_events = events;
    action->target = reinterpret_cast<void*>(target);
    actions_.push_back(action);
  }

  // Returns true if the current state is disabled.
  bool IsDisabled() const;

  // Returns true if the current state is highlighted.
  bool IsHighlighted() const;

  // Returns true if the current state is selected.
  bool IsSelected() const;

  // Sets or clears the disabled state. The widget will be drawn if the state
  // does change.
  void SetDisabled(const bool disabled);

  // Sets or clears the highlighted state. The widget will be drawn if the
  // state does change.
  void SetHighlighted(const bool highlighted);

  // Sets or clears the selected state. The widget will be drawn if the state
  // does change.
  void SetSelected(const bool selected);

  // Unbinds an action paired with the callback and control events.
  template<class Callback>
  void UnbindAction(const ControlEvents events, Callback&& callback) {
    const std::function<void()> function = std::bind(callback, this);
    UnbindActions(events, &function, nullptr);
  }

  // Unbinds an action paired with the target, callback, and control events.
  template<class Callback, class TargetType>
  void UnbindAction(const ControlEvents events, Callback&& callback,
                    TargetType&& target) {
    const std::function<void()> function = std::bind(callback, target, this);
    UnbindActions(events, &function, reinterpret_cast<void*>(target));
  }

  // Unbinds all actions paired with control events, callback, and target.
  // If the callback is nullptr, all instance methods paired with the target
  // will be unbinded. If both callback and target are nullptr, all actions
  // with matched control events will be unbinded.
  void UnbindActions(const ControlEvents events,
                     const std::function<void()>* callback,
                     const void* target);

  // Accessors and setters.
  int highlighted_margin() const { return highlighted_margin_; }
  void set_highlighted_margin(const int margin) {
    highlighted_margin_ = margin;
  }
  int touch_down_margin() const { return touch_down_margin_; }
  void set_touch_down_margin(const int margin) { touch_down_margin_ = margin; }

 private:
  // An action helds a function callback that will be fired when any of the
  // control events is matched.
  struct Action {
    std::function<void()> callback;  // the callback function
    ControlEvents control_events;  // fires the callback when any is matched
    void* target;  // nullptr or the instance the callback belongs to.
  };

  // Responds to the passed control events that populated by HandleEvent().
  void HandleControlEvents(const ControlEvents events);

  // Inherited from Widget class.
  virtual void HandleEvent(Event* event) override final;

  // Inherited from Widget class.
  virtual bool ShouldHandleEvent(const Point location) override final;

  // Holds a list of all binded actions. HandleControlEvents() will iterate
  // the list to fire callbacks with matched control events.
  std::vector<Action*> actions_;

  // The margin in points expanding the widget's bounding box as highlighted
  // area.
  int highlighted_margin_;

  // A bitmask value that indicates the state of a control. A control can have
  // more than one state at a time.
  ControlState state_;

  // The margin in points expanding the widget's boudning box as touch down
  // area.
  int touch_down_margin_;

  DISALLOW_COPY_AND_ASSIGN(Control);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_CONTROL_H_
