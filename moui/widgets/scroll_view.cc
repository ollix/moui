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

#include "moui/widgets/scroll_view.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

#include "moui/core/clock.h"
#include "moui/core/event.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/scroller.h"
#include "moui/widgets/widget.h"

namespace {

// The duration in seconds for animating the content view to next page.
const float kAnimationDurationPerPage = 0.25;

// The duration in seconds for bouncing the content view.
const double kBounceDuration = 0.3;

// The default deceleration rate.
const float kDefaultDecelerationRate = 0.998;

// The maximum duration in seconds to animate the content view beyond the
// boundary limits.
const double kMaximumReachingBoundaryDuration = 0.15;

// The upper bound of the acceptable velocity to scroll the content view.
const float kMaximumScrollVelocity = 2500;

// The minimum velocity that will enable the mechanism of stopping content view
// gradually while scrolling.
const float kScrollVelocityThreshold = 100;

// Returns the displacement of the passed variables.
float CalculateDisplacement(const float initial_velocity,
                            const float deceleration,
                            const double traveled_time) {
  return initial_velocity * traveled_time + \
         0.5 * deceleration * std::pow(traveled_time, 2);
}

float ReviseScrollVelocity(const float velocity) {
  const float kResult = std::min(kMaximumScrollVelocity, std::abs(velocity));
  return velocity >= 0 ? kResult : -kResult;
}

}  // namespace

