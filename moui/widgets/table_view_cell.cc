// Copyright (c) 2016 Ollix. All rights reserved.
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

#include "moui/widgets/table_view_cell.h"

#include <algorithm>

#include "moui/widgets/label.h"
#include "moui/widgets/table_view.h"
#include "moui/widgets/widget.h"
#include "moui/widgets/widget_view.h"

namespace {

// The vector width of the checkmark accessory.
const float kAccessoryCheckmarkVectorWidth = 13;
// The default horizontal padding in points of a cell.
const float kDefaultCellHorizontalPadding = 20;
// The default text color of the detail text label.
const NVGcolor kDefaultDetailTextLabelTextColor = nvgRGB(170, 170, 170);
// The default width and height of the cell's image view.
const float kDefaultImageViewLength = 30;
// The default padding in points between accessory indicator and detail text
// label.
const float kDefaultTextLabelMargin = 10;
// The default font size of the text label.
const float kDefaultTextLabelFontSize = 18;
// The vector width of the disclosure indicator accessory.
const float kAccessoryDisclosureIndicatorVectorWidth = 8;

}  // namespace

namespace moui {

TableViewCell::TableViewCell(const Style style,
                             const std::string& reuse_identifier)
    : accessory_type_(AccessoryType::kNone), detail_text_label_(nullptr),
      highlighted_(false), reuse_identifier_(reuse_identifier),
      selected_(false), style_(style) {
  set_auto_release_children(true);

  // Initializes content view.
  content_view_ = new moui::Widget(true);
  content_view_->BindRenderFunction(&TableViewCell::RenderContentView, this);
  AddChild(content_view_);

  // Initializes text label.
  text_label_ = new Label();
  text_label_->set_font_size(kDefaultTextLabelFontSize);
  text_label_->set_is_opaque(false);
  content_view_->AddChild(text_label_);

  // Initializes image view.
  image_view_ = new Widget(true);
  image_view_->SetWidth(kDefaultImageViewLength);
  image_view_->SetHeight(kDefaultImageViewLength);
  image_view_->SetHidden(true);
  content_view_->AddChild(image_view_);

  // Initializes detail text label.
  if (style == Style::kValue1) {
    detail_text_label_ = new Label();
    detail_text_label_->set_font_size(kDefaultTextLabelFontSize);
    detail_text_label_->set_is_opaque(false);
    detail_text_label_->set_text_color(kDefaultDetailTextLabelTextColor);
    content_view_->AddChild(detail_text_label_);
  }
}

TableViewCell::TableViewCell(const Style style) : TableViewCell(style, "") {
}

TableViewCell::~TableViewCell() {
  if (auto_release_children())
    return;

  moui::Widget::SmartRelease(content_view_);
  moui::Widget::SmartRelease(image_view_);
  moui::Widget::SmartRelease(text_label_);
  moui::Widget::SmartRelease(detail_text_label_);
}

void TableViewCell::PrepareForReuse() {
}

void TableViewCell::RenderContentView(moui::Widget* widget,
                                      NVGcontext* context) {
  if (accessory_type_ == AccessoryType::kCheckmark) {
    const int kVectorHeight = 10;
    nvgTranslate(context,
                 GetWidth() - kDefaultCellHorizontalPadding
                     - kAccessoryCheckmarkVectorWidth,
                 (GetHeight() - kVectorHeight) / 2);
    nvgBeginPath(context);
    nvgMoveTo(context, 11.401, 0.067);
    nvgLineTo(context, 4.346, 7.186);
    nvgLineTo(context, 1.474, 4.313);
    nvgLineTo(context, 0.1, 5.812);
    nvgLineTo(context, 4.346, 9.933);
    nvgLineTo(context, 12.9, 1.441);
    nvgClosePath(context);
    nvgFillColor(context, nvgRGBA(0, 122, 255, 255));
    nvgFill(context);
  } else if (accessory_type_ == AccessoryType::kDisclosureIndicator) {
    const int kVectorHeight = 13;
    nvgTranslate(context,
                 GetWidth() - kDefaultCellHorizontalPadding
                     - kAccessoryDisclosureIndicatorVectorWidth,
                 (GetHeight() - kVectorHeight) / 2);
    nvgBeginPath(context);
    nvgMoveTo(context, 1, 0);
    nvgLineTo(context, 0, 1);
    nvgLineTo(context, 5.75, 6.5);
    nvgLineTo(context, 0, 12);
    nvgLineTo(context, 1, 13);
    nvgLineTo(context, 8, 6.5);
    nvgClosePath(context);
    nvgFillColor(context, nvgRGB(200, 200, 200));
    nvgFill(context);
  }
}

void TableViewCell::UpdateLayout(NVGcontext* context) {
  // Updates the bounds of `content_view_` widget.
  content_view_->SetX(0);
  content_view_->SetY(0);
  content_view_->SetWidth(GetWidth());
  content_view_->SetHeight(GetHeight());

  // Updates the bounds of the `image_view_` widget.
  float left_offset = kDefaultCellHorizontalPadding + left_padding();
  if (!image_view_->IsHidden()) {
    image_view_->SetX(left_offset);
    image_view_->SetY(
        (content_view_->GetHeight() - image_view_->GetHeight()) / 2);
    left_offset += (image_view_->GetWidth() + 15);
  }

  // Determines the widht of accessory view.
  float accessory_view_width = 0;
  if (accessory_type_ == AccessoryType::kCheckmark) {
    accessory_view_width = kAccessoryCheckmarkVectorWidth;
  } else if (accessory_type_ == AccessoryType::kDisclosureIndicator) {
    accessory_view_width = kAccessoryDisclosureIndicatorVectorWidth;
  }
  if (accessory_view_width > 0)
    accessory_view_width += kDefaultTextLabelMargin;

  // Updates the attributes of the `text_label_` widget.
  const float kMaximumLabelWidth = \
      GetWidth()
      - left_offset  // left side
      - kDefaultCellHorizontalPadding  // right side
      - accessory_view_width
      - right_padding();
  text_label_->set_adjusts_font_size_to_fit_width(false);
  text_label_->UpdateWidthToFitText(context);
  text_label_->set_adjusts_font_size_to_fit_width(true);
  const float kLabelWidth = std::min(kMaximumLabelWidth,
                                     text_label_->GetWidth());
  text_label_->SetX(left_offset);
  text_label_->SetHeight(Widget::Unit::kPercent, 100);
  text_label_->SetWidth(kLabelWidth);
  text_label_->set_text_vertical_alignment(Label::Alignment::kMiddle);

  // Updates the attributes of the `detail_text_label_` widget.
  if (detail_text_label_ != nullptr) {
    float offset = kDefaultCellHorizontalPadding + accessory_view_width;
    detail_text_label_->UpdateWidthToFitText(context);
    detail_text_label_->SetX(Widget::Alignment::kRight, Widget::Unit::kPoint,
                             offset);
    detail_text_label_->SetHeight(Widget::Unit::kPercent, 100);
    detail_text_label_->set_adjusts_font_size_to_fit_width(true);
    detail_text_label_->set_text_horizontal_alignment(Label::Alignment::kRight);
    detail_text_label_->set_text_vertical_alignment(Label::Alignment::kMiddle);
    const float kRevisedWidth = \
        detail_text_label_->GetX() + detail_text_label_->GetWidth()
        - (text_label_->GetX() + kLabelWidth + kDefaultTextLabelMargin);
    detail_text_label_->SetWidth(kRevisedWidth);
  }
}

bool TableViewCell::WidgetViewWillRender(NVGcontext* context) {
  const bool kResult = Widget::WidgetViewWillRender(context);
  UpdateLayout(context);
  return kResult;
}

void TableViewCell::set_accessory_type(const AccessoryType accessory_type) {
  if (accessory_type != accessory_type_) {
    accessory_type_ = accessory_type;
    content_view_->Redraw();
  }
}

void TableViewCell::set_highlighted(const bool highlighted) {
  if (highlighted != highlighted_) {
    highlighted_ = highlighted;
    content_view_->Redraw();
  }
}

void TableViewCell::set_selected(const bool selected) {
  if (selected != selected_) {
    selected_ = selected;
    content_view_->Redraw();
  }
}

}  // namespace moui
