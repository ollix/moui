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

#include <cmath>
#include <cstdint>
#include <vector>

#include "moui/base.h"
#include "moui/core/device.h"
#include "moui/core/event.h"
#include "moui/nanovg_hook.h"

namespace {

// The default margin in points to expand control's bounding box for larger
// region to toggle the highlighted state on handheld devices.
const int kHandheldDeviceHighlightedMargin = 48;

}  // namespace

namespace moui {

Control::Control() : Control(true) {
}

Control::Control(const bool caches_rendering)
    : Widget(caches_rendering), highlighted_margin_(0),
      ignores_upcoming_events_(true), state_(ControlState::kNormal),
      touch_down_margin_(0) {
  if (Device::GetCategory() != Device::Category::kDesktop)
    highlighted_margin_ = kHandheldDeviceHighlightedMargin;
}

Control::~Control() {
  for (Action* action : actions_)
    delete action;
}

bool Control::HandleControlEvents(const ControlEvents events) {
  if (IsDisabled())
    return false;

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
  bool handles_events = false;
  for (Action* action : actions_) {
    if (action->control_events & events) {
      action->callback();
      handles_events = true;
    }
  }
  return handles_events;
}

// Handles corresponded control events converted from the passed event.
bool Control::HandleEvent(Event* event) {
  if (IsDisabled() || ignores_upcoming_events_) {
    return true;
  }

  // Determines the control's current origin related to the corresponded
  // widget view's coordinate system.
  Point origin;
  GetMeasuredBounds(&origin, nullptr);

  int control_events = 0;
  // Down.
  if (event->type() == Event::Type::kDown) {
    control_events |= ControlEvents::kTouchDown;
    down_event_origin_ = origin;
  // Cancels if the control's initial origin has been changed. This situation
  // happens when underlying scroll view is scrolling.
  } else if (down_event_origin_.x != INT_MIN &&
             down_event_origin_.y != INT_MIN &&
             (std::abs(origin.x - down_event_origin_.x) >= 1.2 ||
              std::abs(origin.y - down_event_origin_.y) >= 1.2)) {
    control_events |= ControlEvents::kTouchCancel;
    ignores_upcoming_events_ = true;
  // Move.
  } else if (event->type() == Event::Type::kMove) {
    const bool kTouchOutside = !CollidePoint(
        static_cast<Point>(event->locations()->at(0)),  // location
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
  // Handles the populated control event. The event will be propagated to the
  // next responder if no control event is handled.
  return !HandleControlEvents(static_cast<ControlEvents>(control_events));
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

// Returns `true` if the widget's bounding box plus the disired margin size
// collides the passed location.
bool Control::ShouldHandleEvent(const Point location) {
  if (!CollidePoint(location, touch_down_margin_)) {
    return false;
  }
  ignores_upcoming_events_ = false;
  down_event_origin_ = {INT_MIN, INT_MIN};
  return true;
}

void Control::UnbindActions(const ControlEvents events,
                            const std::function<void()>* callback,
                            const void* target) {
  auto action_iterator = actions_.begin();
  while (action_iterator != actions_.end()) {
    Action* action = *action_iterator;
    if ((action->target != this) &&
        (action->control_events == events ||
         events == ControlEvents::kAllTouchEvents) &&
        (target == nullptr || action->target == target) &&
        (callback == nullptr ||
         action->callback.target_type() == callback->target_type())) {
      actions_.erase(action_iterator);
      delete action;
      continue;
    }
    ++action_iterator;
  }
}

void Control::UnbindAllActions() {
  UnbindActions(ControlEvents::kAllTouchEvents, nullptr, nullptr);
}

}  // namespace moui
