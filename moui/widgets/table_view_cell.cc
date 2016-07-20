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

#include "moui/widgets/label.h"
#include "moui/widgets/table_view.h"
#include "moui/widgets/widget.h"
#include "moui/widgets/widget_view.h"

namespace {

// The default horizontal padding in points of a cell.
const float kDefaultCellHorizontalPadding = 20;

// The default width and height of the cell's image view.
const float kDefaultImageViewLength = 30;

// The default font fsize of the text label.
const float kDefaultTextLabelFontSize = 18;

}  // namespace

namespace moui {

TableViewCell::TableViewCell(const Style style,
                             const std::string& reuse_identifier)
    : highlighted_(false), selected_(false), style_(style),
      reuse_identifier_(reuse_identifier) {
  content_view_ = new moui::Widget(false);
  AddChild(content_view_);

  text_label_ = new Label();
  content_view_->AddChild(text_label_);

  image_view_ = new Widget();
  image_view_->SetWidth(kDefaultImageViewLength);
  image_view_->SetHeight(kDefaultImageViewLength);
  image_view_->SetHidden(true);
  content_view_->AddChild(image_view_);
}

TableViewCell::TableViewCell(const Style style) : TableViewCell(style, "") {
}

TableViewCell::~TableViewCell() {
  delete content_view_;
  delete image_view_;
  delete text_label_;
}

void TableViewCell::PrepareForReuse() {
}

void TableViewCell::UpdateLayout() {
  // Updates the bounds of `content_view_` widget.
  content_view_->SetX(0);
  content_view_->SetY(0);
  content_view_->SetWidth(GetWidth());
  content_view_->SetHeight(GetHeight());

  // Updates the bounds of the `image_view_` widget.
  float left_offset = kDefaultCellHorizontalPadding;
  if (!image_view_->IsHidden()) {
    image_view_->SetX(left_offset);
    image_view_->SetY(
        (content_view_->GetHeight() - image_view_->GetHeight()) / 2);
    left_offset += (image_view_->GetWidth() + 15);
  }

  // Updates the attributes of the `text_label_` widget.
  text_label_->SetX(left_offset);
  text_label_->SetWidth(GetWidth()
                        - left_offset  // left side
                        - kDefaultCellHorizontalPadding);  // right side
  text_label_->SetHeight(Widget::Unit::kPercent, 100);
  text_label_->set_font_size(kDefaultTextLabelFontSize);
  text_label_->set_is_opaque(false);
  text_label_->set_text_vertical_alignment(Label::Alignment::kMiddle);
}

bool TableViewCell::WidgetViewWillRender(NVGcontext* context) {
  const bool kResult = Widget::WidgetViewWillRender(context);
  UpdateLayout();
  return kResult;
}

void TableViewCell::set_highlighted(const bool highlighted) {
  if (highlighted != highlighted_) {
    highlighted_ = highlighted;
    if (widget_view() != nullptr)
      widget_view()->Redraw();
  }
}

void TableViewCell::set_selected(const bool selected) {
  if (selected != selected_) {
    selected_ = selected;
    if (widget_view() != nullptr)
      widget_view()->Redraw();
  }
}

}  // namespace moui
