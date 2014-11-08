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

#include "moui/widgets/label.h"

#include <algorithm>
#include <cassert>
#include <string>

#include "moui/nanovg_hook.h"

namespace {

// The maximum number of lines to render on the screen when number_of_lines_ is
// set to 0.
// TODO: Calculates the maximum lines that can fit its parent on demand.
const int kMaximumNumberOfLines = 100;

// The minimum font size guaranteed to render the text.
const float kMinimumFontSize = 1;

// The offset in points to change the vertical position for rendering text box.
// This is a workaround to make the rendered position more as expected.
const float kTextBoxVerticalOffset = 3;

// Default configuration for the label.
std::string default_font_name;
float default_font_size = 12;

}  // namespace

namespace moui {

Label::Label() : Label("") {
}

Label::Label(const std::string& text) : Label(text, "") {
}

Label::Label(const std::string& text, const std::string& font_name)
    : Widget(), adjusts_font_size_to_fit_width_(false),
      adjusts_label_height_to_fit_width_(false), font_name_(font_name),
      font_size_(0), minimum_scale_factor_(0), number_of_lines_(1),
      should_prepare_for_rendering_(true), text_(text),
      text_horizontal_alignment_(Alignment::kLeft),
      text_vertical_alignment_(Alignment::kTop) {
  set_text_color(nvgRGBA(0, 0, 0, 255));
}

Label::~Label() {
}

void Label::ConfigureTextAttributes(NVGcontext* context) {
  int alignment;
  switch (text_horizontal_alignment_) {
    case Alignment::kLeft: alignment = NVG_ALIGN_LEFT; break;
    case Alignment::kCenter: alignment = NVG_ALIGN_CENTER; break;
    case Alignment::kRight: alignment = NVG_ALIGN_RIGHT; break;
    default: assert(false);
  }
  nvgFontSize(context, font_size_to_render_);
  nvgFontFace(context, font_name().c_str());
  nvgFillColor(context, text_color_);
  nvgTextAlign(context, alignment);
}

void Label::Render(NVGcontext* context) {
  ConfigureTextAttributes(context);

  float y;  // the vertical position that will be passed to nvgTextBox()
  float bounds[4];  // bounds of the text
  nvgTextBoxBounds(context, 0, 0, GetWidth(), text_to_render_.c_str(), NULL,
                   bounds);
  switch (text_vertical_alignment_) {
    case Alignment::kTop:
      y = -bounds[1];
      break;
    case Alignment::kMiddle:
      y = (GetHeight() - bounds[3] - bounds[1]) / 2 + kTextBoxVerticalOffset;
      break;
    case Alignment::kBottom:
      y = GetHeight() - bounds[3] + kTextBoxVerticalOffset;
      break;
    default:
      assert(false);
  }
  nvgTextBox(context, 0, y, GetWidth(), text_to_render_.c_str(), NULL);
}

void Label::SetDefaultFontName(const std::string& name) {
  default_font_name = name;
}

void Label::SetDefaultFontSize(const float size) {
  default_font_size = size;
}

// This method begins with determining the actual text and font size to render
// according to `adjusts_font_size_to_fit_width_`, `minimum_scale_factor_` and
// `number_of_lines_`. It also calculates the required height to render the
// desired result, and increases the label's height or adjusts the label's
// position accordingly if `adjusts_label_height_to_fit_width_` is set to true.
void Label::WidgetViewWillRender(NVGcontext* context) {
  if (!should_prepare_for_rendering_)
    return;
  should_prepare_for_rendering_ = false;
  font_size_to_render_ = font_size_ > 0 ? font_size_ : default_font_size;

  const int kExpectedNumberOfLines = number_of_lines_ == 0 ?
                                     kMaximumNumberOfLines : number_of_lines_;
  const char* kExpectedLastCharToRender = text_.c_str() + text_.size();
  const float kLabelWidth = GetWidth();
  const float kLabelHeight = GetHeight();
  const float kMinimumAcceptableFontSize = \
      minimum_scale_factor_ > 0 ?
      font_size_to_render_ * minimum_scale_factor_ : kMinimumFontSize;
  NVGtextRow text_rows[kExpectedNumberOfLines];
  float text_box_height = 0;  // the required height to render text

  while (true) {
    ConfigureTextAttributes(context);
    const int kActualNumberOfLines = nvgTextBreakLines(
        context, text_.c_str(), NULL, kLabelWidth, text_rows,
        kExpectedNumberOfLines);
    text_to_render_.clear();
    text_box_height = 0;
    const char* last_char_to_render;
    for (int i = 0; i < kActualNumberOfLines; ++i) {
      if (i > 0) text_to_render_.append("\n");
      NVGtextRow* row = &text_rows[i];
      text_to_render_.append(std::string(row->start, (row->end - row->start)));
      last_char_to_render = row->end;

      // Updates the height of text box.
      float bounds[4];  // bounds of the row string
      nvgTextBounds(context, 0, 0, row->start, row->end, bounds);
      text_box_height += bounds[3] - bounds[1];

      // Stops populating the text to render if it's known the label's bounding
      // box is not big enough.
      if (adjusts_font_size_to_fit_width_ &&
          font_size_to_render_ > kMinimumAcceptableFontSize &&
          text_box_height > kLabelHeight)
        break;
    }

    if (!adjusts_font_size_to_fit_width_ ||
        font_size_to_render_ == kMinimumAcceptableFontSize ||
        (text_box_height < kLabelHeight &&
         last_char_to_render == kExpectedLastCharToRender))
      break;

    // Reduces the font size and try again.
    font_size_to_render_ = std::max(kMinimumAcceptableFontSize,
                                    font_size_to_render_ - 0.1f);
  }

  // Adjusts label height to fit width.
  if (adjusts_label_height_to_fit_width_ && text_box_height > kLabelHeight) {
    if (text_vertical_alignment_ == Label::Alignment::kMiddle) {
      SetY(Widget::Alignment::kTop, Widget::Unit::kPoint,
           GetY() - (text_box_height - kLabelHeight) / 2);
    } else if (text_vertical_alignment_ == Label::Alignment::kBottom) {
      SetY(Widget::Alignment::kTop, Widget::Unit::kPoint,
           GetY() - (text_box_height - kLabelHeight));
    }
    SetHeight(Widget::Unit::kPoint, text_box_height);
  }
}

void Label::set_adjusts_font_size_to_fit_width(const bool value) {
  if (value != adjusts_font_size_to_fit_width_) {
    adjusts_font_size_to_fit_width_ = value;
    should_prepare_for_rendering_ = true;
    Redraw();
  }
}

void Label::set_adjusts_label_height_to_fit_width(const bool value) {
  if (value != adjusts_label_height_to_fit_width_) {
    adjusts_label_height_to_fit_width_ = value;
    should_prepare_for_rendering_ = true;
    Redraw();
  }
}

std::string Label::font_name() const {
  if (font_name_.empty())
    return default_font_name;
  return font_name_;
}

void Label::set_font_name(const std::string& name) {
  if (font_name_ != name) {
    font_name_ = name;
    should_prepare_for_rendering_ = true;
    Redraw();
  }
}

float Label::font_size() const {
  if (font_size_ < 0)
    return default_font_size;
  return font_size_;
}

void Label::set_font_size(const float size) {
  if (size != font_size_) {
    font_size_ = size;
    should_prepare_for_rendering_ = true;
    Redraw();
  }
}

void Label::set_minimum_scale_factor(const float factor) {
  if (factor != minimum_scale_factor_) {
    minimum_scale_factor_ = factor;
    should_prepare_for_rendering_ = true;
    Redraw();
  }
}

void Label::set_number_of_lines(const int number) {
  if (number != number_of_lines_) {
    number_of_lines_ = number;
    should_prepare_for_rendering_ = true;
    Redraw();
  }
}

void Label::set_text(const std::string& text) {
  if (text.size() != text_.size() || text != text_) {
    text_ = text;
    should_prepare_for_rendering_ = true;
    Redraw();
  }
}

void Label::set_text_color(const NVGcolor text_color) {
  if (!nvgCompareColor(text_color, text_color_)) {
    text_color_ = text_color;
    Redraw();
  }
}

void Label::set_text_horizontal_alignment(const Alignment alignment) {
  if (alignment != text_horizontal_alignment_) {
    text_horizontal_alignment_ = alignment;
    Redraw();
  }
}

void Label::set_text_vertical_alignment(const Alignment alignment) {
  if (alignment != text_vertical_alignment_) {
    text_vertical_alignment_ = alignment;
    Redraw();
  }
}

}  // namespace moui
