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

#include "moui/widgets/control.h"

#include <vector>

#include "moui/base.h"
#include "moui/core/device.h"
#include "moui/core/event.h"
#include "moui/nanovg_hook.h"
#include "moui/opengl_hook.h"
#include "moui/widgets/control.h"
#include "moui/widgets/widget_view.h"

namespace {

// The alpha value for rendering disabled state if there is no binded render
// function.
const float kAlphaForDefaultImageInDisabledState = 0.5;
// The default margins in points expanding widget's bounding box for larger
// response area on handheld devices.
const int kHandheldDeviceHighlightedMargin = 48;
const int kHandheldDeviceTouchDownMargin = 12;
// The number of ControlStates constants.
const int kNumberOfControlStates = 4;

// Deletes the passed framebuffer and sets it to nullptr.
void DeleteFramebuffer(NVGcontext* context, NVGLUframebuffer** framebuffer) {
  if (*framebuffer != nullptr) {
    nvgluDeleteFramebuffer(context, *framebuffer);
    *framebuffer = nullptr;
  }
}

}  // namespace

namespace moui {

Control::Control()
    : normal_state_with_highlighted_effect_framebuffer_(nullptr),
      selected_state_with_highlighted_effect_framebuffer_(nullptr),
      disabled_state_framebuffer_(nullptr), highlighted_margin_(0),
      highlighted_state_framebuffer_(nullptr),
      normal_state_framebuffer_(nullptr), selected_state_framebuffer_(nullptr),
      state_(ControlState::kNormal), touch_down_margin_(0) {
  if (Device::GetCategory() != Device::Category::kDesktop) {
    highlighted_margin_ = kHandheldDeviceHighlightedMargin;
    touch_down_margin_ = kHandheldDeviceTouchDownMargin;
  }

  for (int i = 0; i < kNumberOfControlStates; ++i)
    render_functions_.push_back(NULL);
}

Control::~Control() {
}

void Control::ContextWillChange(NVGcontext* context) {
  DeleteFramebuffer(context, &disabled_state_framebuffer_);
  DeleteFramebuffer(context, &highlighted_state_framebuffer_);
  DeleteFramebuffer(context, &normal_state_framebuffer_);
  DeleteFramebuffer(context,
                    &normal_state_with_highlighted_effect_framebuffer_);
  DeleteFramebuffer(context, &selected_state_framebuffer_);
  DeleteFramebuffer(context,
                    &selected_state_with_highlighted_effect_framebuffer_);
}

void Control::ExecuteRenderFunction(const ControlState state) {
  auto render_function = render_functions_[GetStateIndex(state)];
  if (render_function != NULL) {
    render_function();
    return;
  }

  // Fills configured background color if there is no binded render function.
  if (is_opaque()) {
    nvgBeginPath(context_);
    nvgRect(context_, 0, 0, GetWidth(), GetHeight());
    nvgFillColor(context_, background_color());
    nvgFill(context_);
  }
}

int Control::GetStateIndex(const ControlState state) const {
  switch(state) {
    case ControlState::kNormal:
      return 0;
    case ControlState::kHighlighted:
      return 1;
    case ControlState::kDisabled:
      return 2;
    case ControlState::kSelected:
      return 3;
  }
}

void Control::HandleControlEvents(const ControlEvents events) {
  if (IsDisabled())
    return;

  // Updates the highlighted state and redraw the widget if the state changed.
  if (events & ControlEvents::kTouchDown ||
      events & ControlEvents::kTouchDragEnter) {
    SetHighlighted(true);
  } else if (events & ControlEvents::kTouchCancel ||
             events & ControlEvents::kTouchDragExit ||
             events & ControlEvents::kTouchUpInside) {
    SetHighlighted(false);
  }

  // Fires callbacks with matched control events.
  for (Action* action : actions_) {
    if (action->control_events & events)
      action->callback();
  }
}

// Handles corresponded control events converted from the passed event.
void Control::HandleEvent(Event* event) {
  if (state_ & ControlState::kDisabled)
    return;

  int control_events = 0;
  // Down.
  if (event->type() == Event::Type::kDown) {
    control_events |= ControlEvents::kTouchDown;
  // Move.
  } else if (event->type() == Event::Type::kMove) {
    const bool kTouchOutside = !CollidePoint(
        static_cast<Point>(event->locations()->front()),  // location
        highlighted_margin_);
    if (IsHighlighted()) {
      if (kTouchOutside) {
        control_events |= ControlEvents::kTouchDragExit;
        control_events |= ControlEvents::kTouchDragOutside;
      } else {
        control_events |= ControlEvents::kTouchDragInside;
      }
    } else {
      if (kTouchOutside) {
        control_events |= ControlEvents::kTouchDragOutside;
      } else {
        control_events |= ControlEvents::kTouchDragEnter;
        control_events |= ControlEvents::kTouchDragInside;
      }
    }
  // Up.
  } else if (event->type() == Event::Type::kUp) {
    if (IsHighlighted()) {
      control_events |= ControlEvents::kTouchUpInside;
    } else {
      control_events |= ControlEvents::kTouchUpOutside;
    }
  // Cancel.
  } else if (event->type() == Event::Type::kCancel) {
    control_events |= ControlEvents::kTouchCancel;
  }
  // Handles the populated control event.
  HandleControlEvents(static_cast<ControlEvents>(control_events));
}

bool Control::IsDisabled() const {
  return (state_ & ControlState::kDisabled) != 0;
}

bool Control::IsHighlighted() const {
  return (state_ & ControlState::kHighlighted) != 0;
}

bool Control::IsSelected() const {
  return (state_ & ControlState::kSelected) != 0;
}

void Control::Render(NVGcontext* context) {
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

  const int kWidth = GetWidth();
  const int kHeight = GetHeight();
  nvgBeginPath(context);
  nvgRect(context, 0, 0, kWidth, kHeight);
  NVGpaint paint = nvgImagePattern(context, 0, kHeight, kWidth, kHeight, 0,
                                   (*framebuffer)->image, 1);
  nvgFillPaint(context, paint);
  nvgFill(context);
}

bool Control::RenderFunctionIsBinded(const ControlState state) const {
  return render_functions_[GetStateIndex(state)] != NULL;
}

void Control::SetDisabled(const bool disabled) {
  if (disabled == IsDisabled())
    return;

  if (disabled)
    state_ = static_cast<ControlState>(state_ | ControlState::kDisabled);
  else
    state_ = static_cast<ControlState>(state_ & ~ControlState::kDisabled);
  Redraw();
}

// If highlighted, sets the highlighted state and removes the normal state.
// If not highlighted, removes the highlighted state and sets the normal state
// if not selected.
void Control::SetHighlighted(const bool highlighted) {
  if (highlighted == IsHighlighted())
    return;

  if (highlighted) {
    state_ = static_cast<ControlState>(state_ & ~ControlState::kNormal);
    state_ = static_cast<ControlState>(state_ | ControlState::kHighlighted);
  } else {
    state_ = static_cast<ControlState>(state_ & ~ControlState::kHighlighted);
    if (!IsSelected())
      state_ = static_cast<ControlState>(state_ | ControlState::kNormal);
  }
  Redraw();
}

// If selected, sets the selected state and removes the normal state. If not
// selated, removes the selected state and sets the normal state if not
// highlighted.
void Control::SetSelected(const bool selected) {
  if (selected == IsSelected())
    return;

  if (selected) {
    state_ = static_cast<ControlState>(state_ & ~ControlState::kNormal);
    state_ = static_cast<ControlState>(state_ | ControlState::kSelected);
  } else {
    state_ = static_cast<ControlState>(state_ & ~ControlState::kSelected);
    if (!IsHighlighted())
      state_ = static_cast<ControlState>(state_ | ControlState::kNormal);
  }
  Redraw();
}

// Returns true if the widget's bounding box plus the disired margin size
// collides the passed location.
bool Control::ShouldHandleEvent(const Point location) {
  return CollidePoint(location, touch_down_margin_);
}

void Control::UnbindActions(const ControlEvents events,
                            const std::function<void()>* callback,
                            const void* target) {
  auto action_iterator = actions_.begin();
  while (action_iterator != actions_.end()) {
    Action* action = *action_iterator;
    if (action->control_events == events &&
        (target == nullptr || action->target == target) &&
        (callback == nullptr ||
         action->callback.target_type() == callback->target_type())) {
      actions_.erase(action_iterator);
      continue;
    }
    ++action_iterator;
  }
}

void Control::UnbindRenderFunction(const ControlState state) {
  render_functions_[GetStateIndex(state)] = NULL;
}

// This method renders to the corresponded framebuffer according to the current
// state. However, the current state may not be the actual state to render.
// For example, if the current state is selected but there is no render
// function binded. The normal state will be rendered instead.
void Control::WidgetViewWillRender(NVGcontext* context) {
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
  // Do nothing if the rendering was done.
  if (*framebuffer != nullptr)
    return;

  const int kWidth = GetWidth();
  const int kHeight = GetHeight();
  BeginRenderbufferUpdates(context, framebuffer);
  nvgBeginFrame(context, kWidth, kHeight, Device::GetScreenScaleFactor());
  if (renders_default_disabled_effect)
    nvgGlobalAlpha(context, kAlphaForDefaultImageInDisabledState);
  ExecuteRenderFunction(state);
  if (renders_default_disabled_effect)
    nvgGlobalAlpha(context, 1);
  nvgEndFrame(context);

  // Blends with transparent black foreground for default highlighted effect.
  if (renders_default_highlighted_effect) {
    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    nvgBeginFrame(context, kWidth, kHeight, Device::GetScreenScaleFactor());
    nvgBeginPath(context);
    nvgRect(context, 0, 0, kWidth, kHeight);
    nvgFillColor(context, nvgRGBA(0, 0, 0, 50));
    nvgFill(context);
    nvgEndFrame(context);
  }

  EndRenderbufferUpdates();
}

}  // namespace moui
