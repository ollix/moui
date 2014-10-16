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

#ifndef MOUI_WIDGETS_PAGE_CONTROL_H_
#define MOUI_WIDGETS_PAGE_CONTROL_H_

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace moui {

// PageControl renders page indicator dots at the center of the widget.
// The height of the widget will be adjusted automatically to fit the diameter
// of the page indicator dot.
class PageControl : public Widget {
 public:
  PageControl();
  ~PageControl();

  // Accessors and setters.
  int current_page() const { return current_page_; }
  void set_current_page(const int page) { current_page_ = page; }
  NVGcolor current_page_indicator_color() const {
    return current_page_indicator_color_;
  }
  void set_current_page_indicator_color(const float red, const float green,
                                        const float blue, const float alpha) {
    current_page_indicator_color_ = nvgRGBA(red, green, blue, alpha);
  }
  bool hides_for_single_page() const { return hides_for_single_page_; }
  void set_hides_for_single_page(const bool hides_for_single_page) {
    hides_for_single_page_ = hides_for_single_page;
  }
  int number_of_pages() const { return number_of_pages_; }
  void set_number_of_pages(const int number) { number_of_pages_ = number; }
  NVGcolor page_indicator_color() const { return page_indicator_color_; }
  void set_page_indicator_color(const float red, const float green,
                                const float blue, const float alpha) {
    page_indicator_color_ = nvgRGBA(red, green, blue, alpha);
  }
  int page_indicator_dot_diameter() const {
    return page_indicator_dot_diameter_;
  }
  void set_page_indicator_dot_diameter(const int diameter) {
    page_indicator_dot_diameter_ = diameter;
    SetHeight(Widget::Unit::kPoint, diameter);
  }
  int page_indicator_dot_padding() const { return page_indicator_dot_padding_; }
  void set_page_indicator_dot_padding(const int padding) {
    page_indicator_dot_padding_ = padding;
  }

 private:
  // Inherited from Widget class. Renders the page indicators dots.
  virtual void Render(NVGcontext* context) override final;

  // The current page that causes the corresponded page indicator dot to be
  // shown with a particular color. The default value is 0.
  int current_page_;

  // The color to be used for the current page indicator. The default color
  // is opaque white.
  NVGcolor current_page_indicator_color_;

  // Indicates whether the page control is hidden when there is only one page.
  // The default value is false.
  bool hides_for_single_page_;

  // The number of page indicators to be shown as dots. The default value is 0.
  int number_of_pages_;

  // The color to be used for the page indicator dot. The default colro is
  // translucent white.
  NVGcolor page_indicator_color_;

  // The diameter of page indicator dot in points. The default value is 4.
  int page_indicator_dot_diameter_;

  // The number of points between page indicator dots. The default value is 4.
  int page_indicator_dot_padding_;

  DISALLOW_COPY_AND_ASSIGN(PageControl);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_PAGE_CONTROL_H_
