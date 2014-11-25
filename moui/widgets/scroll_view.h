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

#include <vector>

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace moui {

// Forward declaration.
class Event;
class Scroller;

// The ScrollView class provides support for displaying content that is larger
// than the size of the scroll view itself.
class ScrollView : public Widget {
 public:
  ScrollView();
  explicit ScrollView(const double animating_acceleration);
  ~ScrollView();

  // Adds a child widget to the scroll view.
  void AddChild(Widget* child);

  // Returns the size of the content view.
  Size GetContentViewSize() const;

  // Returns the number of the current page. The page number starts with 0.
  int GetCurrentPage() const;

  // Returns the number of the last page. The page number starts with 0.
  int GetMaximumPage() const;

  // Returns true if the content view is scrolling.
  bool IsScrolling() const;

  // Sets the offset from the content view's origin that correspondes to the
  // scroll view's origin.
  void SetContentViewOffset(const float offset_x, const float offset_y);

  // Sets the size of content view in points.
  void SetContentViewSize(const float width, const float height);

  // Accessors and setters.
  bool always_bounce_horizontal() const { return always_bounce_horizontal_; }
  void set_always_bounce_horizontal(const bool value) {
    always_bounce_horizontal_ = value;
  }
  bool always_bounce_vertical() const { return always_bounce_vertical_; }
  void set_always_bounce_vertical(const bool value) {
    always_bounce_vertical_ = value;
  }
  bool always_scroll_to_next_page() const {
    return always_scroll_to_next_page_;
  }
  void set_always_scroll_to_next_page(const bool value) {
    always_scroll_to_next_page_ = value;
  }
  bool bounces() const { return bounces_; }
  void set_bounces(const bool value) { bounces_ = value; }
  std::vector<Widget*>& children() {
    Widget* content_view = reinterpret_cast<Widget*>(content_view_);
    return content_view->children();
  }
  bool enables_paging() const { return enables_paging_; }
  void set_enables_paging(const bool value) { enables_paging_ = value; }
  bool enables_scroll() const { return enables_scroll_; }
  void set_enables_scroll(const bool value) { enables_scroll_ = value; }
  float page_width() const {
    return !enables_paging_ || page_width_ <= 0 || page_width_ > GetWidth() ?
           GetWidth() : page_width_;
  }
  void set_page_width(const float page_width) { page_width_ = page_width; };
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
  // Inherited from Widget class. Moves the content view and updates animation
  // states accordingly.
  virtual bool WidgetViewWillRender(NVGcontext* context) override;

 private:
  // The animation state for either horizontal or vertical direction.
  struct AnimationState {
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
    double initial_velocity;
    // Indicates whether the animation is ongoing.
    bool is_animating;
    // Indicates whether the animation behaviors as a bouncing effect.
    bool is_bouncing;
    // Records the timestamp when reaching the current content view's boundary.
    double reaches_boundary_timestamp;
  };

  // The record of scroll event.
  struct ScrollEvent {
    // Keeps the location where the event happens.
    Point location;
    // Records the timestamp when the event happens.
    double timestamp;
  };

  // Animates the content view to the passed location horizontally in
  // configured duration. This method calculates the initial velocity to move
  // the view automatically.
  void AnimateContentViewHorizontally(const float origin_x,
                                      const float duration);

  // Animates the content view to the passed location horizontally with
  // configured initial velocity and duration.
  void AnimateContentViewHorizontally(const float origin_x,
                                      const double initial_velocity,
                                      const double duration);

  // Animates the content view to the passed location vertically in
  // configured duration. This method calculates the initial velocity to move
  // the view automatically.
  void AnimateContentViewVertically(const float origin_x, const float duration);

  // Animates the content view to the passed location vertically with
  // configured initial velocity and duration.
  void AnimateContentViewVertically(const float origin_x,
                                    const double initial_velocity,
                                    const double duration);

  // Moves the content view to not reach its horizontal boundaries. If pageing
  // is enabled, it moves the current page to the center of the scroll view.
                                    void BounceContentViewHorizontally();

  // Moves the content view to not reach its vertical boundaries.
  void BounceContentViewVertically();

  // Returns the content view's horizontal offset for the current page.
  float GetContentViewOffsetForCurrentPage() const;

  // Returns the valid range of the content view's origin that guarantees the
  // scroll view is fully covered by the content view.
  void GetContentViewOriginLimits(float* minimum_x, float* minimum_y,
                                  float* maximum_x, float* maximum_y) const;

  // Returns the current location of the animating content view based on the
  // passed animation state.
  float GetCurrentAnimatingLocation(AnimationState& state) const;

  // Gets the current velocity of the scroll.
  void GetScrollVelocity(double* horizontal_velocity,
                         double* vertical_velocity);

  // Inherited from Widget class. Controls the scroll behavior.
  virtual bool HandleEvent(Event* event) override final;

  // Moves the content view based on the expected origin. The actual origin
  // the content view moves to may be changed when reaching the boundary of
  // content view.
  void MoveContentView(const Point& expected_origin);

  // Determines whether the animating content view reaches the boundary
  // on both directions.
  void ReachesContentViewBoundary(bool* horizontal, bool* vertical) const;

  // Redraws the scroller for a specific direction.
  void RedrawScroller(const float scroll_view_length,
                      const float content_view_offset,
                      const float content_view_length,
                      const float content_view_padding,
                      const bool shows_scrollers_on_both_directions,
                      Scroller* scroller);

  // Redraws scrollers on both horizontal and vertical directions.
  void RedrawScrollers();

  // Resolves the passed origin location either in horizontal or vertical
  // direction. This method is created for MoveContentView() to calculate
  // a suitable location for showing the content view in various scenarios.
  float ResolveContentViewOrigin(const float expected_location,
                                 const float scroll_view_length,
                                 const float content_view_length,
                                 const float content_view_padding,
                                 const bool bounces) const;

  // Inherited from Widget class.
  virtual bool ShouldHandleEvent(const Point location) override final;

  // Stops the animation and resets both horizontal and vertical states.
  void StopAnimation();

  // Animates content view to stop gradually. Returns false on failure.
  // This method calculates the initial velocity to animate the content view
  // based on recent scroll events. If paging is enabled, it stops the content
  // view at the next page in a pre-defined duration.
  bool StopScrollingGradually();

  // Updates the passed animation state based on other passed parameters.
  // This method also reacts to those changes properly.
  void UpdateAnimationState(const bool reaches_boundary, Scroller* scroller,
                            AnimationState* state);

  // The acceleration that controls the feel of how the content view moves
  // in animation. This value is measured in points per second and must be
  // a negative number.
  double animating_acceleration_;

  // Indicates whether bouncing always occurs when horizontal scrolling reaches
  // the end of the content. If the value is true, horizontal dragging is
  // allowed even if the content is smaller than the bounds of the scroll view.
  // The default value is false.
  bool always_bounce_horizontal_;

  // Indicates whether bouncing always occurs when vertical scrolling reaches
  // the end of the content. If the value is true, vertical dragging is allowed
  // even if the content is smaller than the bounds of the scroll view. The
  // default value is false.
  bool always_bounce_vertical_;

  // Indicates whether a scroll action should always move the scroll view to
  // the next page. If this value is set to false, the scroll view will stop
  // gradually as usual and then move the current page to the center of which.
  // The default value is true.
  bool always_scroll_to_next_page_;

  // Indicates whether the scroll view bounces past the edge of content back
  // again. If the value is true, the scroll view bounces when it encounters
  // a boundary of the content. If the value is false, scrolling stops
  // immediately at the content boundary without bouncing. The default value
  // is true.
  bool bounces_;

  // The strong reference to the widget that contains scrollable views.
  Widget* content_view_;

  // Indicates whether paging is enabled. If the value is true, the scroll view
  // stops on multiples of the scroll view's bounds when scrolling. The default
  // value is false.
  bool enables_paging_;

  // Indicates whether scrolling is enabled. If the value is false, the scroll
  // view does not respond to coming events. The default value is true.
  bool enables_scroll_;

  // Keeps all event reocrds in the current sequenece of events.
  std::vector<ScrollEvent> event_history_;

  // Keeps the states for animating content view in horizontal direction.
  AnimationState horizontal_animation_state_;

  // The strong reference to the horizontal scroller displayed at the bottom of
  // the scroll view.
  Scroller* horizontal_scroller_;

  // Records the content view's origin when receiving the first scroll event.
  Point initial_scroll_content_view_origin_;

  // Records the current page when received the first scroll event.
  int initial_scroll_page_;

  // Indicates the width of every page. The value should always be greater than
  // 0 and lesser than the scroll view's width, or `page_width()` will return
  // the scroll view's width instead of the actual value.
  float page_width_;

  // Indicates whether the horizontal scroll indicator is visible.
  bool shows_horizontal_scroll_indicator_;

  // Indicates whether the vertical scroll indicator is visible.
  bool shows_vertical_scroll_indicator_;

  // Keeps the states for animating content view in vertical direction.
  AnimationState vertical_animation_state_;

  // The strong reference to the vertical scroller displayed at the right of
  // the scroll view.
  Scroller* vertical_scroller_;

  DISALLOW_COPY_AND_ASSIGN(ScrollView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_SCROLL_VIEW_H_
