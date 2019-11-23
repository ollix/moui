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

#ifndef MOUI_WIDGETS_BUTTON_H_
#define MOUI_WIDGETS_BUTTON_H_

#include <functional>
#include <string>
#include <vector>

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/control.h"

namespace moui {

// Forward declaration.
class Label;

// A button intercepts touch events and calls binded action functions
// accordingly. Methods for settings action functions are inherted from the
// `Control` class. In addtion, this class provides methods for setting the
// title, render function, and other appearance properties to change the
// appearance for each button state.
class Button : public Control {
 public:
  // The style of the highlighted state to render if no corresponded render
  // function is binded.
  enum class Style {
    // Renders the normal state without any change.
    kNone,
    // Renders the state as semi-transparent.
    kSemiTransparent,
    // Renders the state with blended translucent black foreground.
    kTranslucentBlack,
    // Renders the state with blended translucent white foreground.
    kTranslucentWhite,
  };

  Button();
  ~Button();

  // Binds a function or class method for rendering the button with passed
  // states. Calling another `BindRenderFunction()` for the same state will
  // overwrite the previous one.
  //
  // Examples:
  // BindRenderFunction(state, Function)  // function
  // BindRenderFunction(state, &Class::Method)  // class method
  template<class Callback>
  void BindRenderFunction(const ControlState states, Callback&& callback) {
    UnbindRenderFunction(states);
    std::function<void(Button*, NVGcontext*)> render_function = \
        std::bind(callback, std::placeholders::_1, std::placeholders::_2);
    BindRenderFunction(states, render_function);
    Redraw();
  }

  // Binds an instance method for rendering the button with passed states.
  // Calling another `BindRenderFunction()` for the same state will overwrite
  // the previous one.
  //
  // Example: BindRenderFunction(state, &Class::Method, instance)
  template<class Callback, class TargetType>
  void BindRenderFunction(const ControlState states, Callback&& callback,
                          TargetType&& target) {
    UnbindRenderFunction(states);
    std::function<void(Button*, NVGcontext*)> render_function = \
        std::bind(callback, target, std::placeholders::_1,
                  std::placeholders::_2);
    BindRenderFunction(states, render_function);
    Redraw();
  }

  // Returns the current title color that is displayed on the button.
  NVGcolor GetCurrentTitleColor() const;

  // Returns the current title that is displayed on the button.
  std::string GetCurrentTitle() const;

  // Returns the title associated with the specified state. If no title has
  // been set for the specific state, this method returns the title associated
  // with the `ControlState::kNormal` state.
  std::string GetTitle(const ControlState state) const;

  // Returns the title color associated with the specified state.
  NVGcolor GetTitleColor(const ControlState state) const;

  // Resets framebuffers of every state to clear all cached renderings.
  void ResetFramebuffers();

  // Sets the title to use for the specified states.
  void SetTitle(const std::string& title, const ControlState states);

  // Sets the title color to use for the specified states.
  void SetTitleColor(const NVGcolor color, const ControlState states);

  // Unbinds the render function for a control states.
  void UnbindRenderFunction(const ControlState states);

  // Accessors and setters.
  bool adjusts_button_height_to_fit_title_label() const {
    return adjusts_button_height_to_fit_title_label_;
  }
  void set_adjusts_button_height_to_fit_title_label(const bool value);
  bool adjusts_button_width_to_fit_title_label() const {
    return adjusts_button_height_to_fit_title_label_;
  }
  void set_adjusts_button_width_to_fit_title_label(const bool value);
  float darkness() const { return darkness_; }
  void set_darkness(const float darkness) { darkness_ = darkness; }
  Style default_disabled_style() const {
    return default_disabled_style_;
  }
  void set_default_disabled_style(const Style style);
  Style default_highlighted_style() const {
    return default_highlighted_style_;
  }
  void set_default_highlighted_style(const Style style);
  float semi_transparent_style_opacity() const {
    return semi_transparent_style_opacity_;
  }
  void set_semi_transparent_style_opacity(const float opacity);
  EdgeInsets title_edge_insets() const { return title_edge_insets_; }
  void set_title_edge_insets(const EdgeInsets edge_insets);
  Label* title_label() const { return title_label_; }

 protected:
  // Inherited from `Widget` class. Resets all framebuffers.
  void ContextWillChange(NVGcontext* context) override;

  // Inherited from `Widget` class.
  void HandleMemoryWarning(NVGcontext* context) override;

  // Inherited from `Widget` class. Stops transitioning between different
  // control states once the transition is done.
  void WidgetDidRender(NVGcontext* context) override;

  // Inherited from `Widget` class. Updates the title label's attributes to
  // adapt the button's control state and renders the current state to
  // corresonded framebuffer offscreen.
  bool WidgetViewWillRender(NVGcontext* context) override;

 private:
  // The states for animaing the transition between different control states.
  struct TransitionStates {
    // Indicates whether the transition is happening.
    bool is_transitioning;
    // Indicates the timestamp of initiating the transition. The progress
    // value at this time point should be 0.0.
    double initial_timestamp;
    // Indicates the duration in seconds used for the transition.
    double duration;
    // Records the transition progress, specified as a value from 0.0 to 1.0.
    float progress;
    // The framebuffer to render while transitioning.
    NVGframebuffer* framebuffer;
    // Keeps the `title_label_`'s text color that should be transitioned from.
    NVGcolor previous_title_color;
  };

