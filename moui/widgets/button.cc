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

#include "moui/widgets/button.h"

#include <cmath>

#include "moui/base.h"
#include "moui/core/device.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/control.h"
#include "moui/widgets/label.h"
#include "moui/widgets/widget_view.h"

namespace {

// The index corresponded to each control state.
enum class ControlStateIndex {
  kNormal = 0,
  kHighlighted,
  kDisabled,
  kSelected,
};

// The alpha value for rendering default disabled state.
const float kDefaultDisabledStateAlpha = 0.5;

// The alpha value for rendering default highlighted state.
const float kDefaultHighlightedStateAlpha = 0.5;

// The number of ControlStates constants.
const int kNumberOfControlStates = 4;

}  // namespace

namespace moui {

Button::Button() : adjusts_button_height_to_fit_title_label_(false),
                   default_disabled_style_(Style::kSemiTransparent),
                   default_highlighted_style_(Style::kTranslucentBlack),
                   disabled_state_framebuffer_(nullptr),
                   highlighted_state_framebuffer_(nullptr),
                   normal_state_framebuffer_(nullptr),
                   normal_state_with_highlighted_effect_framebuffer_(nullptr),
                   selected_state_framebuffer_(nullptr),
                   selected_state_with_highlighted_effect_framebuffer_(nullptr),
                   title_edge_insets_({0, 0, 0, 0}),
                   title_label_(new Label) {
  title_label_->set_number_of_lines(1);
  SetHeight(Widget::Unit::kPoint, 44);

  const NVGcolor kDefaultColor = nvgRGBA(0, 0, 0, 255);
  for (int i = 0; i < kNumberOfControlStates; ++i) {
    render_functions_.push_back(NULL);
    title_colors_.push_back(kDefaultColor);
    titles_.push_back("");
  }
  SetTitleColor(nvgRGBA(0, 122, 255, 255), ControlState::kNormal);
  SetTitleColor(nvgRGBA(0, 122, 255, 255), ControlState::kSelected);
  SetTitleColor(nvgRGBA(0, 122, 255, 127.5), ControlState::kHighlighted);
  SetTitleColor(nvgRGBA(0, 122, 255, 127.5), ControlState::kDisabled);

  title_label_->set_font_size(18);
  title_label_->set_is_opaque(false);
  title_label_->set_text_horizontal_alignment(Label::Alignment::kCenter);
  title_label_->set_text_vertical_alignment(Label::Alignment::kMiddle);
  AddChild(title_label_);
}

Button::~Button() {
  delete title_label_;
}

void Button::ContextWillChange(NVGcontext* context) {
  Control::ContextWillChange(context);
  ResetFramebuffers();
}

void Button::ExecuteRenderFunction(NVGcontext* context,
                                   const ControlState state) {
  auto render_function = render_functions_[GetControlStateIndex(state)];
  if (render_function != NULL) {
    render_function(context);
    return;
  }

  // Fills configured background color if there is no binded render function.
  if (is_opaque()) {
    nvgBeginPath(context);
    nvgRect(context, 0, 0, GetWidth(), GetHeight());
    nvgFillColor(context, background_color());
    nvgFill(context);
  }
}

int Button::GetControlStateIndex(const ControlState state) const {
  ControlStateIndex index;
  switch (state) {
    case ControlState::kDisabled:
      index = ControlStateIndex::kDisabled;
      break;
    case ControlState::kHighlighted:
      index = ControlStateIndex::kHighlighted;
      break;
    case ControlState::kNormal:
      index = ControlStateIndex::kNormal;
      break;
    case ControlState::kSelected:
      index = ControlStateIndex::kSelected;
      break;
    default:
      index = ControlStateIndex::kNormal;
  }
  return static_cast<int>(index);
}

NVGcolor Button::GetCurrentTitleColor() const {
  if (IsDisabled())
    return GetTitleColor(ControlState::kDisabled);
  if (IsHighlighted())
    return GetTitleColor(ControlState::kHighlighted);
  if (IsSelected())
    return GetTitleColor(ControlState::kSelected);
  return GetTitleColor(ControlState::kNormal);
}

std::string Button::GetCurrentTitle() const {
  if (IsDisabled())
    return GetTitle(ControlState::kDisabled);
  if (IsHighlighted())
    return GetTitle(ControlState::kHighlighted);
  if (IsSelected())
    return GetTitle(ControlState::kSelected);
  return GetTitle(ControlState::kNormal);
}

std::string Button::GetTitle(const ControlState state) const {
  std::string title = titles_[GetControlStateIndex(state)];
  if (title.empty())
    return titles_[GetControlStateIndex(ControlState::kNormal)];
  return title;
}

NVGcolor Button::GetTitleColor(const ControlState state) const {
  return title_colors_[GetControlStateIndex(state)];
}

void Button::HandleMemoryWarning(NVGcontext* context) {
  Control::HandleMemoryWarning(context);
  ResetFramebuffers();
}

void Button::Render(NVGcontext* context) {
  // Determines the renderbuffer to render.
  NVGLUframebuffer** framebuffer;
  if (IsDisabled()) {
    framebuffer = &disabled_state_framebuffer_;
  } else if (IsHighlighted()) {
    if (RenderFunctionIsBinded(ControlState::kHighlighted))
      framebuffer = &highlighted_state_framebuffer_;
    else if (IsSelected())
      framebuffer = &selected_state_with_highlighted_effect_framebuffer_;
    else
      framebuffer = &normal_state_with_highlighted_effect_framebuffer_;
  } else if (IsSelected()) {
    framebuffer = &selected_state_framebuffer_;
  } else {
    framebuffer = &normal_state_framebuffer_;
  }
  if (*framebuffer == nullptr)
    return;

  const float kWidth = GetWidth();
  const float kHeight = GetHeight();
  nvgBeginPath(context);
  nvgRect(context, 0, 0, kWidth, kHeight);
  NVGpaint paint = nvgImagePattern(context, 0, kHeight, kWidth, kHeight, 0,
                                   (*framebuffer)->image, 1);
  nvgFillPaint(context, paint);
  nvgFill(context);
}

// This method updates the specific framebuffer according to the current state.
// However, the current state may not be the actual state to render. For
// example, if the current state is selected but there is no render function
// binded. The normal state will be rendered instead.
void Button::RenderFramebuffer(NVGcontext* context) {
  // Determines what state to render and which framebuffer to render to.
  NVGLUframebuffer** framebuffer;
  ControlState state = ControlState::kNormal;
  bool renders_default_disabled_effect = false;
  bool renders_default_highlighted_effect = false;
  if (IsDisabled()) {
    framebuffer = &disabled_state_framebuffer_;
    if (RenderFunctionIsBinded(ControlState::kDisabled)) {
      state = ControlState::kDisabled;
    } else {
      renders_default_disabled_effect = true;
      if (IsSelected() && RenderFunctionIsBinded(ControlState::kSelected))
        state = ControlState::kSelected;
    }
  } else if (IsHighlighted()) {
    if (RenderFunctionIsBinded(ControlState::kHighlighted)) {
      framebuffer = &highlighted_state_framebuffer_;
      state = ControlState::kHighlighted;
    } else if (IsSelected() &&
               RenderFunctionIsBinded(ControlState::kSelected)) {
      framebuffer = &selected_state_with_highlighted_effect_framebuffer_;
      state = ControlState::kSelected;
      renders_default_highlighted_effect = true;
    } else if (default_highlighted_style_ == Style::kNone) {
      framebuffer = &normal_state_framebuffer_;
    } else {
      framebuffer = &normal_state_with_highlighted_effect_framebuffer_;
      renders_default_highlighted_effect = true;
    }
  } else if (IsSelected()) {
    framebuffer = &selected_state_framebuffer_;
    if (RenderFunctionIsBinded(ControlState::kSelected))
      state = ControlState::kSelected;
  } else {
    framebuffer = &normal_state_framebuffer_;
  }

  if (*framebuffer == nullptr) {
    UpdateFramebuffer(context, framebuffer, state,
                      renders_default_disabled_effect,
                      renders_default_highlighted_effect);
  }
}

bool Button::RenderFunctionIsBinded(const ControlState state) const {
  return render_functions_[GetControlStateIndex(state)] != NULL;
}

void Button::ResetFramebuffers() {
  moui::nvgDeleteFramebuffer(&disabled_state_framebuffer_);
  moui::nvgDeleteFramebuffer(&highlighted_state_framebuffer_);
  moui::nvgDeleteFramebuffer(&normal_state_framebuffer_);
  moui::nvgDeleteFramebuffer(
      &normal_state_with_highlighted_effect_framebuffer_);
  moui::nvgDeleteFramebuffer(&selected_state_framebuffer_);
  moui::nvgDeleteFramebuffer(
      &selected_state_with_highlighted_effect_framebuffer_);
}

void Button::SetTitle(const std::string& title, const ControlState state) {
  titles_[GetControlStateIndex(state)] = title;
}

void Button::SetTitleColor(const NVGcolor color, const ControlState state) {
  title_colors_[GetControlStateIndex(state)] = color;
}

void Button::UnbindRenderFunction(const ControlState state) {
  render_functions_[GetControlStateIndex(state)] = NULL;

  if (state == ControlState::kNormal) {
    moui::nvgDeleteFramebuffer(&normal_state_framebuffer_);
  } else if (state == ControlState::kHighlighted) {
    moui::nvgDeleteFramebuffer(&highlighted_state_framebuffer_);
    moui::nvgDeleteFramebuffer(
        &normal_state_with_highlighted_effect_framebuffer_);
    moui::nvgDeleteFramebuffer(
        &selected_state_with_highlighted_effect_framebuffer_);
  } else if (state == ControlState::kSelected) {
    moui::nvgDeleteFramebuffer(&selected_state_framebuffer_);
  } else if (state == ControlState::kDisabled) {
    moui::nvgDeleteFramebuffer(&disabled_state_framebuffer_);
  }
}

void Button::UpdateFramebuffer(NVGcontext* context,
                               NVGLUframebuffer** framebuffer,
                               const ControlState state,
                               const bool renders_default_disabled_effect,
                               const bool renders_default_highlighted_effect) {
  float scale_factor;
  if (!BeginFramebufferUpdates(context, framebuffer, &scale_factor))
    return;

  const float kWidth = GetWidth();
  const float kHeight = GetHeight();
  nvgBeginFrame(context, kWidth, kHeight, scale_factor);

  if ((renders_default_disabled_effect &&
       default_disabled_style_ == Style::kSemiTransparent) ||
      (renders_default_highlighted_effect &&
       default_highlighted_style_ == Style::kSemiTransparent))
    nvgGlobalAlpha(context, kDefaultDisabledStateAlpha);
  ExecuteRenderFunction(context, state);
  nvgEndFrame(context);

  // Blends with translucent foreground for default highlighted effect.
  if ((renders_default_disabled_effect &&
       default_disabled_style_ != Style::kSemiTransparent) ||
      (renders_default_highlighted_effect &&
       default_highlighted_style_ != Style::kSemiTransparent)) {
    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    nvgBeginFrame(context, kWidth, kHeight, scale_factor);
    nvgBeginPath(context);
    nvgRect(context, 0, 0, kWidth, kHeight);
    if ((renders_default_disabled_effect &&
         default_disabled_style_ == Style::kTranslucentBlack) ||
        (renders_default_highlighted_effect &&
         default_highlighted_style_ == Style::kTranslucentBlack))
      nvgFillColor(context, nvgRGBA(0, 0, 0, 50));
    else if (renders_default_disabled_effect &&
             default_disabled_style_ == Style::kTranslucentWhite)
      nvgFillColor(context, nvgRGBA(255, 255, 255, 200));
    else if (renders_default_highlighted_effect &&
             default_highlighted_style_ == Style::kTranslucentWhite)
      nvgFillColor(context, nvgRGBA(255, 255, 255, 50));
    nvgFill(context);
    nvgEndFrame(context);
  }

  EndFramebufferUpdates();
}

void Button::UpdateTitleLabel() {
  const std::string title = GetCurrentTitle();
  if (title.empty()) {
    title_label_->SetHidden(true);
    return;
  }

  title_label_->set_text(title);
  title_label_->set_text_color(GetCurrentTitleColor());
  title_label_->SetHidden(false);
  title_label_->SetX(title_edge_insets_.left);
  title_label_->SetY(title_edge_insets_.top);
  // Updates the title label's width and makes sure the text can always fit no
  // matter what the current scale is.
  const float kExpectedTitleLabelWidth = \
      GetWidth() - title_edge_insets_.left - title_edge_insets_.right;
  const float kAcutalTitleLabelWidth = \
      std::ceil(kExpectedTitleLabelWidth * GetMeasuredScale())
      / GetMeasuredScale();
  title_label_->SetWidth(kAcutalTitleLabelWidth);

  // Adjusts the button's height to fit the title label.
  if (adjusts_button_height_to_fit_title_label_) {
    const float kRequiredButtonHeight = title_edge_insets_.top + \
                                        title_label_->GetHeight() + \
                                        title_edge_insets_.bottom;
    if (kRequiredButtonHeight > GetHeight()) {
      SetHeight(kRequiredButtonHeight);
      ResetFramebuffers();
    }
  }
  // Updates the label's height to fit the button's height.
  title_label_->SetHeight(
      GetHeight() - title_edge_insets_.top - title_edge_insets_.bottom);
}

bool Button::WidgetViewWillRender(NVGcontext* context) {
  UpdateTitleLabel();
  return true;
}

void Button::set_adjusts_button_height_to_fit_title_label(const bool value) {
  if (value != adjusts_button_height_to_fit_title_label_) {
    adjusts_button_height_to_fit_title_label_ = value;
    Redraw();
  }
}

void Button::set_default_disabled_style(const Style style) {
  if (style == default_disabled_style_)
    return;

  moui::nvgDeleteFramebuffer(&disabled_state_framebuffer_);
  default_disabled_style_ = style;
}

void Button::set_default_highlighted_style(const Style style) {
  if (style == default_highlighted_style_)
    return;

  moui::nvgDeleteFramebuffer(
      &normal_state_with_highlighted_effect_framebuffer_);
  moui::nvgDeleteFramebuffer(
      &selected_state_with_highlighted_effect_framebuffer_);
  default_highlighted_style_ = style;
}

void Button::set_title_edge_insets(const EdgeInsets edge_insets) {
  if (edge_insets.top == title_edge_insets_.top &&
      edge_insets.left == title_edge_insets_.left &&
      edge_insets.bottom == title_edge_insets_.bottom &&
      edge_insets.right == title_edge_insets_.right) {
    return;
  }
  title_edge_insets_ = edge_insets;
  if (widget_view() != nullptr)
    widget_view()->Redraw();
}

}  // namespace moui