namespace moui {

ScrollView::ScrollView()
    : Widget(false), always_bounce_horizontal_(false),
      always_bounce_vertical_(false), always_scroll_both_directions_(true),
      always_scroll_to_next_page_(false),
      deceleration_rate_(kDefaultDecelerationRate), bounces_(true),
      content_view_(new Widget(false)), enables_paging_(false),
      enables_scroll_(true), moves_content_view_to_page_(0), page_width_(0),
      scroll_indicator_insets_({0, 0, 0, 0}),
      shows_horizontal_scroll_indicator_(true),
      shows_vertical_scroll_indicator_(true) {
  // Initializes content view.
  content_view_->set_box_sizing(Widget::BoxSizing::kBorderBox);
  content_view_->set_is_opaque(false);
  content_view_->SetWidth(Widget::Unit::kPercent, 100);
  content_view_->SetHeight(Widget::Unit::kPercent, 100);
  Widget::AddChild(content_view_);

  // Initializes scrollers.
  horizontal_scroller_ = new Scroller(Scroller::Direction::kHorizontal);
  horizontal_scroller_->SetHidden(true);
  Widget::AddChild(horizontal_scroller_);
  vertical_scroller_ = new Scroller(Scroller::Direction::kVertical);
  vertical_scroller_->SetHidden(true);
  Widget::AddChild(vertical_scroller_);

  // Initializes animation states.
  horizontal_animation_states_.is_animating = false;
  horizontal_animation_states_.is_bouncing = false;
  vertical_animation_states_.is_animating = false;
  vertical_animation_states_.is_bouncing = false;
}

ScrollView::~ScrollView() {
  if (frees_children_on_destruction())
    return;

  delete content_view_;
  delete horizontal_scroller_;
  delete vertical_scroller_;
}

// Adds the passed child to the internal `content_view_` actually and sets the
// child's parent to this scroll view.
void ScrollView::AddChild(Widget* child) {
  content_view_->AddChild(child);
  child->set_parent(this);
}

void ScrollView::AnimateContentViewHorizontally(const float origin_x,
                                                const double duration) {
  if (duration <= 0)
    return;

  const float kDisplacement = origin_x - content_view_->GetX();
  if (kDisplacement == 0)
    return;

  const float kInitialVelocity = 2 * kDisplacement / duration;
  const float kDeceleration = (0 - kInitialVelocity) / duration;
  AnimateContentViewHorizontally(origin_x, kInitialVelocity, kDeceleration,
                                 duration);
}

void ScrollView::AnimateContentViewHorizontally(const float origin_x,
                                                const float initial_velocity,
                                                const float deceleration,
                                                const double duration) {
  horizontal_animation_states_.deceleration = deceleration;
  horizontal_animation_states_.is_animating = true;
  horizontal_animation_states_.destination_location = origin_x;
  horizontal_animation_states_.duration = duration;
  horizontal_animation_states_.reaches_boundary_timestamp = -1;
  horizontal_animation_states_.initial_location = content_view_->GetX();
  horizontal_animation_states_.initial_velocity = initial_velocity;
  horizontal_animation_states_.initial_timestamp = -1;
  if (!IsAnimating())
    StartAnimation();
}

void ScrollView::AnimateContentViewOffset(const Point offset,
                                          const double duration) {
  const float kOriginX = -offset.x;
  const float kOriginY = -offset.y;
  AnimateContentViewHorizontally(kOriginX, duration);
  AnimateContentViewVertically(kOriginY, duration);
}

void ScrollView::AnimateContentViewVertically(const float origin_y,
                                              const double duration) {
  if (duration <= 0)
    return;

  const float kDisplacement = origin_y - content_view_->GetY();
  if (kDisplacement == 0)
    return;

  const float kInitialVelocity = 2 * kDisplacement / duration;
  const float kDeceleration = (0 - kInitialVelocity) / duration;
  AnimateContentViewVertically(origin_y, kInitialVelocity, kDeceleration,
                               duration);
}

void ScrollView::AnimateContentViewVertically(const float origin_y,
                                              const float initial_velocity,
                                              const float deceleration,
                                              const double duration) {
  vertical_animation_states_.deceleration = deceleration;
  vertical_animation_states_.is_animating = true;
  vertical_animation_states_.destination_location = origin_y;
  vertical_animation_states_.duration = duration;
  vertical_animation_states_.reaches_boundary_timestamp = -1;
  vertical_animation_states_.initial_location = content_view_->GetY();
  vertical_animation_states_.initial_velocity = initial_velocity;
  vertical_animation_states_.initial_timestamp = -1;
  if (!IsAnimating())
    StartAnimation();
}

void ScrollView::BounceContentView(AnimationStates* animation_state) {
  if (animation_state == &horizontal_animation_states_)
    BounceContentViewHorizontally();
  else if (animation_state == &vertical_animation_states_)
    BounceContentViewVertically();
}

void ScrollView::BounceContentViewHorizontally() {
  // Determines the location to move to.
  float minimum_x, maximum_x;
  GetContentViewBoundaries(&minimum_x, nullptr, &maximum_x, nullptr);
  const float kOriginX = content_view_->GetX();
  float destination_x = kOriginX;
  if (enables_paging_) {
    const int kCurrentPage = GetCurrentPage();
    const float kPageWidth = page_width();
    destination_x = kCurrentPage * -kPageWidth + (GetWidth() - kPageWidth) / 2;
  } else if (kOriginX < minimum_x) {
    destination_x = minimum_x;
  } else if (kOriginX > maximum_x) {
    destination_x = maximum_x;
  }
  // Do nothing if the horizontal location is not changed.
  if (destination_x == kOriginX) {
    horizontal_animation_states_.is_animating = false;
  } else {
    horizontal_animation_states_.is_bouncing = true;
    AnimateContentViewHorizontally(destination_x, kBounceDuration);
  }
}

void ScrollView::BounceContentViewVertically() {
  // Determines the location to move to.
  float minimum_y, maximum_y;
  GetContentViewBoundaries(nullptr, &minimum_y, nullptr, &maximum_y);
  const float kOriginY = content_view_->GetY();
  float destination_y = kOriginY;
  destination_y = std::max(minimum_y, destination_y);
  destination_y = std::min(maximum_y, destination_y);
  // Do nothing if the vertical location is not changed.
  if (destination_y == kOriginY) {
    vertical_animation_states_.is_animating = false;
  } else {
    vertical_animation_states_.is_bouncing = true;
    AnimateContentViewVertically(destination_y, kBounceDuration);
  }
}

bool ScrollView::BringChildToFront(Widget* child) {
  return content_view_->BringChildToFront(child);
}

Point ScrollView::GetContentViewOffset() const {
  return {-content_view_->GetX(), -content_view_->GetY()};
}

float ScrollView::GetContentViewOriginForPage(const int page) const {
  const float kPageWidth = page_width();
  return page * -kPageWidth + (GetWidth() - kPageWidth) / 2;
}

void ScrollView::GetContentViewBoundaries(float* minimum_x,
                                          float* minimum_y,
                                          float* maximum_x,
                                          float* maximum_y) const {
  const float kScrollViewWidth = GetWidth();
  const float kHorizontalPadding = (kScrollViewWidth - page_width()) / 2;
  if (minimum_x != nullptr) {
    *minimum_x = std::min(
        0.0f,
        kScrollViewWidth - content_view_->GetWidth() - kHorizontalPadding);
  }
  if (maximum_x != nullptr) {
    *maximum_x = kHorizontalPadding;
  }
  if (minimum_y != nullptr) {
    *minimum_y = std::min(0.0f, this->GetHeight() - content_view_->GetHeight());
  }
  if (maximum_y != nullptr) {
    *maximum_y = 0;
  }
}

Size ScrollView::GetContentViewSize() const {
  return {content_view_->GetWidth(), content_view_->GetHeight()};
}

int ScrollView::GetCurrentPage() const {
  int page = GetPage(content_view_->GetX());
  page = std::max(0, page);
  page = std::min(GetMaximumPage(), page);
  return page;
}

int ScrollView::GetMaximumPage() const {
  const float kHorizontalPadding = GetWidth() - page_width();
  return (content_view_->GetWidth() - kHorizontalPadding) / page_width();
}

int ScrollView::GetPage(const float origin_x) const {
  const float kPageWidth = page_width();
  if (content_view_->GetWidth() < kPageWidth)
    return 0;
  return (GetWidth() / 2 - origin_x) / kPageWidth;
}

bool ScrollView::GetScrollDirection(ScrollDirection* direction) const {
  if (event_history_.size() < 2)
    return false;

  *direction = static_cast<ScrollDirection>(0);
  ScrollEvent latest_event = event_history_.back();
  ScrollEvent previous_event = event_history_.front();

  const float kOffsetX = latest_event.location.x - previous_event.location.x;
  const float kOffsetY = latest_event.location.y - previous_event.location.y;
  if (kOffsetX == 0 && kOffsetY == 0) {
    return false;
  }

  const float kTheta = std::atan2(kOffsetY, kOffsetX);
  float angle = (kTheta + M_PI / 2) * 180 / M_PI;
  if (angle < 0) angle += 360;
  float half_range = 360.0 / 8 / 2;
  if (acceptable_scroll_directions_ == (ScrollDirection::kHorizontal |
                                        ScrollDirection::kVertical)) {
    if (angle < half_range || angle > (360 - half_range) ||
        (angle > (180 - half_range) && angle < (180 + half_range))) {
      *direction = ScrollDirection::kVertical;
      return true;
    }
    if ((angle > (90 - half_range) && angle < (90 + half_range)) ||
        (angle > (270 - half_range) && angle < (270 + half_range))) {
      *direction = ScrollDirection::kHorizontal;
      return true;
    }
    *direction = static_cast<ScrollDirection>(ScrollDirection::kVertical |
                                              ScrollDirection::kHorizontal);
    return true;
  }
  // Acceptable direction is either horizontal or vertical.
  half_range = 360.0 / 4 / 2;
  if (angle < half_range || angle > (270 + half_range) ||
      (angle > (180 - half_range) && angle < (180 + half_range)))
    *direction = ScrollDirection::kVertical;
  else
    *direction = ScrollDirection::kHorizontal;
  return true;
}

void ScrollView::GetScrollVelocity(float* horizontal_velocity,
                                   float* vertical_velocity) {
  if (horizontal_velocity != nullptr)
    *horizontal_velocity = 0;
  if (vertical_velocity != nullptr)
    *vertical_velocity = 0;
  if (event_history_.size() < 2) {
    return;
  }
  ScrollEvent last_event = event_history_.back();
  ScrollEvent initial_event = {{0, 0}, 0};
  double elapsed_time = -1;
  for (int i = static_cast<int>(event_history_.size()) - 2; i >= 0; --i) {
    initial_event = event_history_[i];
    elapsed_time = last_event.timestamp - initial_event.timestamp;
    // Stops if reached the expected minimum time for calculating the velocity.
    if (elapsed_time > 0.07)
      break;
  }
  if (elapsed_time <= 0) {
    return;
  }

  // If both vertical and horizontal directions are accepted. The velocities
  // of each direction are calculated separated based on the displacements
  // of different directions.
  const float kXDifference = last_event.location.x - initial_event.location.x;
  const float kYDifference = last_event.location.y - initial_event.location.y;
  if ((locked_scroll_directions_ & ScrollDirection::kHorizontal) != 0 &&
      (locked_scroll_directions_ & ScrollDirection::kVertical) != 0) {
    if (horizontal_velocity != nullptr &&
        (locked_scroll_directions_ & ScrollDirection::kHorizontal) != 0) {
      *horizontal_velocity = ReviseScrollVelocity(kXDifference / elapsed_time);
    }
    if (vertical_velocity != nullptr &&
        (locked_scroll_directions_ & ScrollDirection::kVertical) != 0) {
      *vertical_velocity = ReviseScrollVelocity(kYDifference / elapsed_time);
    }
  // However, if only one direction is accepted. The velocity is calculated
  // based on the distance between two event locations.
  } else {
    const float kDistance = \
        std::sqrt(std::pow(kXDifference, 2) + std::pow(kYDifference, 2));
    if (horizontal_velocity != nullptr &&
        (locked_scroll_directions_ & ScrollDirection::kHorizontal) != 0) {
      const float kDisplacement = kXDifference > 0 ? kDistance : -kDistance;
      *horizontal_velocity = ReviseScrollVelocity(kDisplacement / elapsed_time);
    } else if (vertical_velocity != nullptr &&
        (locked_scroll_directions_ & ScrollDirection::kVertical) != 0) {
      const float kDisplacement = kYDifference > 0 ? kDistance : -kDistance;
      *vertical_velocity = ReviseScrollVelocity(kDisplacement / elapsed_time);
    }
  }
}

bool ScrollView::HandleEvent(Event* event) {
  if (horizontal_animation_states_.is_bouncing)
    ignores_upcoming_events_ = true;
  if (ignores_upcoming_events_)
    return true;

  // Finalization.
  if (event->type() == Event::Type::kUp ||
      event->type() == Event::Type::kCancel ||
      event->locations()->size() > 1) {
    ignores_upcoming_events_ = true;
    is_scrolling_ = false;
    StopScrollingGradually();
    // Hides scrollers if not animating.
    if (!horizontal_animation_states_.is_animating &&
        !vertical_animation_states_.is_animating) {
      horizontal_scroller_->HideInAnimation();
      vertical_scroller_->HideInAnimation();
    }
    return false;
  }

  const double kTimestamp = Clock::GetTimestamp();
  Point current_location = event->locations()->at(0);

  // Handles the first receivied event.
  if (event_history_.empty()) {
    StopAnimation();
    initial_scroll_content_view_origin_ = {content_view_->GetX(),
                                           content_view_->GetY()};
  }
  event_history_.push_back({current_location, kTimestamp});

  // Handles the move event.
  if (event->type() != Event::Type::kMove)
    return false;

  if (locked_scroll_directions_ == static_cast<ScrollDirection>(0)) {
    if (always_scroll_both_directions_ &&
        (acceptable_scroll_directions_ & ScrollDirection::kHorizontal) != 0 &&
        (acceptable_scroll_directions_ & ScrollDirection::kVertical) != 0) {
      locked_scroll_directions_ = acceptable_scroll_directions_;
    } else {
      ScrollDirection scroll_direction;
      if (!GetScrollDirection(&scroll_direction))
        return true;
      locked_scroll_directions_ = static_cast<ScrollDirection>(
          acceptable_scroll_directions_ & scroll_direction);
      if (locked_scroll_directions_ == static_cast<ScrollDirection>(0)) {
        ignores_upcoming_events_ = true;
        return true;
      }
    }
    is_scrolling_ = true;
  }

  // Moves the content view.
  ScrollEvent first_event = event_history_.front();
  Point origin = initial_scroll_content_view_origin_;
  if ((locked_scroll_directions_ & ScrollDirection::kHorizontal) != 0)
    origin.x += current_location.x - first_event.location.x;
  if ((locked_scroll_directions_ & ScrollDirection::kVertical) != 0)
    origin.y += current_location.y - first_event.location.y;
  SetContentViewOrigin(origin);
  return false;
}

bool ScrollView::HorizontalScrollingIsAcceptable() const {
  return content_view_->GetWidth() > GetWidth() ||
         (bounces_ && always_bounce_horizontal_);
}

bool ScrollView::InsertChildAboveContentView(Widget* child) {
  return Widget::InsertChildAboveSibling(child, content_view_);
}

bool ScrollView::InsertChildAboveSibling(Widget* child, Widget* sibling) {
  return content_view_->InsertChildAboveSibling(child, sibling);
}

bool ScrollView::InsertChildBelowSibling(Widget* child, Widget* sibling) {
  return content_view_->InsertChildBelowSibling(child, sibling);
}

void ScrollView::RedrawScroller(const float scroll_view_length,
                                const float content_view_offset,
                                const float content_view_length,
                                const float content_view_padding,
                                const float scroller_beginning_padding,
                                const float scroller_endding_padding,
                                const bool shows_scrollers_on_both_directions,
                                Scroller* scroller) {
  const float kContentViewOffset = content_view_offset - content_view_padding;
  const float kContentViewLength = \
      content_view_length + content_view_padding * 2;

  float content_view_visible_length;
  if (kContentViewLength < scroll_view_length) {
    content_view_visible_length = scroll_view_length;
  } else if (kContentViewOffset > 0) {
    content_view_visible_length = scroll_view_length - kContentViewOffset;
  } else if ((kContentViewOffset + kContentViewLength) < scroll_view_length) {
    content_view_visible_length = kContentViewOffset + kContentViewLength;
  } else {
    content_view_visible_length = scroll_view_length;
  }

  const float kKnobProportion = \
      content_view_visible_length / kContentViewLength \
      * (scroll_view_length - scroller_beginning_padding \
         - scroller_endding_padding) / scroll_view_length;
  const float kKnobPosition = \
      std::max(0.0f,
               -kContentViewOffset / content_view_visible_length
                   * kKnobProportion) \
      + scroller_beginning_padding / scroll_view_length;

  scroller->set_knob_position(kKnobPosition);
  scroller->set_knob_proportion(kKnobProportion);
  scroller->set_shows_scrollers_on_both_directions(
      shows_scrollers_on_both_directions);
}

void ScrollView::RedrawScrollers() {
  const float kScrollViewWidth = this->GetWidth();
  const float kScrollViewHeight = this->GetHeight();
  const float kContentViewWidth = content_view_->GetWidth();
  const float kContentViewHeight = content_view_->GetHeight();

  const bool kShowsHorizontalScroller = \
      !horizontal_scroller_->IsHidden() ||
      (shows_horizontal_scroll_indicator_ &&
       (is_scrolling_ || IsAnimating()) &&
       kContentViewWidth > kScrollViewWidth);
  const bool kShowsVerticalScroller = \
      !vertical_scroller_->IsHidden() ||
      (shows_vertical_scroll_indicator_ &&
       (is_scrolling_ || IsAnimating()) &&
       kContentViewHeight > kScrollViewHeight);
  const bool kShowsScrollersOnBothDirections = kShowsHorizontalScroller &&
                                               kShowsVerticalScroller;

  if (kShowsHorizontalScroller) {
    horizontal_scroller_->SetHidden(false);
    RedrawScroller(kScrollViewWidth, content_view_->GetX(),
                   kContentViewWidth, (kScrollViewWidth - page_width()) / 2,
                   scroll_indicator_insets_.left,
                   scroll_indicator_insets_.right,
                   kShowsScrollersOnBothDirections,
                   horizontal_scroller_);
  }

  if (kShowsVerticalScroller) {
    vertical_scroller_->SetHidden(false);
    RedrawScroller(kScrollViewHeight, content_view_->GetY(),
                   kContentViewHeight, 0,
                   scroll_indicator_insets_.top,
                   scroll_indicator_insets_.bottom,
                   kShowsScrollersOnBothDirections,
                   vertical_scroller_);
  }
}

float ScrollView::ResolveContentViewOrigin(const float expected_origin,
                                           const float scroll_view_length,
                                           const float content_view_length,
                                           const float content_view_padding,
                                           const bool always_bounces) const {
  // Stops immediately at content view's boundary if bounce is disabled. This
  // applies no matter whether the content view is animating or not.
  if (!bounces_ || (content_view_length < scroll_view_length &&
                    !always_bounces)) {
    if (expected_origin > content_view_padding ||
        content_view_length < scroll_view_length) {
      return content_view_padding;
    }
    if ((expected_origin + content_view_length + content_view_padding) < \
        scroll_view_length) {
      return scroll_view_length - content_view_length - content_view_padding;
    }
    return expected_origin;
  }

  // Decreases the offset to move gradually while the scroll view is not fully
  // covered by the content view. It also guarantees at least half of the
  // scroll view is covered by the content view.
  float overflowed_offset;
  if (expected_origin > content_view_padding ||
      content_view_length < scroll_view_length) {
    overflowed_offset = std::abs(content_view_padding - expected_origin);
  } else if ((expected_origin + content_view_length + content_view_padding) <
               scroll_view_length) {
    overflowed_offset = scroll_view_length - content_view_padding - \
                        (expected_origin + content_view_length);
  } else {
    return expected_origin;  // scroll view is fully covered by content view
  }
  const float kProgress = \
      std::min(1.0f, (overflowed_offset / 2) / scroll_view_length);
  const float kMaximumOffset = scroll_view_length * 0.5;
  float resolved_offset = std::sin(kProgress / 2 * M_PI) * kMaximumOffset;
  if (expected_origin > content_view_padding)  // overflowed on the left side
    return resolved_offset + content_view_padding;
  if (content_view_length < scroll_view_length)  // content view is smaller
    return -resolved_offset + content_view_padding;
  // Offset is overflowed on the right side.
  return scroll_view_length - content_view_padding - resolved_offset - \
         content_view_length;
}

bool ScrollView::SendChildToBack(Widget* child) {
  return content_view_->SendChildToBack(child);
}

void ScrollView::SetContentViewOffset(const Point offset) {
  SetContentViewOrigin({-offset.x, -offset.y});
}

void ScrollView::SetContentViewOrigin(const Point& origin) {
  Point resolved_origin = origin;
  if (!IsAnimating()) {
    resolved_origin.x = ResolveContentViewOrigin(
        origin.x, this->GetWidth(), content_view_->GetWidth(),
        (GetWidth() - page_width()) / 2, always_bounce_horizontal_);
    resolved_origin.y = ResolveContentViewOrigin(
        origin.y, this->GetHeight(), content_view_->GetHeight(), 0,
        always_bounce_vertical_);
  }
  if (resolved_origin.x == content_view_->GetX() &&
      resolved_origin.y == content_view_->GetY())
    return;

  content_view_->SetX(resolved_origin.x);
  content_view_->SetY(resolved_origin.y);
  RedrawScrollers();
}

void ScrollView::SetContentViewSize(const float width, const float height) {
  if (width >= 0)
    content_view_->SetWidth(width);
  if (height >= 0)
    content_view_->SetHeight(height);
}

bool ScrollView::ShouldHandleEvent(const Point location) {
  if (!enables_scroll_ || !CollidePoint(location, 0))
    return false;

  // Determines acceptable scroll directions.
  acceptable_scroll_directions_ = static_cast<ScrollDirection>(0);
  if (HorizontalScrollingIsAcceptable()) {
    acceptable_scroll_directions_ = static_cast<ScrollDirection>(
        acceptable_scroll_directions_ | ScrollDirection::kHorizontal);
  }
  if (VerticalScrollingIsAcceptable()) {
    acceptable_scroll_directions_ = static_cast<ScrollDirection>(
        acceptable_scroll_directions_ | ScrollDirection::kVertical);
  }
  if (acceptable_scroll_directions_ == static_cast<ScrollDirection>(0))
    return false;

  event_history_.clear();
  ignores_upcoming_events_ = false;
  locked_scroll_directions_ = static_cast<ScrollDirection>(0);
  is_scrolling_ = false;
  return true;
}

void ScrollView::ShowPage(const int page, const double duration) {
  const float kPageOrigin = GetContentViewOriginForPage(page);
  if (duration <= 0)
    SetContentViewOrigin({kPageOrigin, content_view_->GetY()});
  else
    AnimateContentViewHorizontally(kPageOrigin, duration);
}

void ScrollView::StopAnimation() {
  horizontal_animation_states_.is_animating = false;
  vertical_animation_states_.is_animating = false;
  horizontal_animation_states_.is_bouncing = false;
  vertical_animation_states_.is_bouncing = false;
  if (IsAnimating()) {
    horizontal_scroller_->HideInAnimation();
    vertical_scroller_->HideInAnimation();
    Widget::StopAnimation(true);
  }
}

void ScrollView::StopScrollingGradually() {
  float horizontal_velocity, vertical_velocity;
  GetScrollVelocity(&horizontal_velocity, &vertical_velocity);

  const bool kAcceptsHorizontalScroll = \
      (locked_scroll_directions_ & ScrollView::kHorizontal) != 0;
  const bool kAcceptsVerticalScroll = \
      (locked_scroll_directions_ & ScrollView::kVertical) != 0;
  if (is_scrolling_ && !kAcceptsHorizontalScroll && !kAcceptsVerticalScroll)
    return;

  if (!is_scrolling_ || kAcceptsVerticalScroll) {
    if (std::abs(vertical_velocity) < kScrollVelocityThreshold) {
      BounceContentViewVertically();
    } else {
      const float kDeceleration = vertical_velocity * deceleration_rate_ * -1;
      const double kDuration = (0 - vertical_velocity) / kDeceleration;
      const float kDisplacement = CalculateDisplacement(
          vertical_velocity, kDeceleration, kDuration);
      AnimateContentViewVertically(content_view_->GetY() + kDisplacement,
                                   vertical_velocity, kDeceleration, kDuration);
    }
  }

  if (is_scrolling_ && !kAcceptsHorizontalScroll)
    return;

  const float kDeceleration = horizontal_velocity * deceleration_rate_ * -1;
  const double kHorizontalDuration = (0 - horizontal_velocity) / kDeceleration;
  const float kHorizontalDisplacement = CalculateDisplacement(
      horizontal_velocity, kDeceleration, kHorizontalDuration);

  if (enables_paging_) {
    if (std::abs(horizontal_velocity) < 250)
      return BounceContentViewHorizontally();

    // Determines the expected page to scroll to.
    bool scrolls_to_next_page = always_scroll_to_next_page_;
    const int kInitialPage = GetPage(initial_scroll_content_view_origin_.x);
    int page = kInitialPage;
    if (!scrolls_to_next_page) {
      page = GetPage(content_view_->GetX() + kHorizontalDisplacement);
      scrolls_to_next_page = (page == kInitialPage);
    }
    if (scrolls_to_next_page)
      page = horizontal_velocity < 0 ? page + 1 : page - 1;
    page = std::max(0, page);
    page = std::min(GetMaximumPage(), page);

    // Animates to the expected page only if the page changed.
    if (page != kInitialPage) {
      const int kPageDifference = std::abs(page - kInitialPage);
      const double kDuration = kPageDifference * kAnimationDurationPerPage;
      return AnimateContentViewHorizontally(GetContentViewOriginForPage(page),
                                            kDuration);
    }
  }

  if (std::abs(horizontal_velocity) < kScrollVelocityThreshold) {
    BounceContentViewHorizontally();
  } else {
    const float kNewOriginX = content_view_->GetX() + kHorizontalDisplacement;
    AnimateContentViewHorizontally(kNewOriginX, horizontal_velocity,
                                   kDeceleration, kHorizontalDuration);
  }
}

void ScrollView::UpdateAnimationOriginAndStates(const double timestamp,
                                                AnimationStates* states) {
  if (!states->is_animating)
    return;

  if (states->initial_timestamp < 0) {
    states->initial_timestamp = Clock::GetTimestamp();
    states->elapsed_time = 0;
  } else {
    // Updates elapsed time and the origin for the current timing.
    states->elapsed_time = timestamp - states->initial_timestamp;
  }
  // Updates the origin.
  states->is_animating = (states->elapsed_time < states->duration);
  float origin = !states->is_animating ?
                 states->destination_location :
                 states->initial_location + \
                     CalculateDisplacement(states->initial_velocity,
                                           states->deceleration,
                                           states->elapsed_time);
  if (states == &horizontal_animation_states_)
    content_view_->SetX(origin);
  else if (states == &vertical_animation_states_)
    content_view_->SetY(origin);

  // Finds the timing to bounce the content view. Do nothing if it's already
  // bouncing.
  if (states->is_bouncing)
    return;

  // Bounces back immediately once the duration since reaching the boundary
  // has exceeded `kMaximumReachingBoundaryDuration`.
  if (states->reaches_boundary_timestamp >= 0 &&
      (timestamp - states->reaches_boundary_timestamp) >= \
       kMaximumReachingBoundaryDuration) {
    return BounceContentView(states);
  }

  float min_boundary_origin, max_boundary_origin, view_length;
  bool always_bounces = false;
  if (states == &horizontal_animation_states_) {
    GetContentViewBoundaries(&min_boundary_origin, nullptr,
                             &max_boundary_origin, nullptr);
    view_length = GetWidth();
    always_bounces = always_bounce_horizontal_ ||
                     content_view_->GetWidth() > GetWidth();
  } else if (states == &vertical_animation_states_) {
    GetContentViewBoundaries(nullptr, &min_boundary_origin,
                             nullptr, &max_boundary_origin);
    view_length = GetHeight();
    always_bounces = always_bounce_vertical_ ||
                     content_view_->GetHeight() > GetHeight();
  }
  const float kMaxOverflowPadding = view_length / 3;

  // Stops animation immediately if reaching the content view boundary while
  // bouncing is disabled.
  if (!bounces_ || !always_bounces) {
    bool stops_animation = false;
    if (origin > max_boundary_origin) {
      origin = max_boundary_origin;
      stops_animation = true;
    } else if (origin < min_boundary_origin) {
      origin = min_boundary_origin;
      stops_animation = true;
    }

    if (stops_animation) {
      states->is_animating = false;
      if (states == &horizontal_animation_states_)
        content_view_->SetX(origin);
      else if (states == &vertical_animation_states_)
        content_view_->SetY(origin);
      return;
    }
  }

  // Bounces back immediately if reaching the maximum acceptable overflowed
  // boundary limits.
  if (origin > (max_boundary_origin + kMaxOverflowPadding) ||
      origin < (min_boundary_origin - kMaxOverflowPadding)) {
    BounceContentView(states);
  // Updates the timestamp when first time reaching the content view boundary.
  } else if (states->reaches_boundary_timestamp < 0 &&
             origin != states->initial_location &&
             (origin >= max_boundary_origin ||
              origin <= min_boundary_origin)) {
    states->reaches_boundary_timestamp = timestamp;
  }
}

bool ScrollView::VerticalScrollingIsAcceptable() const {
  return content_view_->GetHeight() > GetHeight() ||
         (bounces_ && always_bounce_vertical_);
}

bool ScrollView::WidgetViewWillRender(NVGcontext* context) {
  // Moves the content view to the expected page if paging is enabled.
  if (enables_paging_ && moves_content_view_to_page_ >= 0) {
    ShowPage(moves_content_view_to_page_, 0);
    moves_content_view_to_page_ = -1;
  }

  if (!horizontal_animation_states_.is_animating &&
      !vertical_animation_states_.is_animating)
    return true;

  const double kCurrentTimestamp = Clock::GetTimestamp();
  UpdateAnimationOriginAndStates(kCurrentTimestamp,
                                 &horizontal_animation_states_);
  UpdateAnimationOriginAndStates(kCurrentTimestamp,
                                 &vertical_animation_states_);
  RedrawScrollers();

  if (!horizontal_animation_states_.is_animating &&
      !vertical_animation_states_.is_animating) {
    StopAnimation();
  }
  return true;
}

void ScrollView::set_bottom_padding(const float padding) {
  if (padding != content_view_->bottom_padding()) {
    content_view_->set_bottom_padding(padding);
  }
}

void ScrollView::set_box_sizing(const BoxSizing box_sizing) {
  if (box_sizing != content_view_->box_sizing()) {
    content_view_->set_box_sizing(box_sizing);
  }
}

void ScrollView::set_left_padding(const float padding) {
  if (padding != content_view_->left_padding()) {
    content_view_->set_left_padding(padding);
  }
}

void ScrollView::set_scroll_indicator_insets(const EdgeInsets insets) {
  if (insets.top != scroll_indicator_insets_.top ||
      insets.left != scroll_indicator_insets_.left ||
      insets.bottom != scroll_indicator_insets_.bottom ||
      insets.right != scroll_indicator_insets_.right) {
    scroll_indicator_insets_ = insets;
    Redraw();
  }
}

void ScrollView::set_page_width(const float page_width) {
  if (page_width != page_width_) {
    moves_content_view_to_page_ = GetCurrentPage();
    page_width_ = page_width;
    Redraw();
  }
}

void ScrollView::set_right_padding(const float padding) {
  if (padding != content_view_->right_padding()) {
    content_view_->set_right_padding(padding);
  }
}

void ScrollView::set_shows_horizontal_scroll_indicator(const bool value) {
  shows_horizontal_scroll_indicator_ = value;
  if (!shows_horizontal_scroll_indicator_)
    horizontal_scroller_->SetHidden(true);
}

void ScrollView::set_shows_vertical_scroll_indicator(const bool value) {
  shows_vertical_scroll_indicator_ = value;
  if (!shows_vertical_scroll_indicator_)
    vertical_scroller_->SetHidden(true);
}

void ScrollView::set_top_padding(const float padding) {
  if (padding != content_view_->top_padding()) {
    content_view_->set_top_padding(padding);
  }
}

}  // namespace moui