  // Binda a render function for rendering the button with passed states.
  void BindRenderFunction(
      const ControlState states,
      std::function<void(Button*, NVGcontext*)> render_function);

  // Executes the render function for passed state or fills white background
  // if nothing binded.
  void ExecuteRenderFunction(NVGcontext* context, const ControlState state);

  // Returns the index of the specified control state.
  int GetControlStateIndex(const ControlState state) const;

  // Inherited from `Widget` class. This method takes control of how to render
  // the button. Subclasses should never override this method either.
  // To render customized appearance. Use `BindRenderFunction()` to bind a
  // function for rendering a specific control state.
  void Render(NVGcontext* context) final;

  // Inherited from `Widget` class. Renders for the current state into the
  // corresponded framebuffer.
  void RenderFramebuffer(NVGcontext* context) final;

  // Renders the specified `control_state` to the passed `framebuffer`.
  // Returns `false` on failure.
  bool RenderFramebufferForControlState(
      NVGcontext* context, NVGframebuffer** framebuffer,
      const ControlState control_state,
      const bool renders_default_disabled_effect,
      const bool renders_default_highlighted_effect);

  // Renders the button's current transition state to the passed `framebuffer`.
  // Returns `false` on failure.
  bool RenderFramebufferForTransition(NVGcontext* context,
                                      NVGframebuffer** framebuffer);

  // Returns `true` if a render function is binded to the passed control state.
  bool RenderFunctionIsBinded(const ControlState state) const;

  // Stops transitioning between different control states. The is a callback
  // function for `kTouchCancel`, `kTouchUpInside` and `kTouchUpOutside`
  // control events. And it is called in `WidgetDidRender()` once the
  // transition is done.
  void StopTransitioningBetweenControlStates(Control* control);

  // Starts transitioning between different control states. This is a callback
  // function for both `kTouchDragEnter` and `kTouchDragExit` control events.
  void TransitionBetweenControlStates(Control* control);

  // Updates the title label's attributes based on the button's current state.
  // Returns `true` if the dimensions of the button or its title label are
  // changed.
  bool UpdateTitleLabel(NVGcontext* context);

  // Indicates whether the height of the button should be increased
  // automatically in order to fit the title label's vertical size.
  // The adjustment not only respects button's height but also vertical values
  // defined in `title_edge_insets_`. The default value is `false`.
  bool adjusts_button_height_to_fit_title_label_;

  // Indicates whether the width of the button should be changed automatically
  // in order to fit the title label's text. The default value is `false`.
  bool adjusts_button_width_to_fit_title_label_;

  // Keeps the reference to the framebuffer pointer of current control state.
  // This value is updated in the `RenderFramebuffer()` method.
  NVGframebuffer* current_framebuffer_;
  
  // Indiates the darkness (0-1) applied to the button.
  float darkness_;

  // The style used to render default disabled state when no corresponded
  // render function is binded. The default style is `kSemiTransparent`. Note
  // that this value has nothing to do with any child widget including the
  // managed title label.
  Style default_disabled_style_;

  // The style used to render default highlighted state when no corresponded
  // render function is binded. The default style is `kTranslucentBlack`. Note
  // that this value has nothing to do with any child widget including the
  // managed title label.
  Style default_highlighted_style_;

  // The framebuffer for rendering the button in disabled state.
  NVGframebuffer* disabled_state_framebuffer_;

  // The reference to the framebuffer pointer that will be actually rendered
  // in the `Render()` method. This value is updated in the
  // `RenderFramebuffer()` method.
  NVGframebuffer* final_framebuffer_;

  // The framebuffer for rendering the button in highlighted state.
  NVGframebuffer* highlighted_state_framebuffer_;

  // The framebuffer for rendering the button in normal state.
  NVGframebuffer* normal_state_framebuffer_;

  // The framebuffer for rendering the button in normal state with default
  // highlighted effect.
  NVGframebuffer* normal_state_with_highlighted_effect_framebuffer_;

  // Keeps the reference to the framebuffer pointer of previous control state.
  // This value is updated in the `RenderFramebuffer()` method.
  NVGframebuffer* previous_framebuffer_;

  // Keeps the binded render functions for different control states. The vector
  // will be initialized in constructor to have the same number of elemens as
  // control states. The element position corresponded to a control state is
  // determined by `GetControlStateIndex()`. Each element could be `NULL` to
  // represent no binded render function.
  std::vector<std::function<void(Button*, NVGcontext*)>> render_functions_;

  // The framebuffer for rendering the button in selected state.
  NVGframebuffer* selected_state_framebuffer_;

  // The framebuffer for rendering the button in selected state with default
  // highlighted effect.
  NVGframebuffer* selected_state_with_highlighted_effect_framebuffer_;

  // Indicates the opacity used when rendering the button for
  // `Style::kSemiTransparent`. The default value is 0.5.
  float semi_transparent_style_opacity_;

  // Records title colors for every control states.
  std::vector<NVGcolor> title_colors_;

  // The inset or outset margins for the rectangle around the button's title
  // text.
  EdgeInsets title_edge_insets_;

  // The label widget that displays the current title for a button.
  Label* title_label_;

  // Records titles for every control states.
  std::vector<std::string> titles_;

  // Keeps the transition states for animating the transition between different
  // control states.
  TransitionStates transition_states_;

  DISALLOW_COPY_AND_ASSIGN(Button);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_BUTTON_H_
