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

namespace {

// The default margin in points to expand control's bounding box for larger
// region to toggle the highlighted state on handheld devices.
const int kHandheldDeviceHighlightedMargin = 48;

// The default margin in points to expand control's bounding box for larger
// region to receive the touch down event on handheld devices.
const int kHandheldDeviceTouchDownMargin = 12;

}  // namespace

namespace moui {

Control::Control() : Widget(), highlighted_margin_(0),
                     state_(ControlState::kNormal), touch_down_margin_(0) {
  if (Device::GetCategory() != Device::Category::kDesktop) {
    highlighted_margin_ = kHandheldDeviceHighlightedMargin;
    touch_down_margin_ = kHandheldDeviceTouchDownMargin;
  }
}

Control::~Control() {
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

}  // namespace moui
