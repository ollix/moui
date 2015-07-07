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

#ifndef MOUI_WIDGETS_SCROLL_VIEW_H_
#define MOUI_WIDGETS_SCROLL_VIEW_H_

#include <cmath>
#include <vector>

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace moui {

// Forward declaration.
class Event;
class Scroller;

// The `ScrollView` class allows to display content that is larger than the
// size of the scroll view itself.
class ScrollView : public Widget {
 public:
  ScrollView();
  ~ScrollView();

  // Adds a child widget to the scroll view.
  void AddChild(Widget* child);

  // Moves the content view to the specified offset in animation.
  void AnimateContentViewOffset(const Point offset, const double duration);

  // Returns the offset of the content view.
  Point GetContentViewOffset() const;

  // Returns the size of the content view.
  Size GetContentViewSize() const;

  // Returns the number of the current page. The page number starts with 0.
  int GetCurrentPage() const;

  // Sets content view's offset that correspondes to the scroll view's origin.
  void SetContentViewOffset(const Point offset);

  // Sets the size of content view in points. Negative values will be ignored.
  void SetContentViewSize(const float width, const float height);

  // Moves the content view to the specified page. If the specified duration
  // is greater than 0, the content view will be moved in animation.
  void ShowPage(const int page, const double duration);

  // Accessors and setters.
  bool always_bounce_horizontal() const { return always_bounce_horizontal_; }
  void set_always_bounce_horizontal(const bool value) {
    always_bounce_horizontal_ = value;
  }
  bool always_bounce_vertical() const { return always_bounce_vertical_; }
  void set_always_bounce_vertical(const bool value) {
    always_bounce_vertical_ = value;
  }
  bool always_scroll_both_directions() const {
    return always_scroll_both_directions_;
  }
  void set_always_scroll_both_directions(const bool value) {
    always_scroll_both_directions_ = value;
  }
  bool always_scroll_to_next_page() const {
    return always_scroll_to_next_page_;
  }
  void set_always_scroll_to_next_page(const bool value) {
    always_scroll_to_next_page_ = value;
  }
  bool bounces() const { return bounces_; }
  void set_bounces(const bool value) { bounces_ = value; }
  std::vector<Widget*>& children() { return content_view_->children(); }
  float deceleration_rate() const { return deceleration_rate_; }
  void set_deceleration_rate(const float deceleration_rate) {
    deceleration_rate_ = std::abs(deceleration_rate);
  }
  bool enables_paging() const { return enables_paging_; }
  void set_enables_paging(const bool value) { enables_paging_ = value; }
  bool enables_scroll() const { return enables_scroll_; }
  void set_enables_scroll(const bool value) { enables_scroll_ = value; }
  bool is_scrolling() const { return is_scrolling_; }
  float page_width() const {
    return !enables_paging_ || page_width_ <= 0 || page_width_ > GetWidth() ?
           GetWidth() : page_width_;
  }
  void set_page_width(const float page_width);
  bool shows_horizontal_scroll_indicator() const {
    return shows_horizontal_scroll_indicator_;
  }
  void set_shows_horizontal_scroll_indicator(const bool value) {
    shows_horizontal_scroll_indicator_ = value;
  }
  bool shows_vertical_scroll_indicator() const {
    return shows_vertical_scroll_indicator_;
  }
  void set_shows_vertical_scroll_indicator(const bool value) {
    shows_vertical_scroll_indicator_ = value;
  }

 protected:
  // Animates the content view to the passed location horizontally in
  // configured duration.
  void AnimateContentViewHorizontally(const float origin_x,
                                      const double duration);

  // Animates the content view to the passed location vertically in
  // configured duration.
  void AnimateContentViewVertically(const float origin_y,
                                    const double duration);

  // Gets the current velocity of the scroll.
  void GetScrollVelocity(float* horizontal_velocity,
                         float* vertical_velocity);

  // Inherited from `Widget` class. Controls the scroll behavior.
  bool HandleEvent(Event* event) override;

  // Redraws scrollers on both horizontal and vertical directions.
  void RedrawScrollers();

  // Sets the content view's origin based on the expected origin. The actual
  // origin may be changed when reaching the boundary of the content view.
  void SetContentViewOrigin(const Point& origin);

  // Inherited from `Widget` class.
  bool ShouldHandleEvent(const Point location) override;

  // Stops the animation and resets both horizontal and vertical states.
  void StopAnimation();

  // Animates content view to stop gradually.
  void StopScrollingGradually();

  // Inherited from `Widget` class. Animates the content view and updates
  // the animation states accordingly.
  bool WidgetViewWillRender(NVGcontext* context) override;

 private:
  // The directions of the scroll action.
  enum ScrollDirection {
    // The scroll direction is horizontal.
    kHorizontal = 0x01 << 0,
    // The scroll direction is vertical.
    kVertical = 0x01 << 1,
  };

  // The animation states for either horizontal or vertical direction.
  struct AnimationStates {
    // The deceleration for animating the content view.
    float deceleration;
    // The destination location of the content view.
    float destination_location;
    // The animation duration measured in seconds.
    double duration;
    // Records the elapsed time of the current animation.
    double elapsed_time;
    // The initial location of the content view.
    float initial_location;
    // Records the timestamp when starting the animation.
    double initial_timestamp;
    // The initial velocity for animating the content view.
    float initial_velocity;
    // Indicates whether the animation is ongoing.
    bool is_animating;
    // Indicates whether the animation behaviors as a bouncing effect.
    bool is_bouncing;
    // Records the timestamp when reaching the current content view's boundary.
    double reaches_boundary_timestamp;
  };

  // The record of a scroll event.
  struct ScrollEvent {
    // Keeps the location where the event happens.
    Point location;
    // Records the timestamp when the event happens.
    double timestamp;
  };

  // Animates the content view to the passed location horizontally with
  // configured initial velocity, deceleration and duration.
  void AnimateContentViewHorizontally(const float origin_x,
                                      const float initial_velocity,
                                      const float deceleration,
                                      const double duration);

  // Animates the content view to the passed location vertically with
  // configured initial velocity, deceleration, and duration.
  void AnimateContentViewVertically(const float origin_y,
                                    const float initial_velocity,
                                    const float deceleration,
                                    const double duration);

  // Bounces the content view based on the direction of the specified
  // `animation_states`.
  void BounceContentView(AnimationStates* animation_states);

  // Moves the content view to not reach its horizontal boundaries. If pageing
  // is enabled, it moves the current page to the center of the scroll view.
  void BounceContentViewHorizontally();

  // Moves the content view to not reach its vertical boundaries.
  void BounceContentViewVertically();

  // Returns the content view's horizontal origin for the specified page.
  float GetContentViewOriginForPage(const int page) const;

  // Returns the boundaries of content view.
  void GetContentViewBoundaries(float* minimum_x, float* minimum_y,
                                float* maximum_x, float* maximum_y) const;

  // Returns the number of the last page. The page number starts with 0.
  int GetMaximumPage() const;

  // Returns the page of the specified horizontal origin of the content view.
  int GetPage(const float origin_x) const;

  // Gets the current scroll direction based on the
  // `acceptable_scroll_directions_` and the latest `event_history_`.
  // Returns `false` on failure.
  bool GetScrollDirection(ScrollDirection* direction) const;

  // Redraws the scroller for a specific direction.
  void RedrawScroller(const float scroll_view_length,
                      const float content_view_offset,
                      const float content_view_length,
                      const float content_view_padding,
                      const bool shows_scrollers_on_both_directions,
                      Scroller* scroller);

  // Returns the preferred content view location that should be used while
  // moving the content view manually. Specifically, it converts the
  // `expected_location` to a suitable location when the `expected_location`
  // is beyond its boundaries. However, this method is not designed for
  // animation use.
  float ResolveContentViewOrigin(const float expected_location,
                                 const float scroll_view_length,
                                 const float content_view_length,
                                 const float content_view_padding,
                                 const bool always_bounces) const;

  void UpdateAnimationOriginAndStates(const double timestamp,
                                      AnimationStates* states);

  // Indicates the direction that is acceptable for scrolling. This value is
  // determined in the `ShouldHandleEvent()` method.
  ScrollDirection acceptable_scroll_directions_;

  // Indicates whether bouncing always occurs when horizontal scrolling reaches
  // the end of the content. If the value is `true`, horizontal dragging is
  // allowed even if the content is smaller than the bounds of the scroll view.
  // The default value is `false`.
  bool always_bounce_horizontal_;

  // Indicates whether bouncing always occurs when vertical scrolling reaches
  // the end of the content. If the value is true, vertical dragging is allowed
  // even if the content is smaller than the bounds of the scroll view. The
  // default value is `false`.
  bool always_bounce_vertical_;

  // Indicates whether to always allow to scroll to any direction. This value
  // is effective only if both vertical and horizontal scroll directions are
  // accepted. If the value is `false`, the scroll view will lock the
  // acceptable scroll directions to one of the vertical direction, horizontal
  // direction or both depending on the user's scroll direction. The default
  // value is `true`.
  bool always_scroll_both_directions_;

  // Indicates whether scrolling should always stop at next page. This property
  // is effective only if `enables_paging_` is `true`. The default value is
  // `false`. When this property is `false`, the scrolling still stops at a
  // specific page but which page to stop is depending on the horizontal
  // scrolling velocity.
  bool always_scroll_to_next_page_;

  // Indicates whether the scroll view bounces past the edge of content back
  // again. If the value is `true`, the scroll view bounces when it encounters
  // a boundary of the content. If the value is false, scrolling stops
  // immediately at the content boundary without bouncing. The default value
  // is `true`.
  bool bounces_;

  // The strong reference to the widget that contains scrollable views.
  Widget* content_view_;

  // Indicates the deceleration rate that used to slow down the scrolling
  // until the content view is still. The actual deceleration is calculated
  // by multiplying the rate by the desired initial velocity. This value
  // should always be specified as a positive value.
  float deceleration_rate_;

  // Indicates whether paging is enabled. If the value is true, the scroll view
  // stops on multiples of the scroll view's bounds when scrolling. The default
  // value is `false`.
  bool enables_paging_;

  // Indicates whether scrolling is enabled. If the value is false, the scroll
  // view does not respond to coming events. The default value is `true`.
  bool enables_scroll_;

  // Keeps all event reocrds in the current sequenece of events.
  std::vector<ScrollEvent> event_history_;

  // Keeps the states for animating content view in horizontal direction.
  AnimationStates horizontal_animation_states_;

  // The strong reference to the horizontal scroller displayed at the bottom of
  // the scroll view.
  Scroller* horizontal_scroller_;

  // Indicates whether upcoming events should be ignored by `HandleEvent()`.
  // This value is reset to `false` in `ShouldHandleEvent()`.
  bool ignores_upcoming_events_;

  // Records the content view's origin when receiving the first scroll event.
  Point initial_scroll_content_view_origin_;

  // Records the current page when received the first scroll event.
  int initial_scroll_page_;

  // Indicates whether the scroll view is scrolling.
  bool is_scrolling_;

  // Indicates the directions available for scrolling. This value is reset in
  // `ShouldHandleEvent()` and updated in `HandleEvent()`.
  ScrollDirection locked_scroll_directions_;

  // Indicates the width of every page. The value should always be greater than
  // 0 and lesser than the scroll view's width, or `page_width()` will return
  // the scroll view's width instead of the actual value.
  float page_width_;

  // The page to move to when `enables_paging_` is `true`.
  int moves_content_view_to_page_;

  // Indicates whether the horizontal scroll indicator is visible.
  bool shows_horizontal_scroll_indicator_;

  // Indicates whether the vertical scroll indicator is visible.
  bool shows_vertical_scroll_indicator_;

  // Keeps the states for animating content view in vertical direction.
  AnimationStates vertical_animation_states_;

  // The strong reference to the vertical scroller displayed at the right of
  // the scroll view.
  Scroller* vertical_scroller_;

  DISALLOW_COPY_AND_ASSIGN(ScrollView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_SCROLL_VIEW_H_
