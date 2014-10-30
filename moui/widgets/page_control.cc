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

#include "moui/widgets/page_control.h"

#include "moui/nanovg_hook.h"

namespace moui {

PageControl::PageControl() : Widget(), current_page_(0),
                             hides_for_single_page_(false),
                             number_of_pages_(0),
                             page_indicator_dot_diameter_(-1),
                             page_indicator_dot_padding_(4) {
  set_current_page_indicator_color(nvgRGBA(255, 255, 255, 255));
  set_page_indicator_color(nvgRGBA(255, 255, 255, 127.5));
  set_page_indicator_dot_diameter(4);
  set_is_opaque(false);
}

PageControl::~PageControl() {
}

void PageControl::Render(NVGcontext* context) {
  const float kDotRadius = page_indicator_dot_diameter_ / 2.0;
  const float kCenterPageIndex = (number_of_pages_ + 1) / 2.0 - 1;
  const float kControlCenterX = GetWidth() / 2.0;

  for (int page = 0; page < number_of_pages_; ++page) {
    const float kDotOffset = \
        (page - kCenterPageIndex) * \
        (page_indicator_dot_padding_ + page_indicator_dot_diameter_);
    nvgBeginPath(context);
    nvgCircle(context, kControlCenterX + kDotOffset, kDotRadius, kDotRadius);
    if (page == current_page_)
      nvgFillColor(context, current_page_indicator_color_);
    else
      nvgFillColor(context, page_indicator_color_);
    nvgFill(context);
  }
}

void PageControl::set_current_page(const int page) {
  if (page != current_page_) {
    current_page_ = page;
    Redraw();
  }
}

void PageControl::set_current_page_indicator_color(const NVGcolor color) {
  if (!nvgCompareColor(color, current_page_indicator_color_)) {
    current_page_indicator_color_ = color;
    Redraw();
  }
}

void PageControl::set_hides_for_single_page(const bool hides_for_single_page) {
  if (hides_for_single_page != hides_for_single_page_) {
    hides_for_single_page_ = hides_for_single_page;
    Redraw();
  }
}

void PageControl::set_number_of_pages(const int number_of_pages) {
  if (number_of_pages != number_of_pages_) {
    number_of_pages_ = number_of_pages;
    Redraw();
  }
}

void PageControl::set_page_indicator_color(const NVGcolor color) {
  if (!nvgCompareColor(color, page_indicator_color_)) {
    page_indicator_color_ = color;
    Redraw();
  }
}

void PageControl::set_page_indicator_dot_diameter(const int diameter) {
  if (diameter != page_indicator_dot_diameter_) {
    page_indicator_dot_diameter_ = diameter;
    SetHeight(Widget::Unit::kPoint, diameter);
    Redraw();
  }
}

void PageControl::set_page_indicator_dot_padding(const int padding) {
  if (padding != page_indicator_dot_padding_) {
    page_indicator_dot_padding_ = padding;
    Redraw();
  }
}

}  // namespace moui
