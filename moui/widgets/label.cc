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
#include <cfloat>
#include <cmath>
#include <string>

#include "moui/nanovg_hook.h"

namespace {

// The maximum number of lines to render on the screen when `number_of_lines_`
// is set to 0.
// TODO(olliwang): Calculates the maximum lines that can fit its parent on
//                 demand.
const int kMaximumNumberOfLines = 100;

// The minimum font size guaranteed to render the text.
const float kMinimumFontSize = 1;

// Default configuration for the label.
std::string default_font_name;
float default_font_baseline = 0;
float default_font_size = 12;
float default_font_size_scale = 1;

}  // namespace

namespace moui {

Label::Label() : Label("") {
}

Label::Label(const std::string& text) : Label(text, "") {
}

Label::Label(const std::string& text, const std::string& font_name)
    : adjusts_font_size_to_fit_width_(false),
      adjusts_label_height_to_fit_width_(false), font_baseline_(FLT_MIN),
      font_name_(font_name), font_size_(0), font_size_scale_(0),
      line_height_(1), minimum_scale_factor_(0), number_of_lines_(1),
      should_prepare_for_rendering_(true), text_(text),
      text_horizontal_alignment_(Alignment::kLeft),
      text_vertical_alignment_(Alignment::kTop) {
  set_text_color(nvgRGBA(0, 0, 0, 255));
  cached_label_width_.width = -1;
}

Label::~Label() {
}

void Label::ConfigureTextAttributes(NVGcontext* context) {
  int horizontal_alignment;
  switch (text_horizontal_alignment_) {
    case Alignment::kLeft: horizontal_alignment = NVG_ALIGN_LEFT; break;
    case Alignment::kCenter: horizontal_alignment = NVG_ALIGN_CENTER; break;
    case Alignment::kRight: horizontal_alignment = NVG_ALIGN_RIGHT; break;
    default: assert(false);
  }
  nvgFillColor(context, text_color_);
  nvgFontFace(context, font_name().c_str());
  nvgFontSize(context, font_size_to_render_ * font_size_scale());
  nvgTextAlign(context, horizontal_alignment | NVG_ALIGN_TOP);
  nvgTextLineHeight(context, line_height_);
  nvgTextLetterSpacing(context, 0);
}

void Label::Redraw() {
  should_prepare_for_rendering_ = true;
  Widget::Redraw();
}

void Label::Render(NVGcontext* context) {
  if (text_to_render_.empty())
    return;

  ConfigureTextAttributes(context);
  float y;  // the vertical position that will be passed to nvgTextBox()
  float bounds[4];  // bounds of the text
  nvgTextBoxBounds(context, 0, 0, GetWidth(), text_to_render_.c_str(), NULL,
                   bounds);
  switch (text_vertical_alignment_) {
    case Alignment::kTop:
      y = -bounds[1] + font_baseline();
      break;
    case Alignment::kMiddle:
      y = (GetHeight() - (bounds[3] - bounds[1])) / 2 + font_baseline();
      break;
    case Alignment::kBottom:
      y = GetHeight() - bounds[3] + font_baseline();
      break;
    default:
      assert(false);
  }
  nvgTextBox(context, 0, y, GetWidth(), text_to_render_.c_str(), NULL);
}

void Label::SetDefaultFontBaseline(const float font_baseline) {
  default_font_baseline = font_baseline;
}

void Label::SetDefaultFontName(const std::string& name) {
  default_font_name = name;
}

void Label::SetDefaultFontSize(const float font_size) {
  default_font_size = font_size;
}

void Label::SetDefaultFontSizeScale(const float font_size_scale) {
  default_font_size_scale = font_size_scale;
}

void Label::UpdateWidthToFitText(NVGcontext* context) {
  font_size_to_render_ = font_size_ > 0 ? font_size_ : default_font_size;
  if (font_size_to_render_ <= 0 || text_.empty())
    return;

  float label_width;
  if (cached_label_width_.width >= 0 &&
      font_size_to_render_ == cached_label_width_.font_size_to_render &&
      font_name_ == cached_label_width_.font_name &&
      text_to_render_ == text_) {
    label_width = cached_label_width_.width;
  } else {
    ConfigureTextAttributes(context);
    const char* kStart = text_.c_str();
    const char* kEnd = kStart + text_.size();
    float bounds[4];

    // This is a workaround to fix the issue that `nvgTextBounds()` may not
    // return a correct result.
    float result = nvgTextBounds(context, 0, 0, kStart, kEnd, bounds);
    label_width = std::ceil(result + 2);

    // Caches the result.
    cached_label_width_.font_size_to_render = font_size_to_render_;
    cached_label_width_.font_name = font_name_;
    cached_label_width_.text = text_;
    cached_label_width_.width = label_width;
  }

  if (label_width != GetWidth())
    SetWidth(label_width);
}

// This method begins with determining the actual text and font size to render
// according to `adjusts_font_size_to_fit_width_`, `minimum_scale_factor_` and
// `number_of_lines_` properties. It also calculates the required height to
// render the desired result, and increases the label's height or adjusts the
// label's position accordingly if `adjusts_label_height_to_fit_width_` is
// set to `true`.
bool Label::WidgetViewWillRender(NVGcontext* context) {
  const float kLabelWidth = GetWidth();
  const float kLabelHeight = GetHeight();
  if ((!adjusts_label_height_to_fit_width_ && kLabelHeight <= 0) ||
      (adjusts_label_height_to_fit_width_ && kLabelWidth <= 0) ||
      text_.empty()) {
    text_to_render_.clear();
    return true;
  }

  if (!should_prepare_for_rendering_)
    return true;
  should_prepare_for_rendering_ = false;
  font_size_to_render_ = font_size_ > 0 ? font_size_ : default_font_size;

  const int kExpectedNumberOfLines = number_of_lines_ == 0 ?
                                     kMaximumNumberOfLines : number_of_lines_;
  const char* kExpectedLastCharToRender = text_.c_str() + text_.size();
  const float kMinimumAcceptableFontSize = \
      minimum_scale_factor_ > 0 ?
      font_size_to_render_ * minimum_scale_factor_ * font_size_scale() :
      kMinimumFontSize;
  NVGtextRow text_rows[kExpectedNumberOfLines];
  float text_box_height = 0;  // the required height to render text

  while (true) {
    ConfigureTextAttributes(context);
    const int kActualNumberOfLines = nvgTextBreakLines(
        context, text_.c_str(), kExpectedLastCharToRender, kLabelWidth,
        text_rows, kExpectedNumberOfLines);

    text_to_render_.clear();
    text_box_height = 0;
    const char* last_char_to_render = nullptr;
    for (int i = 0; i < kActualNumberOfLines; ++i) {
      if (i > 0) text_to_render_.append("\n");
      NVGtextRow* row = &text_rows[i];
      text_to_render_.append(std::string(row->start, (row->end - row->start)));
      last_char_to_render = row->end;

      // Updates the height of text box.
      float bounds[4];  // bounds of the row string
      nvgTextBounds(context, 0, 0, row->start, row->end, bounds);
      text_box_height += (bounds[3] - bounds[1]) * line_height_;

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
                                    font_size_to_render_ - 1);
    nvgFontSize(context, font_size_to_render_);
  }

  // Adjusts label height to fit width.
  if (adjusts_label_height_to_fit_width_ && text_box_height != kLabelHeight)
    SetHeight(Widget::Unit::kPoint, text_box_height);

  return true;
}

void Label::set_adjusts_font_size_to_fit_width(const bool value) {
  if (value != adjusts_font_size_to_fit_width_) {
    adjusts_font_size_to_fit_width_ = value;
    Redraw();
  }
}

void Label::set_adjusts_label_height_to_fit_width(const bool value) {
  if (value != adjusts_label_height_to_fit_width_) {
    adjusts_label_height_to_fit_width_ = value;
    Redraw();
  }
}

float Label::font_baseline() const {
  if (font_baseline_ == FLT_MIN) {
    return default_font_baseline;
  }
  return font_baseline_;
}

void Label::set_font_baseline(const float font_baseline) {
  if (font_baseline != font_baseline_) {
    font_baseline_ = font_baseline;
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
    Redraw();
  }
}

float Label::font_size() const {
  if (font_size_ <= 0)
    return default_font_size;
  return font_size_;
}

void Label::set_font_size(const float font_size) {
  const int kFontSize = static_cast<int>(font_size);
  if (kFontSize != font_size_) {
    font_size_ = kFontSize;
    Redraw();
  }
}

float Label::font_size_scale() const {
  if (font_size_scale_ <= 0) {
    return default_font_size_scale;
  }
  return font_size_scale_;
}

void Label::set_font_size_scale(const float font_size_scale) {
  if (font_size_scale != font_size_scale_) {
    font_size_scale_ = font_size_scale;
    Redraw();
  }
}

void Label::set_line_height(const float line_height) {
  if (line_height != line_height_) {
    line_height_ = line_height;
    Redraw();
  }
}

void Label::set_minimum_scale_factor(const float factor) {
  if (factor != minimum_scale_factor_) {
    minimum_scale_factor_ = factor;
    Redraw();
  }
}

void Label::set_number_of_lines(const int number) {
  if (number != number_of_lines_) {
    number_of_lines_ = number;
    Redraw();
  }
}

void Label::set_text(const std::string& text) {
  if (text.size() != text_.size() || text != text_) {
    text_ = text;
    Redraw();
  }
}

void Label::set_text_color(const NVGcolor text_color) {
  if (!moui::nvgCompareColor(text_color, text_color_)) {
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
