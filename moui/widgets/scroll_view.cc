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

// The duration to animate the content view to the next page.
const double kAnimatingNextPageDuration = 0.1;

// The default acceleration measured in points per second to stop the content
// view gradually.
const double kDefaultAnimatingAcceleration = -1000;

// The maximum duration to bounce the content view.
const float kMaximumBounceDuration = 0.2;

// The maximum velocity to scroll the content view.
const double kMaximumScrollVelocity = 1800;

// The minimum duration to bounce the content view.
const float kMinimumBounceDuration = 0.05;

// The maximum duration to animate the content view beyond boundary limits.
const float kReachingBoundaryDuration = 0.05;

// The factor used to calculate the scroll velocity.
const float kScrollVelocityFactor = 0.7;

// The velocity threshold that treats the current sequence of events as scroll.
const double kScrollVelocityThreshold = 100;

// Returns the displacement of the passed variables.
float CalculateDisplacement(const double initial_velocity,
                            const double acceleration,
                            const float traveled_time) {
  const double kAcceleration = initial_velocity >= 0 ? acceleration :
                                                       -acceleration;
  return initial_velocity * traveled_time + \
         0.5 * kAcceleration * std::pow(traveled_time, 2);
}

// Returns the initial velocity of the passed variables.
double CalculateInitialVelocity(const float displacement,
                                const double acceleration,
                                const float duration) {
  const double kAcceleration = displacement >= 0 ? acceleration : -acceleration;
  return displacement / duration - 0.5 * kAcceleration * duration;
}

double ReviseScrollVelocity(const double velocity) {
  const double kResult = std::min(kMaximumScrollVelocity,
                                  std::abs(velocity) * kScrollVelocityFactor);
  return velocity >= 0 ? kResult : -kResult;
}

}  // namespace

