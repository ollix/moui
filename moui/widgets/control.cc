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

}  // namespace

namespace moui {

Control::Control() : context_(nullptr), default_highlighted_normal_image_(-1),
                     default_highlighted_selected_image_(-1),
                     highlighted_margin_(0), state_(ControlState::kNormal),
                     touch_down_margin_(0) {
  if (Device::GetCategory() != Device::Category::kDesktop) {
    highlighted_margin_ = kHandheldDeviceHighlightedMargin;
    touch_down_margin_ = kHandheldDeviceTouchDownMargin;
  }

  for (int i = 0; i < kNumberOfControlStates; ++i)
    render_functions_.push_back(NULL);
}

Control::~Control() {
  UpdateContext(nullptr);
}

bool Control::ExecuteRenderFunction(const ControlState state) {
  auto render_function = render_functions_[GetStateIndex(state)];
  if (render_function != NULL) {
    render_function();
    return true;
  }
  if (state != ControlState::kNormal)
    return false;

  // Fills white background if there is no function binded to normal state.
  nvgBeginPath(context_);
  nvgRect(context_, 0, 0, GetWidth(), GetHeight());
  nvgFillColor(context_, nvgRGBA(255, 255, 255, 255));
  nvgFill(context_);
  return true;
}

// This method generates the default highlighted image in FBO. It firstly
// renders the control in either normal or selected control state according to
// the corresponded binding function, or while background will be filled if
// there is none. Then a transparent black foreground will be blended to the
// original image to generate the final image. The final image will also be
// cached for subsequent calls.
int Control::GenerateDefaultHighlightedImage(NVGcontext* context) {
  const bool kRendersNormalState = \
      !IsSelected() |
      render_functions_[GetStateIndex(ControlState::kSelected)] == NULL;
  int* highlighted_image = kRendersNormalState ?
                           &default_highlighted_normal_image_ :
                           &default_highlighted_selected_image_;
  if (*highlighted_image >= 0)
    return *highlighted_image;

  const int kWidth = GetWidth();
  const int kHeight = GetHeight();
  const int kScaledWidth = kWidth * Device::GetScreenScaleFactor();
  const int kScaledHeight = kHeight * Device::GetScreenScaleFactor();

  // Creates a new context to avoid conflicting with the original one.
  // `context_` will be restored after finished rendering.
  context_ = nvgCreateGL(NVG_ANTIALIAS);
  NVGLUframebuffer* fbo = nvgluCreateFramebuffer(context_, kScaledWidth,
                                                 kScaledHeight, 0);
  nvgluBindFramebuffer(fbo);
  glViewport(0, 0, kScaledWidth, kScaledHeight);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  nvgBeginFrame(context_, kWidth, kHeight, Device::GetScreenScaleFactor());
  if (kRendersNormalState) {
    ExecuteRenderFunction(ControlState::kNormal);
  } else if (!kRendersNormalState) {
    ExecuteRenderFunction(ControlState::kSelected);
  }
  nvgEndFrame(context_);
  // Blends transparent black foreground.
  glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  nvgBeginFrame(context_, kWidth, kHeight, Device::GetScreenScaleFactor());
  nvgBeginPath(context_);
  nvgRect(context_, 0, 0, kWidth, kHeight);
  nvgFillColor(context_, nvgRGBA(0, 0, 0, 50));
  nvgFill(context_);
  nvgEndFrame(context_);
  // Generates the image and finalization.
  *highlighted_image = nvgCreateImageScreenshot(context, 0, 0, kScaledWidth,
                                                kScaledHeight);
  nvgluBindFramebuffer(NULL);
  nvgluDeleteFramebuffer(context_, fbo);
  nvgDeleteGL(context_);
  glViewport(0, 0, widget_view()->GetWidth() * Device::GetScreenScaleFactor(),
             widget_view()->GetHeight() * Device::GetScreenScaleFactor());
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  context_ =  context;  // makes sure it's consistent in subsequent calls
  return *highlighted_image;
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

  // Fires callbacks with matched control events.
  for (Action* action : actions_) {
    if (action->control_events & events)
      action->callback();
  }

  // Updates the highlighted state and redraw the widget if the state changed.
  if (events & ControlEvents::kTouchDown ||
      events & ControlEvents::kTouchDragEnter) {
    SetHighlighted(true);
  } else if (events & ControlEvents::kTouchCancel ||
             events & ControlEvents::kTouchDragExit ||
             events & ControlEvents::kTouchUpInside) {
    SetHighlighted(false);
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
  UpdateContext(context);

  if (IsDisabled()) {
    if (ExecuteRenderFunction(ControlState::kDisabled))
      return;
    nvgGlobalAlpha(context, kAlphaForDefaultImageInDisabledState);
  }

  if (IsHighlighted()) {
    if (ExecuteRenderFunction(ControlState::kHighlighted))
      return;

    // Draws default image in highlighted state.
    const int kWidth = GetWidth();
    const int kHeight = GetHeight();
    nvgBeginPath(context);
    nvgRect(context, 0, 0, kWidth, kHeight);
    const int kHighlightedImage = GenerateDefaultHighlightedImage(context);
    NVGpaint paint = nvgImagePattern(context, 0, 0, kWidth, kHeight, 0,
                                     kHighlightedImage, 1);
    nvgFillPaint(context, paint);
    nvgFill(context);
    return;
  }

  bool renders_normal_state = true;
  if (IsSelected())
    renders_normal_state = !ExecuteRenderFunction(ControlState::kSelected);
  if (renders_normal_state)
    ExecuteRenderFunction(ControlState::kNormal);

  // Restores the alpha value in disabled state.
  if (IsDisabled())
    nvgGlobalAlpha(context, 1.0);
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

void Control::UpdateContext(NVGcontext* context) {
  if (context == context_)
    return;

  if (context_ != nullptr) {
    nvgDeleteImage(context_, default_highlighted_normal_image_);
    nvgDeleteImage(context_, default_highlighted_selected_image_);
    default_highlighted_normal_image_ = -1;
    default_highlighted_selected_image_ = -1;
  }
  context_ = context;
}

}  // namespace moui
