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

// The duration to bounce the content view.
const double kAnimatingBounceDuration = 0.2;

// The duration to animate the content view to the next page.
const double kAnimatingNextPageDuration = 0.1;

// The default acceleration measured in points per second to animate the
// content view.
const double kDefaultAnimatingAcceleration = -250;

// The maximum initial velocity to scroll the content view.
const double kMaximumScrollInitialVelocity = 1000;

// The maximum duration to animate the content view beyond boundary limits.
const double kReachingBoundaryDuration = 0.05;

// The velocity threshold that treats the current sequence of events as scroll.
const int kScrollVelocityThreshold = 50;

// Returns the displacement of the passed variables.
float CalculateDisplacement(const double initial_velocity,
                            const double acceleration,
                            const double traveled_time) {
  // Calculates the distance traveled and returns the displacement.
  const float kDistance = \
      std::abs(initial_velocity) * traveled_time +
      0.5 * acceleration * std::pow(traveled_time, 2);
  return (initial_velocity >= 0) ? kDistance : -kDistance;
}

// Returns the initial velocity of the passed variables.
double CalculateInitialVelocity(const float displacement,
                                const double acceleration,
                                const double duration) {
  return displacement / duration - 0.5 * acceleration * duration;
}

}  // namespace

namespace moui {

ScrollView::ScrollView() : ScrollView(kDefaultAnimatingAcceleration) {
}

ScrollView::ScrollView(const double animating_acceleration)
    : Widget(false), always_bounce_horizontal_(false),
      always_bounce_vertical_(false),
      animating_acceleration_(animating_acceleration), bounces_(true),
      content_view_(new Widget(false)), enables_paging_(false),
      enables_scroll_(true), shows_horizontal_scroll_indicator_(true),
      shows_vertical_scroll_indicator_(true) {
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
  if (duration <= 0)
    return;

  const float kDisplacement = origin_x - content_view_->GetX();
  const double kVelocity = CalculateInitialVelocity(kDisplacement,
                                                    animating_acceleration_,
                                                    duration);
  AnimateContentViewHorizontally(origin_x, kVelocity, duration);

}

void ScrollView::AnimateContentViewHorizontally(const float origin_x,
                                                const double initial_velocity,
                                                const double duration) {
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

void ScrollView::AnimateContentViewVertically(const float origin_y,
                                              const float duration) {
  if (duration <= 0)
    return;

  const float kDisplacement = origin_y - content_view_->GetY();
  const double kVelocity = CalculateInitialVelocity(kDisplacement,
                                                    animating_acceleration_,
                                                    duration);
  AnimateContentViewVertically(origin_y, kVelocity, duration);
}

void ScrollView::AnimateContentViewVertically(const float origin_y,
                                              const double initial_velocity,
                                              const double duration) {
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
  GetContentViewOriginLimits(&minimum_x, &minimum_y, &maximum_x, &maximum_y);
  float x = content_view_->GetX();
  if (x < minimum_x)
    x = minimum_x;
  else if (x > maximum_x)
    x = maximum_x;
  else
    return;
  horizontal_animation_state_.is_bouncing = true;
  AnimateContentViewHorizontally(x, kAnimatingBounceDuration);
}

void ScrollView::BounceContentViewVertically() {
  float minimum_x, minimum_y, maximum_x, maximum_y;
  GetContentViewOriginLimits(&minimum_x, &minimum_y, &maximum_x, &maximum_y);
  float y = content_view_->GetY();
  if (y < minimum_y)
    y = minimum_y;
  else if (y > maximum_y)
    y = maximum_y;
  else
    return;

  vertical_animation_state_.is_bouncing = true;
  AnimateContentViewVertically(y, kAnimatingBounceDuration);
}

void ScrollView::GetContentViewOriginLimits(float* minimum_x,
                                            float* minimum_y,
                                            float* maximum_x,
                                            float* maximum_y) const {
  if (enables_paging_) {
    *minimum_x = GetCurrentPage() * this->GetWidth() * -1;
    *maximum_x = *minimum_x;
  } else {
    *minimum_x = std::min(0.0f, this->GetWidth() - content_view_->GetWidth());
    *maximum_x = 0;
  }
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
                                                        animating_acceleration_,
                                                        state.elapsed_time);
}

int ScrollView::GetCurrentPage() const {
  const int kPageWidth = this->GetWidth();
  if (content_view_->GetWidth() < kPageWidth)
    return 0;

  const int kPage = (kPageWidth / 2 - content_view_->GetX()) / kPageWidth;
  if (kPage < 0)
    return 0;

  const int kMaximumPageNumber = GetMaximumPage();
  if (kPage > kMaximumPageNumber)
    return kMaximumPageNumber;

  return kPage;
}

int ScrollView::GetMaximumPage() const {
  return content_view_->GetWidth() / this->GetWidth() - 1;
}

void ScrollView::GetScrollVelocity(double* horizontal_velocity,
                                   double* vertical_velocity) {
  if (event_history_.size() < 2) {
    *horizontal_velocity = 0;
    *vertical_velocity = 0;
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
    *horizontal_velocity = 0;
    *vertical_velocity = 0;
  }  else {
    *horizontal_velocity = \
        (last_event.location.x - initial_event.location.x) / elapsed_time;
    *horizontal_velocity = std::min(kMaximumScrollInitialVelocity,
                                    *horizontal_velocity * 0.3);
    *vertical_velocity = \
        (last_event.location.y - initial_event.location.y) / elapsed_time;
    *vertical_velocity = std::min(kMaximumScrollInitialVelocity,
                                  *vertical_velocity * 0.3);
  }
}

bool ScrollView::HandleEvent(Event* event) {
  const double kTimestamp = Clock::GetTimestamp();
  Point current_location = event->locations()->at(0);

  if (event->type() == Event::Type::kDown) {
    StopAnimation();
    event_history_.push_back({current_location, kTimestamp});
    initial_scroll_content_view_origin_ = {content_view_->GetX(),
                                           content_view_->GetY()};
    initial_scroll_page_ = GetCurrentPage();
  } else if (event->type() == Event::Type::kMove) {
    event_history_.push_back({current_location, kTimestamp});
    ScrollEvent first_event = event_history_.front();
    const float kOffsetX = current_location.x - first_event.location.x;
    const float kOffsetY = current_location.y - first_event.location.y;
    const float kOriginX = initial_scroll_content_view_origin_.x + kOffsetX;
    const float kOriginY = initial_scroll_content_view_origin_.y + kOffsetY;
    MoveContentView({kOriginX, kOriginY});
  } else if (event->type() == Event::Type::kUp ||
             event->type() == Event::Type::kCancel) {
    // Stops the current scroll gradully or bounces the view if the current
    // action is not recognized as scroll at all.
    if (!StopScrollingGradually()) {
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
  }
  return false;
}

bool ScrollView::IsScrolling() const {
  return !event_history_.empty();
}

void ScrollView::MoveContentView(const Point& expected_origin) {
  Point resolved_origin = expected_origin;
  resolved_origin.x = ResolveContentViewOrigin(expected_origin.x,
                                               this->GetWidth(),
                                               content_view_->GetWidth(),
                                               always_bounce_horizontal_);
  resolved_origin.y = ResolveContentViewOrigin(expected_origin.y,
                                               this->GetHeight(),
                                               content_view_->GetHeight(),
                                               always_bounce_vertical_);
  SetContentViewOffset(resolved_origin.x, resolved_origin.y);
  RedrawScrollers() ;
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
                                const bool shows_scrollers_on_both_directions,
                                Scroller* scroller) {
  float content_view_visible_length;
  if (content_view_length < scroll_view_length)
    content_view_visible_length = scroll_view_length;
  else if (content_view_offset > 0)
    content_view_visible_length = scroll_view_length - content_view_offset;
  else if ((content_view_offset + content_view_length) < scroll_view_length)
    content_view_visible_length = content_view_offset + content_view_length;
  else
    content_view_visible_length = scroll_view_length;

  const double kKnobPosition = \
      std::max(0.0f, (0 - content_view_offset)) * 1.0 / content_view_length;
  const double kKnobProportion = \
      content_view_visible_length / content_view_length;

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

  const bool kShowsHorizontalScroller = \
      shows_horizontal_scroll_indicator_ &&
      (IsScrolling() || horizontal_animation_state_.is_animating) &&
      kContentViewWidth > kScrollViewWidth;
  const bool kShowsVerticalScroller = \
      shows_vertical_scroll_indicator_ &&
      (IsScrolling() || vertical_animation_state_.is_animating) &&
      kContentViewHeight > kScrollViewHeight;
  const bool kShowsScrollersOnBothDirections = kShowsHorizontalScroller &&
                                               kShowsVerticalScroller;

  if (kShowsHorizontalScroller) {
    RedrawScroller(kScrollViewWidth, content_view_->GetX(),
                   kContentViewWidth, kShowsScrollersOnBothDirections,
                   horizontal_scroller_);
    horizontal_scroller_->SetHidden(false);
  }

  if (kShowsVerticalScroller) {
    RedrawScroller(kScrollViewHeight, content_view_->GetY(),
                   kContentViewHeight, kShowsScrollersOnBothDirections,
                   vertical_scroller_);
    vertical_scroller_->SetHidden(false);
  }
}

float ScrollView::ResolveContentViewOrigin(const float expected_origin,
                                           const float scroll_view_length,
                                           const float content_view_length,
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
  if (expected_origin > 0 || content_view_length < scroll_view_length) {
    overflowed_offset = std::abs(expected_origin);
  } else if ((expected_origin + content_view_length) < scroll_view_length) {
    overflowed_offset = \
        scroll_view_length - (expected_origin + content_view_length);
  } else {
    return expected_origin;  // scroll view is fully covered by content view
  }
  const float kProgress = \
      std::min(1.0f, (overflowed_offset / 2) / scroll_view_length);
  const float kMaximumOffset = scroll_view_length * 0.5;
  float resolved_offset = std::sin(kProgress / 2 * M_PI) * kMaximumOffset;
  if (expected_origin > 0)  // offset is overflowed on the left side
    return resolved_offset;
  if (content_view_length < scroll_view_length)  // content view is smaller
    return -resolved_offset;
  // Offset is overflowed on the right side.
  return scroll_view_length - resolved_offset - content_view_length;
}

void ScrollView::SetContentViewOffset(const float offset_x,
                                      const float offset_y) {
  if (offset_x == content_view_->GetX() && offset_y == content_view_->GetY())
    return;

  content_view_->SetX(Widget::Alignment::kLeft, Widget::Unit::kPoint, offset_x);
  content_view_->SetY(Widget::Alignment::kTop, Widget::Unit::kPoint, offset_y);
  Redraw();
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

  if (bounces_ && (always_bounce_horizontal_ || always_bounce_vertical_))
    return true;

  return content_view_->GetWidth() > GetWidth() ||
         content_view_->GetHeight() > GetHeight();
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

  if (std::abs(horizontal_velocity) < kScrollVelocityThreshold &&
      std::abs(vertical_velocity) < kScrollVelocityThreshold) {
    return false;
  }

  const double kVerticalDuration = \
      std::abs(vertical_velocity / animating_acceleration_);
  const double kVerticalDisplacement = CalculateDisplacement(
      vertical_velocity, animating_acceleration_, kVerticalDuration);
  AnimateContentViewVertically(content_view_->GetY() + kVerticalDisplacement,
                               vertical_velocity, kVerticalDuration);

  if (enables_paging_) {
    // Determines the next page.
    const int kNextPage = horizontal_velocity < 0 ? initial_scroll_page_ + 1 :
                                                    initial_scroll_page_ - 1;
    if (kNextPage >= 0 && kNextPage <= GetMaximumPage()) {
      AnimateContentViewHorizontally(kNextPage * -GetWidth(),
                                     kAnimatingNextPageDuration);
      return true;
    }
  }

  const double kHorizontalDuration = \
      std::abs(horizontal_velocity / animating_acceleration_);
  const double kHorizontalDisplacement = CalculateDisplacement(
      horizontal_velocity, animating_acceleration_, kHorizontalDuration);
  AnimateContentViewHorizontally(
      content_view_->GetX() + kHorizontalDisplacement, horizontal_velocity,
      kHorizontalDuration);
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

  // Determines whether the view should bounces in the current diration.
  const bool kBounces = \
      scroller == horizontal_scroller_ ?
      always_bounce_horizontal_ || content_view_->GetWidth() > GetWidth():
      always_bounce_vertical_ || content_view_->GetHeight() > GetHeight();

  // Stops animating if reaching preset animation duration or reaching content
  // view's boundary without bounce supported.
  if (state->elapsed_time >= state->duration ||
      ((!bounces_ || !kBounces) && reaches_boundary)) {
    state->is_animating = false;
    state->is_bouncing = false;
    scroller->HideInAnimation();
  }

  // Starts bouncing the content view if meets the maximum accepatable duration.
  if (!state->is_bouncing && reaches_boundary &&
      (kCurrentTimestamp - state->reaches_boundary_timestamp) >
          kReachingBoundaryDuration) {
    if (scroller == horizontal_scroller_)
      BounceContentViewHorizontally();
    else if (scroller == vertical_scroller_)
      BounceContentViewVertically();
  }
}

void ScrollView::WidgetWillRender(NVGcontext* context) {
  if (!IsAnimating())
    return;

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
  MoveContentView(origin);

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
}

}  // namespace moui
