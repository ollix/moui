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

#ifndef MOUI_WIDGETS_LABEL_H_
#define MOUI_WIDGETS_LABEL_H_

#include <string>

#include "moui/base.h"
#include "moui/nanovg_hook.h"
#include "moui/widgets/widget.h"

namespace moui {

// The `Label` widget implements a read-only text view.
class Label : public Widget {
 public:
  enum class Alignment {
    kLeft,  // aligns text along the left edge
    kCenter,  // aligns text equally along both sides of the center line
    kRight,  // aligns text along the right edge
    kTop,  // aligns text along the top edge
    kMiddle,  // aligns text equally along both sides of the middle line
    kBottom,  // aligns text along the bottom edge
  };

  Label();
  explicit Label(const std::string& text);
  Label(const std::string& text, const std::string& font_name);
  ~Label();

  // Inherited from `Widget` class;
  void Redraw() final;

  // Sets the default font name.
  static void SetDefaultFontName(const std::string& name);

  // Sets the default font size.
  static void SetDefaultFontSize(const float size);

  // Updates the label's width to fit the text.
  void UpdateWidthToFitText(NVGcontext* context);

  // Accessors and setters.
  bool adjusts_font_size_to_fit_width() const {
    return adjusts_font_size_to_fit_width_;
  }
  void set_adjusts_font_size_to_fit_width(const bool value);
  bool adjusts_label_height_to_fit_width() const {
    return adjusts_label_height_to_fit_width_;
  }
  void set_adjusts_label_height_to_fit_width(const bool value);
  std::string font_name() const;
  void set_font_name(const std::string& name);
  float font_size() const;
  void set_font_size(const float font_size);
  float font_size_to_render() const { return font_size_to_render_; }
  float minimum_scale_factor() const { return minimum_scale_factor_; }
  void set_minimum_scale_factor(const float factor);
  int number_of_lines() const { return number_of_lines_; }
  void set_number_of_lines(const int number);
  std::string text() const { return text_; }
  void set_text(const std::string& text);
  NVGcolor text_color() const { return text_color_; }
  void set_text_color(const NVGcolor text_color);
  Alignment text_horizontal_alignment() const {
    return text_horizontal_alignment_;
  }
  void set_text_horizontal_alignment(const Alignment alignment);
  Alignment text_vertical_alignment() const { return text_vertical_alignment_; }
  void set_text_vertical_alignment(const Alignment alignment);

 private:
  // Configures text attributes through nanovg APIs.
  void ConfigureTextAttributes(NVGcontext* context);

  // Inherited from `Widget` class.
  void Render(NVGcontext* context) final;

  // Inherited from `Widget` class. The implementation prepares the rendering
  // environemnt according various configurations to render expected results.
  bool WidgetViewWillRender(NVGcontext* context) final;

  // Indicates whether the font size should be reduced in order to fit the text
  // into the label's bounding rectangle. The default value is no.
  bool adjusts_font_size_to_fit_width_;

  // Indicates whether the height of the label should be increased automatically
  // in order to fit the text into the label's bounding rectangle. The vertical
  // position of the label will also be adjusted as well to respect
  // `text_vertical_alignment_`. The default value is `false`.
  bool adjusts_label_height_to_fit_width_;

  // The font name of the font that applies to enitre string of text. Note
  // that the font name must previously registered through `nvgCreateFont()`,
  // or nothing will display on the screen.
  std::string font_name_;

  // The size of the font that applies to entire string of text. The default
  // value is 12. If the value is set to 0, the font size set through
  // `SetDefaultFontSize()` will be used.
  float font_size_;

  // The actual font size for rendering on screen. This value is calculated
  // automatically in `WidgetViewWillRender()` according to various
  // configurations.
  float font_size_to_render_;

  // The minimum scale factor supported for the label's text. The value
  // indicates the smallest multiplier for the current font size that yields
  // an acceptable font size to use when rendering the label's text. If the
  // value is set to 0, the font size to render can be as small as possible.
  // The default value is 0.
  float minimum_scale_factor_;

  // The maximum number of lines to use for rendering text. The default value
  // is 1. To remove any maximum limit, and use as many lines as needed, set
  // the value to 0.
  int number_of_lines_;

  // Indicates whether the label should prepare the environment before actual
  // rendering. If true, the preparation will be done in `WidgetWillRender()`.
  bool should_prepare_for_rendering_;

  // The text displayed by the label.
  std::string text_;

  // The color of the text.
  NVGcolor text_color_;

  // The actual text for rendering on screen. This value is populated
  // automatically in `WidgetWillRender()` according to various configurations.
  std::string text_to_render_;

  // The technique to use for aligning the text horizontally. The value must
  // be one of `Alignment::kLeft`, `Alignment::kCenter`, `Alignment::kRight`.
  // The default value is `Alignment::kLeft`.
  Alignment text_horizontal_alignment_;

  // The technique to use for aligning the text vertically. The value must
  // be one of `Alignment::kTop`, `Alignment::kMiddle`, `Alignment::kBottom`.
  // The default value is `Alignment::kTop`.
  Alignment text_vertical_alignment_;

  DISALLOW_COPY_AND_ASSIGN(Label);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_LABEL_H_