namespace moui {

ScrollView::ScrollView()
    : Widget(false), always_bounce_horizontal_(false),
      always_bounce_vertical_(false), always_scroll_to_next_page_(true),
      animating_acceleration_(kDefaultAnimatingAcceleration), bounces_(true),
      content_view_(new Widget(false)), enables_paging_(false),
      enables_scroll_(true), page_width_(0),
      shows_horizontal_scroll_indicator_(true),
      shows_vertical_scroll_indicator_(true) {
  set_responder_chain_identifier("moui::ScrollView");

  // Initializes content view.
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
}

ScrollView::~ScrollView() {
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
                                                const float duration) {
  AnimateContentViewHorizontally(origin_x, animating_acceleration_, duration);
}

void ScrollView::AnimateContentViewHorizontally(const float origin_x,
                                                const double acceleration,
                                                const float duration) {
  if (duration <= 0)
    return;

  const float kDisplacement = origin_x - content_view_->GetX();
  if (kDisplacement == 0)
    return;

  const double kVelocity = CalculateInitialVelocity(kDisplacement,
                                                    acceleration, duration);
  AnimateContentViewHorizontally(origin_x, kVelocity, acceleration, duration);
}

void ScrollView::AnimateContentViewHorizontally(const float origin_x,
                                                const double initial_velocity,
                                                const double acceleration,
                                                const float duration) {
  horizontal_animation_state_.acceleration = acceleration;
  horizontal_animation_state_.is_animating = true;
  horizontal_animation_state_.destination_location = origin_x;
  horizontal_animation_state_.duration = duration;
  horizontal_animation_state_.reaches_boundary_timestamp = -1;
  horizontal_animation_state_.initial_location = content_view_->GetX();
  horizontal_animation_state_.initial_velocity = initial_velocity;
  horizontal_animation_state_.initial_timestamp = Clock::GetTimestamp();
  if (!IsAnimating())
    StartAnimation();
}

void ScrollView::AnimateContentViewOffset(const Point offset,
                                          const float duration) {
  const float kOriginX = -offset.x;
  const float kOriginY = -offset.y;
  AnimateContentViewHorizontally(kOriginX, duration);
  AnimateContentViewVertically(kOriginY, duration);
}

void ScrollView::AnimateContentViewVertically(const float origin_y,
                                              const float duration) {
  AnimateContentViewVertically(origin_y, animating_acceleration_, duration);
}

void ScrollView::AnimateContentViewVertically(const float origin_y,
                                              const double acceleration,
                                              const float duration) {
  if (duration <= 0)
    return;

  const float kDisplacement = origin_y - content_view_->GetY();
  if (kDisplacement == 0)
    return;

  const double kVelocity = CalculateInitialVelocity(kDisplacement, acceleration,
                                                    duration);
  AnimateContentViewVertically(origin_y, kVelocity, acceleration, duration);
}

void ScrollView::AnimateContentViewVertically(const float origin_y,
                                              const double initial_velocity,
                                              const double acceleration,
                                              const float duration) {
  vertical_animation_state_.acceleration = acceleration;
  vertical_animation_state_.is_animating = true;
  vertical_animation_state_.destination_location = origin_y;
  vertical_animation_state_.duration = duration;
  vertical_animation_state_.reaches_boundary_timestamp = -1;
  vertical_animation_state_.initial_location = content_view_->GetY();
  vertical_animation_state_.initial_velocity = initial_velocity;
  vertical_animation_state_.initial_timestamp = Clock::GetTimestamp();
  if (!IsAnimating())
    StartAnimation();
}

void ScrollView::BounceContentViewHorizontally() {
  float minimum_x, minimum_y, maximum_x, maximum_y;
  if (enables_paging_) {
    minimum_x = -GetContentViewOffsetForPage(GetCurrentPage());
    maximum_x = minimum_x;
  } else {
    GetContentViewOriginLimits(&minimum_x, &minimum_y, &maximum_x, &maximum_y);
  }
  const float kCurrentX = content_view_->GetX();
  float x = kCurrentX;
  if (x < minimum_x)
    x = minimum_x;
  else if (x > maximum_x)
    x = maximum_x;
  else
    return;

  const float kDistance = std::abs(x - kCurrentX);
  const float kHalfWidth = GetWidth() / 2;
  float duration = kMaximumBounceDuration;
  if (kDistance < kHalfWidth) {
    duration = std::sin((kDistance / kHalfWidth) * 0.5 * M_PI) * \
               kMaximumBounceDuration * 5;
    duration = std::max(kMinimumBounceDuration, duration);
    duration = std::min(kMaximumBounceDuration, duration);
  }
  horizontal_animation_state_.is_bouncing = true;
  AnimateContentViewHorizontally(x, kDefaultAnimatingAcceleration, duration);
}

void ScrollView::BounceContentViewVertically() {
  float minimum_x, minimum_y, maximum_x, maximum_y;
  GetContentViewOriginLimits(&minimum_x, &minimum_y, &maximum_x, &maximum_y);
  const float kCurrentY = content_view_->GetY();
  float y = kCurrentY;
  if (y < minimum_y)
    y = minimum_y;
  else if (y > maximum_y)
    y = maximum_y;
  else
    return;

  const float kDistance = std::abs(y - kCurrentY);
  const float kHalfHeight = GetHeight() / 2;
  float duration = kMaximumBounceDuration;
  if (kDistance < kHalfHeight) {
    duration = std::sin((kDistance / kHalfHeight) * 0.5 * M_PI) * \
               kMaximumBounceDuration * 5;
    duration = std::max(kMinimumBounceDuration, duration);
    duration = std::min(kMaximumBounceDuration, duration);
  }
  vertical_animation_state_.is_bouncing = true;
  AnimateContentViewVertically(y, kDefaultAnimatingAcceleration, duration);
}

Point ScrollView::GetContentViewOffset() const {
  return {-content_view_->GetX(), -content_view_->GetY()};
}

float ScrollView::GetContentViewOffsetForPage(const int page) const {
  const float kPageWidth = page_width();
  const float kHorizontalPadding = (GetWidth() - kPageWidth) / 2;
  return page * kPageWidth - kHorizontalPadding;
}

void ScrollView::GetContentViewOriginLimits(float* minimum_x,
                                            float* minimum_y,
                                            float* maximum_x,
                                            float* maximum_y) const {
  // Determines horizontal limits.
  if (enables_paging_ && always_scroll_to_next_page_) {
    *minimum_x = -GetContentViewOffsetForPage(GetCurrentPage());
    *maximum_x = *minimum_x;
  } else {
    const float kScrollViewWidth = GetWidth();
    const float kHorizontalPadding = (kScrollViewWidth - page_width()) / 2;
    *minimum_x = std::min(
        0.0f,
        kScrollViewWidth - content_view_->GetWidth() - kHorizontalPadding);
    *maximum_x = kHorizontalPadding;
  }
  // Determines vertical limits.
  *minimum_y = std::min(0.0f, this->GetHeight() - content_view_->GetHeight());
  *maximum_y = 0;
}

Size ScrollView::GetContentViewSize() const {
  return {content_view_->GetWidth(), content_view_->GetHeight()};
}

float ScrollView::GetCurrentAnimatingLocation(AnimationState& state) const {
  return state.elapsed_time >= state.duration ?
         state.destination_location :
         state.initial_location + CalculateDisplacement(state.initial_velocity,
                                                        state.acceleration,
                                                        state.elapsed_time);
}

int ScrollView::GetCurrentPage() const {
  const float kPageWidth = page_width();
  if (content_view_->GetWidth() < kPageWidth)
    return 0;

  const int kPage = (GetWidth() / 2 - content_view_->GetX()) / kPageWidth;
  if (kPage < 0)
    return 0;

  const int kMaximumPageNumber = GetMaximumPage();
  if (kPage > kMaximumPageNumber)
    return kMaximumPageNumber;

  return kPage;
}

int ScrollView::GetMaximumPage() const {
  return content_view_->GetWidth() / page_width() - 1;
}

ScrollView::ScrollDirection ScrollView::GetScrollDirection() const {
  if (event_history_.size() < 2)
    return ScrollDirection::kUnknown;

  // auto event_history_iterator = event_history_.end();
  // ScrollEvent latest_event = *(event_history_iterator - 1);
  // ScrollEvent previous_event = *(event_history_iterator - 2);

  ScrollEvent latest_event = event_history_.back();
  ScrollEvent previous_event = event_history_.front();

  const float kOffsetX = latest_event.location.x - previous_event.location.x;
  const float kOffsetY = latest_event.location.y - previous_event.location.y;
  const float kTheta = std::atan2(kOffsetY, kOffsetX);
  float angle = (kTheta + M_PI / 2) * 180 / M_PI;
  if (angle < 0) angle += 360;
  if (acceptable_scroll_direction_ == ScrollDirection::kBoth) {
    const float kHalfRange = 360 / 8 / 2;
    if (angle < kHalfRange || angle > (360 - kHalfRange) ||
        (angle > (180 - kHalfRange) && angle < (180 + kHalfRange))) {
      return ScrollDirection::kVertical;
    }
    if ((angle > (90 - kHalfRange) && angle < (90 + kHalfRange)) ||
        (angle > (270 - kHalfRange) && angle < (270 + kHalfRange))) {
      return ScrollDirection::kHorizontal;
    }
    return ScrollDirection::kBoth;
  }
  // Acceptable direction is either horizontal or vertical.
  const float kHalfRange = 360 / 4 / 2;
  if (angle < kHalfRange || angle > (270 + kHalfRange) ||
      (angle > (180 - kHalfRange) && angle < (180 + kHalfRange))) {
    return ScrollDirection::kVertical;
  }
  return ScrollDirection::kHorizontal;
}

void ScrollView::GetScrollVelocity(double* horizontal_velocity,
                                   double* vertical_velocity) {
  if (event_history_.size() < 2) {
    if (horizontal_velocity != nullptr) *horizontal_velocity = 0;
    if (vertical_velocity != nullptr) *vertical_velocity = 0;
    return;
  }
  ScrollEvent last_event = event_history_.back();
  ScrollEvent initial_event;
  double elapsed_time = -1;
  for (ScrollEvent& event : event_history_) {
    initial_event = event;
    elapsed_time = last_event.timestamp - initial_event.timestamp;
    if (elapsed_time < 0.1)
      break;
  }
  if (elapsed_time <= 0) {
    if (horizontal_velocity != nullptr) *horizontal_velocity = 0;
    if (vertical_velocity != nullptr) *vertical_velocity = 0;
    return;
  }
  if (horizontal_velocity != nullptr) {
    *horizontal_velocity = \
        (last_event.location.x - initial_event.location.x) / elapsed_time;
    *horizontal_velocity = ReviseScrollVelocity(*horizontal_velocity);
  }
  if (vertical_velocity != nullptr) {
    *vertical_velocity = \
        (last_event.location.y - initial_event.location.y) / elapsed_time;
    *vertical_velocity = ReviseScrollVelocity(*vertical_velocity);
  }
}

bool ScrollView::HandleEvent(Event* event) {
  if (ignores_upcoming_events_)
    return true;

  const double kTimestamp = Clock::GetTimestamp();
  Point current_location = event->locations()->at(0);
  event_history_.push_back({current_location, kTimestamp});

  if (locked_scroll_direction_ == ScrollDirection::kUnknown) {
    locked_scroll_direction_ = GetScrollDirection();
    if (acceptable_scroll_direction_ != ScrollDirection::kBoth &&
        locked_scroll_direction_ != ScrollDirection::kUnknown &&
        locked_scroll_direction_ != acceptable_scroll_direction_)
      ignores_upcoming_events_ = true;
  }

  if (locked_scroll_direction_ == ScrollDirection::kUnknown ||
      event->type() == Event::Type::kDown) {
    StopAnimation();
    initial_scroll_content_view_origin_ = {content_view_->GetX(),
                                           content_view_->GetY()};
    initial_scroll_page_ = GetCurrentPage();
    return true;
  }

  if (!ignores_upcoming_events_ && event->type() == Event::Type::kMove) {
    ScrollEvent first_event = event_history_.front();
    const float kOffsetX = current_location.x - first_event.location.x;
    const float kOffsetY = current_location.y - first_event.location.y;
    const float kOriginX = initial_scroll_content_view_origin_.x + kOffsetX;
    const float kOriginY = initial_scroll_content_view_origin_.y + kOffsetY;
    SetContentViewOrigin({kOriginX, kOriginY});
    return false;
  }

  if (ignores_upcoming_events_ || event->type() == Event::Type::kUp ||
      event->type() == Event::Type::kCancel) {
    // Stops the current scroll gradully or bounces the view if the current
    // action is not recognized as scroll at all.
    if (ignores_upcoming_events_ || event_history_.size() <= 2 ||
        !StopScrollingGradually()) {
      BounceContentViewHorizontally();
      BounceContentViewVertically();
    }
    // Clears the event history.
    event_history_.clear();
    // Hides scrollers if not animating.
    if (!horizontal_animation_state_.is_animating)
      horizontal_scroller_->HideInAnimation();
    if (!vertical_animation_state_.is_animating)
      vertical_scroller_->HideInAnimation();
    return true;
  }
  return ignores_upcoming_events_;
}

bool ScrollView::IsScrolling() const {
  return !event_history_.empty();
}

void ScrollView::ReachesContentViewBoundary(bool* horizontal,
                                            bool* vertical) const {
  if (!IsAnimating())
    return;

  float minimum_x, minimum_y, maximum_x, maximum_y;
  GetContentViewOriginLimits(&minimum_x, &minimum_y, &maximum_x, &maximum_y);

  // Checks the horizontal direction.
  float start_location = horizontal_animation_state_.initial_location;
  float end_location = horizontal_animation_state_.destination_location;
  *horizontal = ((end_location - start_location) > 0 &&  // towards right
                 content_view_->GetX() >= maximum_x) ||
                ((end_location - start_location) < 0 &&  // towards left
                 content_view_->GetX() <= minimum_x);

  // Checks the vertical direction.
  start_location = vertical_animation_state_.initial_location;
  end_location = vertical_animation_state_.destination_location;
  *vertical = ((end_location - start_location) > 0 &&  // towards bottom
               content_view_->GetY() >= maximum_y) ||
              ((end_location - start_location) < 0 &&  // towards up
               content_view_->GetY() <= minimum_y);
}

void ScrollView::RedrawScroller(const float scroll_view_length,
                                const float content_view_offset,
                                const float content_view_length,
                                const float content_view_padding,
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

  const double kKnobProportion = \
      content_view_visible_length / kContentViewLength;
  const double kKnobPosition = \
      -kContentViewOffset / content_view_visible_length * kKnobProportion;

  scroller->set_knob_position(kKnobPosition);
  scroller->set_knob_proportion(kKnobProportion);
  scroller->set_shows_scrollers_on_both_directions(
      shows_scrollers_on_both_directions);
  scroller->Redraw();
}

void ScrollView::RedrawScrollers() {
  const float kScrollViewWidth = this->GetWidth();
  const float kScrollViewHeight = this->GetHeight();
  const float kContentViewWidth = content_view_->GetWidth();
  const float kContentViewHeight = content_view_->GetHeight();

  const bool kIsScrolling = IsScrolling();
  const bool kShowsHorizontalScroller = \
      !horizontal_scroller_->IsHidden() ||
      (shows_horizontal_scroll_indicator_ &&
       (kIsScrolling || horizontal_animation_state_.is_animating) &&
       kContentViewWidth - page_width() > 0);
  const bool kShowsVerticalScroller = \
      !vertical_scroller_->IsHidden() ||
      (shows_vertical_scroll_indicator_ &&
       (kIsScrolling || vertical_animation_state_.is_animating) &&
       kContentViewHeight > kScrollViewHeight);
  const bool kShowsScrollersOnBothDirections = kShowsHorizontalScroller &&
                                               kShowsVerticalScroller;

  if (kShowsHorizontalScroller) {
    RedrawScroller(kScrollViewWidth, content_view_->GetX(),
                   kContentViewWidth, (kScrollViewWidth - page_width()) / 2,
                   kShowsScrollersOnBothDirections,
                   horizontal_scroller_);
    horizontal_scroller_->SetHidden(false);
  }

  if (kShowsVerticalScroller) {
    RedrawScroller(kScrollViewHeight, content_view_->GetY(),
                   kContentViewHeight, 0, kShowsScrollersOnBothDirections,
                   vertical_scroller_);
    vertical_scroller_->SetHidden(false);
  }
}

float ScrollView::ResolveContentViewOrigin(const float expected_origin,
                                           const float scroll_view_length,
                                           const float content_view_length,
                                           const float content_view_padding,
                                           const bool bounces) const {
  // Stops immediately at content view's boundary if bounce is disabled. This
  // applies no matter whether the content view is animating or not.
  if (!bounces_ || (!bounces && content_view_length <= scroll_view_length)) {
    if (expected_origin > 0 || content_view_length < scroll_view_length)
      return 0;
    if ((expected_origin + content_view_length) < scroll_view_length)
      return scroll_view_length - content_view_length;
    return expected_origin;
  }

  // Does nothing if the scroll view is animating.
  if (IsAnimating())
    return expected_origin;

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

void ScrollView::SetContentViewOffset(const Point offset) {
  const float kOffsetX = -offset.x;
  const float kOffsetY = -offset.y;
  if (kOffsetX == content_view_->GetX() && kOffsetY == content_view_->GetY())
    return;

  content_view_->SetX(Widget::Alignment::kLeft, Widget::Unit::kPoint, kOffsetX);
  content_view_->SetY(Widget::Alignment::kTop, Widget::Unit::kPoint, kOffsetY);
  RedrawScrollers();
}

void ScrollView::SetContentViewOrigin(const Point& expected_origin) {
  Point resolved_origin = expected_origin;
  resolved_origin.x = ResolveContentViewOrigin(expected_origin.x,
                                               this->GetWidth(),
                                               content_view_->GetWidth(),
                                               (GetWidth() - page_width()) / 2,
                                               always_bounce_horizontal_);
  resolved_origin.y = ResolveContentViewOrigin(expected_origin.y,
                                               this->GetHeight(),
                                               content_view_->GetHeight(),
                                               0, always_bounce_vertical_);
  SetContentViewOffset({-resolved_origin.x, -resolved_origin.y});
}

void ScrollView::SetContentViewSize(const float width, const float height) {
  if (width == content_view_->GetWidth() &&
      height == content_view_->GetHeight())
    return;

  content_view_->SetWidth(Widget::Unit::kPoint, width);
  content_view_->SetHeight(Widget::Unit::kPoint, height);
  Redraw();
}

bool ScrollView::ShouldHandleEvent(const Point location) {
  if (!enables_scroll_ || !CollidePoint(location, 0))
    return false;

  const bool kEnablesHorizontalScroll = \
      (content_view_->GetWidth() > GetWidth()) ||
      (bounces_ && always_bounce_horizontal_);

  const bool kEnablesVerticalScroll = \
      (content_view_->GetHeight() > GetHeight()) ||
      (bounces_ && always_bounce_vertical_);

  if (kEnablesHorizontalScroll && kEnablesVerticalScroll) {
    acceptable_scroll_direction_ = ScrollDirection::kBoth;
  } else if (kEnablesHorizontalScroll) {
    acceptable_scroll_direction_ = ScrollDirection::kHorizontal;
  } else if (kEnablesVerticalScroll) {
    acceptable_scroll_direction_ = ScrollDirection::kVertical;
  } else {
    return false;
  }
  ignores_upcoming_events_ = false;
  locked_scroll_direction_ = ScrollDirection::kUnknown;
  return true;
}

void ScrollView::ShowPage(const int page, const float duration) {
  const float kPageOffset = GetContentViewOffsetForPage(page);
  if (duration <= 0)
    SetContentViewOffset({kPageOffset, -content_view_->GetY()});
  else
    AnimateContentViewHorizontally(-kPageOffset, duration);
}

void ScrollView::StopAnimation() {
  horizontal_animation_state_.is_animating = false;
  vertical_animation_state_.is_animating = false;
  horizontal_animation_state_.is_bouncing = false;
  vertical_animation_state_.is_bouncing = false;
  Widget::StopAnimation();
}

bool ScrollView::StopScrollingGradually() {
  double horizontal_velocity, vertical_velocity;
  GetScrollVelocity(&horizontal_velocity, &vertical_velocity);

  const bool kAnimatesHorizontally = \
      std::abs(horizontal_velocity) >= kScrollVelocityThreshold;
  const bool kAnimatesVertically = \
      std::abs(vertical_velocity) >= kScrollVelocityThreshold;
  if (!kAnimatesHorizontally && !kAnimatesVertically) {
    return false;
  }

  bool result = false;
  if (kAnimatesVertically &&
      (locked_scroll_direction_ == ScrollDirection::kBoth ||
       locked_scroll_direction_ == ScrollDirection::kVertical)) {
    const double kVerticalDuration = \
        std::abs(vertical_velocity / animating_acceleration_);
    const double kVerticalDisplacement = CalculateDisplacement(
        vertical_velocity, animating_acceleration_, kVerticalDuration);
    AnimateContentViewVertically(content_view_->GetY() + kVerticalDisplacement,
                                 vertical_velocity, animating_acceleration_,
                                 kVerticalDuration);
    result = true;
  }

  if (!kAnimatesHorizontally ||
      (locked_scroll_direction_ != ScrollDirection::kBoth &&
       locked_scroll_direction_ != ScrollDirection::kHorizontal))
    return result;

  if (enables_paging_ && always_scroll_to_next_page_) {
    // Determines the next page.
    const int kNextPage = horizontal_velocity < 0 ? initial_scroll_page_ + 1 :
                                                    initial_scroll_page_ - 1;
    if (kNextPage >= 0 && kNextPage <= GetMaximumPage()) {
      const float kPageWidth = page_width();
      const float kX = kNextPage * -kPageWidth + (GetWidth() - kPageWidth) / 2;
      AnimateContentViewHorizontally(kX, kAnimatingNextPageDuration);
      return true;
    }
  }

  const double kHorizontalDuration = \
      std::abs(horizontal_velocity / animating_acceleration_);
  const double kHorizontalDisplacement = CalculateDisplacement(
      horizontal_velocity, animating_acceleration_, kHorizontalDuration);
  AnimateContentViewHorizontally(
      content_view_->GetX() + kHorizontalDisplacement, horizontal_velocity,
      animating_acceleration_, kHorizontalDuration);
  return true;
}

void ScrollView::UpdateAnimationState(const bool reaches_boundary,
                                      Scroller* scroller,
                                      AnimationState* state) {
  if (!state->is_animating)
    return;

  // Updates the timestamp when first time reaching the boundary limits.
  const double kCurrentTimestamp = state->initial_timestamp + \
                                   state->elapsed_time;
  if (!state->is_bouncing && reaches_boundary &&
      state->reaches_boundary_timestamp < 0) {
    state->reaches_boundary_timestamp = kCurrentTimestamp;
  }

  // Determines whether the view should bounces in the current direction.
  const bool kShouldBounce = \
      scroller == horizontal_scroller_ ?
      always_bounce_horizontal_ || content_view_->GetWidth() - page_width() > 0:
      always_bounce_vertical_ || content_view_->GetHeight() > GetHeight();

  // Stops animating if reaching preset animation duration or reaching content
  // view's boundary without bounce supported.
  if (state->elapsed_time >= state->duration ||
      ((!bounces_ || !kShouldBounce) && reaches_boundary)) {
    state->is_animating = false;
    state->is_bouncing = false;
  }

  // Starts bouncing the content view if meets the maximum accepatable duration.
  const bool kReachesBoundaryDuration = \
      (kCurrentTimestamp - state->reaches_boundary_timestamp) >
      kReachingBoundaryDuration;

  if (!state->is_animating ||
      (!state->is_bouncing && reaches_boundary && kReachesBoundaryDuration)) {
    if (scroller == horizontal_scroller_)
      BounceContentViewHorizontally();
    else if (scroller == vertical_scroller_)
      BounceContentViewVertically();
  }

  // Hides the scroller if the animation is done.
  if (!state->is_animating)
    scroller->HideInAnimation();
}

bool ScrollView::WidgetViewWillRender(NVGcontext* context) {
  // Moves the current page to center if paging is enabled and the scroll view
  // is not responding to events.
  if (enables_paging_ && event_history_.empty()) {
    Point offset = GetContentViewOffset();
    offset.x = GetContentViewOffsetForPage(GetCurrentPage());
    SetContentViewOffset(offset);
  }

  if (!horizontal_animation_state_.is_animating &&
      !vertical_animation_state_.is_animating)
    return true;

  const double kCurrentTimestamp = Clock::GetTimestamp();
  horizontal_animation_state_.elapsed_time = \
      kCurrentTimestamp - horizontal_animation_state_.initial_timestamp;
  vertical_animation_state_.elapsed_time = \
      kCurrentTimestamp - vertical_animation_state_.initial_timestamp;

  // Moves the content view based on the elapsed time.
  Point origin = {content_view_->GetX(), content_view_->GetY()};
  if (horizontal_animation_state_.is_animating)
    origin.x = GetCurrentAnimatingLocation(horizontal_animation_state_);
  if (vertical_animation_state_.is_animating)
    origin.y = GetCurrentAnimatingLocation(vertical_animation_state_);
  SetContentViewOrigin(origin);

  // Updates the animation states.
  bool reaches_horizontal_boundary, reaches_vertical_boundary;
  ReachesContentViewBoundary(&reaches_horizontal_boundary,
                             &reaches_vertical_boundary);
  UpdateAnimationState(reaches_horizontal_boundary, horizontal_scroller_,
                       &horizontal_animation_state_);
  UpdateAnimationState(reaches_vertical_boundary, vertical_scroller_,
                       &vertical_animation_state_);
  if (!horizontal_animation_state_.is_animating &&
      !vertical_animation_state_.is_animating)
    StopAnimation();
  return true;
}

}  // namespace moui
