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
// Control class. In addtion, this class provides methods for setting the
// title, render function, and other appearance properties to change the
// appearance for each button state.
class Button : public Control {
 public:
  // Defines inset distances for the title label.
  struct EdgeInsets {
    int top;
    int right;
    int bottom;
    int left;
  };

  // The style of the highlighted state to render if no corresponded render
  // function is binded.
  enum class Style {
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
  // state. Calling another `BindRenderFunction()` for the same state will
  // overwrite the previous one.
  //
  // Examples:
  // BindRenderFunction(state, Function)  // function
  // BindRenderFunction(state, &Class::Method)  // class method
  template<class Callback>
  void BindRenderFunction(const ControlState state, Callback&& callback) {
    UnbindRenderFunction(state);
    render_functions_[GetControlStateIndex(state)] = \
        std::bind(callback, std::placeholders::_1);
  }

  // Binds an instance method for rendering the button with passed state.
  // Calling another `BindRenderFunction()` for the same state will overwrite
  // the previous one.
  //
  // Example: BindRenderFunction(state, &Class::Method, instance)
  template<class Callback, class TargetType>
  void BindRenderFunction(const ControlState state, Callback&& callback,
                          TargetType&& target) {
    UnbindRenderFunction(state);
    render_functions_[GetControlStateIndex(state)] = \
        std::bind(callback, target, std::placeholders::_1);
  }

  // Returns the current title color that is displayed on the button.
  NVGcolor GetCurrentTitleColor() const;

  // Returns the current title that is displayed on the button.
  std::string GetCurrentTitle() const;

  // Returns the title associated with the specified state. If no title has
  // been set for the specific state, this method returns the title associated
  // with the ControlState::kNormal state.
  std::string GetTitle(const ControlState state) const;

  // Returns the title color associated with the specified state.
  NVGcolor GetTitleColor(const ControlState state) const;

  // Resets framebuffers of every state to clear all cached renderings.
  void ResetFramebuffers();

  // Sets the title to use for the specified state.
  void SetTitle(const std::string& title, const ControlState state);

  // Sets the title color to use for the specified state.
  void SetTitleColor(const NVGcolor color, const ControlState state);

  // Unbinds the render function for a control state.
  void UnbindRenderFunction(const ControlState state);

  // Accessors and setters.
  bool adjusts_button_height_to_fit_title_label() const {
    return adjusts_button_height_to_fit_title_label_;
  }
  void set_adjusts_button_height_to_fit_title_label(const bool value);
  Style default_disabled_style() const {
    return default_disabled_style_;
  }
  void set_default_disabled_style(const Style style);
  Style default_highlighted_style() const {
    return default_highlighted_style_;
  }
  void set_default_highlighted_style(const Style style);
  EdgeInsets title_edge_insets() const { return title_edge_insets_; }
  void set_title_edge_insets(const EdgeInsets edge_insets);
  Label* title_label() const { return title_label_; }

 protected:
  // Inherited from Widget class. Resets all framebuffers.
  virtual void ContextWillChange(NVGcontext* context) override;

 private:
  // Executes the render function for passed state or fills white background
  // if nothing binded.
  void ExecuteRenderFunction(const ControlState state, NVGcontext* context);

  // Returns the index of the specified control state.
  int GetControlStateIndex(const ControlState state) const;

  // Inherited from Widget class. This method takes control of how to render
  // the button. Subclasses should never override this method either.
  // To render customized appearance. Use `BindRenderFunction()` to bind a
  // function for rendering a specific control state.
  virtual void Render(NVGcontext* context) override final;

  // Inherited from Widget class. Renders for the current state into the
  // corresponded framebuffer.
  virtual void RenderFramebuffer(NVGcontext* context) override final;

  // Returns true if a render function is binded to the passed control state.
  bool RenderFunctionIsBinded(const ControlState state) const;

  // Updates the passed framebuffer offscreen according to passed parameters.
  void UpdateFramebuffer(const ControlState state,
                         const bool renders_default_disabled_effect,
                         const bool renders_default_highlighted_effect,
                         NVGcontext* context, NVGLUframebuffer** framebuffer);

  // Updates the title label's attribute based on the button's current state.
  void UpdateTitleLabel();

  // Inherited from Widget class. Updates the title label's attributes to
  // adapt the button's control state and renders the current state to
  // corresonded framebuffer offscreen.
  virtual bool WidgetViewWillRender(NVGcontext* context) override final;

  // Indicates whether the height of the button should be increased
  // automatically in order to fit the title label's vertical size.
  // The adjustment not only respects button's height but also vertical values
  // defined in `title_edge_insets_`. The default value is false.
  bool adjusts_button_height_to_fit_title_label_;

  // The style used to render default disabled state when no corresponded
  // render function is binded. The default style is kSemiTransparent. Note
  // that this value has nothing to do with any child widget including the
  // managed title label.
  Style default_disabled_style_;

  // The style used to render default highlighted state when no corresponded
  // render function is binded. The default style is kTranslucentBlack. Note
  // that this value has nothing to do with any child widget including the
  // managed title label.
  Style default_highlighted_style_;

  // The framebuffer for rendering the button in disabled state.
  NVGLUframebuffer* disabled_state_framebuffer_;

  // The framebuffer for rendering the button in highlighted state.
  NVGLUframebuffer* highlighted_state_framebuffer_;

  // The framebuffer for rendering the button in normal state.
  NVGLUframebuffer* normal_state_framebuffer_;

  // The framebuffer for rendering the button in normal state with default
  // highlighted effect.
  NVGLUframebuffer* normal_state_with_highlighted_effect_framebuffer_;

  // Keeps the binded render functions for different control states. The vector
  // will be initialized in constructor to have the same number of elemens as
  // control states. The element position corresponded to a control state is
  // determined by `GetControlStateIndex()`. Each element could be NULL to
  // represent no binded render function.
  std::vector<std::function<void(NVGcontext*)>> render_functions_;

  // The framebuffer for rendering the button in selected state.
  NVGLUframebuffer* selected_state_framebuffer_;

  // The framebuffer for rendering the button in selected state with default
  // highlighted effect.
  NVGLUframebuffer* selected_state_with_highlighted_effect_framebuffer_;

  // Records title colors for every control states.
  std::vector<NVGcolor> title_colors_;

  // The inset or outset margins for the rectangle around the button's title
  // text.
  EdgeInsets title_edge_insets_;

  // The label widget that displays the current title for a button.
  Label* title_label_;

  // Records titles for every control states.
  std::vector<std::string> titles_;

  DISALLOW_COPY_AND_ASSIGN(Button);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_BUTTON_H_
