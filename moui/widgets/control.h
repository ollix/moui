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

  // Binds a function or class method for rendering the control with passed
  // state. Calling another BindRenderFunction() for the same state will
  // overwrite the previous one.
  //
  // Examples:
  // BindRenderFunction(state, Function)  // function
  // BindRenderFunction(state, &Class::Method)  // class method
  template<class Callback>
  void BindRenderFunction(const ControlState state, Callback&& callback) {
    render_functions_[GetStateIndex(state)] = \
        std::bind(callback, std::ref(context_));
  }

  // Binds an instance method for rendering the control with passed state.
  // Calling another BindRenderFunction() for the same state will overwrite
  // the previous one.
  //
  // Example: BindRenderFunction(state, &Class::Method, instance)
  template<class Callback, class TargetType>
  void BindRenderFunction(const ControlState state, Callback&& callback,
                          TargetType&& target) {
    render_functions_[GetStateIndex(state)] = \
        std::bind(callback, target, std::ref(context_));
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

  // Unbinds the render function for a control state.
  void UnbindRenderFunction(const ControlState state);

  // Setters.
  void set_highlighted_margin(const int margin) {
    highlighted_margin_ = margin;
  }
  void set_touch_down_margin(const int margin) { touch_down_margin_ = margin; }

 private:
  // An action helds a function callback that will be fired when any of the
  // control events is matched.
  struct Action {
    std::function<void()> callback;  // the callback function
    ControlEvents control_events;  // fires the callback when any is matched
    void* target;  // nullptr or the instance the callback belongs to.
  };

  // Inherited from Widget class.
  virtual void ContextWillChange(NVGcontext* context) override final;

  // Executes the render function for passed state or fills white background
  // if nothing binded.
  void ExecuteRenderFunction(const ControlState state);

  // Returns the index of a state. A negative number will be returned if the
  // passed state represents more than one states.
  int GetStateIndex(const ControlState state) const;

  // Responds to the passed control events that populated by HandleEvent().
  void HandleControlEvents(const ControlEvents events);

  // Inherited from Widget class.
  virtual void HandleEvent(Event* event) override final;

  // Inherited from Widget class. This method takes control of how to render
  // the control. Subclasses should never override this method either.
  // To render customzied appearance. Use BindRenderFunction() to bind a
  // function for rendering a specific control state.
  virtual void Render(NVGcontext* context) override final;

  // Returns true if a render function is binded to the passed control state.
  bool RenderFunctionIsBinded(const ControlState state) const;

  // Inherited from Widget class.
  virtual bool ShouldHandleEvent(const Point location) override final;

  // Inherited from Widget class. Renders the current state to corresonded
  // framebuffer offscreen.
  virtual void WidgetWillRender(NVGcontext* context) override final;

  // Holds a list of all binded actions. HandleControlEvents() will iterate
  // the list to fire callbacks with matched control events.
  std::vector<Action*> actions_;

  // The framebuffer for rendering the control in disabled state. The rendering
  // will be done in RenderOffscreen() on demand.
  NVGLUframebuffer* disabled_state_framebuffer_;

  // The margin in points expanding the widget's bounding box as highlighted
  // area.
  int highlighted_margin_;

  // The framebuffer for rendering the control in highlighted state. The
  // rendering will be done in RenderOffscreen() on demand.
  NVGLUframebuffer* highlighted_state_framebuffer_;

  // The framebuffer for rendering the control in normal state. The rendering
  // will be done in RenderOffscreen() on demand.
  NVGLUframebuffer* normal_state_framebuffer_;

  // The framebuffer for rendering the control in normal state with default
  // highlighted effect. The rendering will be done in RenderOffscreen() on
  // demand.
  NVGLUframebuffer* normal_state_with_highlighted_effect_framebuffer_;

  // Keeps the binded render functions for different control states. The vector
  // will be initialized in constructor to have the same number of elemens as
  // control states. The element position corresponded to a control state is
  // determined by GetStateIndex(). Each element could be NULL to represent no
  // binded render function.
  std::vector<std::function<void()>> render_functions_;

  // The framebuffer for rendering the control in selected state. The rendering
  // will be done in RenderOffscreen() on demand.
  NVGLUframebuffer* selected_state_framebuffer_;

  // The framebuffer for rendering the control in selected state with default
  // highlighted effect. The rendering will be done in RenderOffscreen() on
  // demand.
  NVGLUframebuffer* selected_state_with_highlighted_effect_framebuffer_;

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
