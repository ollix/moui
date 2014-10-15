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

#include "moui/widgets/widget_view.h"

#include <cassert>
#include <vector>

#include "moui/core/device.h"
#include "moui/core/event.h"
#include "moui/nanovg_hook.h"
#include "moui/opengl_hook.h"
#include "moui/ui/native_view.h"
#include "moui/ui/view.h"
#include "moui/widgets/widget.h"

namespace moui {

WidgetView::WidgetView() : View(), context_(nullptr), event_responder_(nullptr),
                           widget_(new Widget) {
  widget_->set_widget_view(this);
}

WidgetView::~WidgetView() {
  if (context_ != nullptr) {
    nvgDeleteGL(context_);
  }
  delete widget_;
}

void WidgetView::HandleEvent(std::unique_ptr<Event> event) {
  if (event_responder_ == nullptr)
    return;
  // Resets the event responder if the widget view of the current responder
  // widget is not this widget view object. This may happen when the responder
  // widget is deattached from this widget view.
  if (event_responder_->widget_view() != this) {
    event_responder_ = nullptr;
    return;
  }
  // Asks the current responder widget to handle the event.
  event_responder_->HandleEvent(event.get());
}

// If the passed widget is visible on screen. Creates a WidgetItem object for
// the widget and adds it to the `widget_list`. Then repeats this process for
// its child widgets.
void WidgetView::PopulateWidgetList(WidgetList* widget_list, Widget* widget,
                                    WidgetItem* parent_item) {
  if (widget->IsHidden()) return;
  const int kWidgetWidth = widget->GetWidth();
  if (kWidgetWidth <= 0) return;
  const int kWidgetHeight = widget->GetHeight();
  if (kWidgetWidth <= 0) return;

  // Determines the translate origin and the scissor area.
  Point translate_origin = {0.0f, 0.0f};
  Point scissor_origin = {0.0f, 0.0f};
  float scissor_width = kWidgetWidth;
  float scissor_height = kWidgetHeight;
  if (parent_item != nullptr) {
    translate_origin.x = parent_item->translate_origin.x + widget->GetX();
    translate_origin.y = parent_item->translate_origin.y + widget->GetY();
    // Determines the scissor's horizontal position.
    scissor_origin.x = std::max(parent_item->scissor_origin.x,
                                translate_origin.x);
    if (scissor_origin.x >= GetWidth()) return;
    // Determines the scissor's vertical position.
    scissor_origin.y = std::max(parent_item->scissor_origin.y,
                                translate_origin.y);
    if (scissor_origin.y >= GetHeight()) return;
    // Stops if the widget is invisible on the scissor's left or top.
    if ((translate_origin.x + kWidgetWidth - 1) < scissor_origin.x ||
        (translate_origin.y + kWidgetHeight - 1) < scissor_origin.y)
      return;
    // Determines the scissor width.
    const float kParentOriginX = parent_item->scissor_origin.x;
    scissor_width = std::min(
        scissor_width,
        kParentOriginX + parent_item->scissor_width - scissor_origin.x);
    scissor_width = std::min(
        scissor_width,
        scissor_origin.x + kWidgetWidth - kParentOriginX);
    if (scissor_width <= 0 || (scissor_origin.x + scissor_width - 1) < 0)
      return;
    // Determines the scissor height.
    const float kParentOriginY = parent_item->scissor_origin.y;
    scissor_height = std::min(
        scissor_height,
        kParentOriginY + parent_item->scissor_height - scissor_origin.y);
    scissor_height = std::min(
        scissor_height, scissor_origin.y + kWidgetHeight - kParentOriginY);
    if (scissor_height <= 0 || (scissor_origin.y + scissor_height - 1) < 0)
      return;
  }

  // The widget is visible. Adds it to the widget list and checks its children.
  auto item = new WidgetItem{widget, parent_item, translate_origin,
                             scissor_origin, scissor_width, scissor_height};;
  widget_list->push_back(item);
  for (Widget* child : widget->children())
    PopulateWidgetList(widget_list, child, item);
}

void WidgetView::Render() {
  if (context_ == nullptr) {
    context_ = nvgCreateGL(NVG_ANTIALIAS);
    widget_->UpdateContext(context_);
    ContextDidCreate(context_);
  }

  // Determines widgets to render in order and filters invisible onces.
  WidgetList widget_list;
  PopulateWidgetList(&widget_list, widget_, nullptr);

  // Notifies valid widgets that the rendering on screen will occur and
  // provides an opportunity to do offscreen rendering.
  for (WidgetItem* item : widget_list) {
    item->widget->WidgetWillRender(context_);
    item->widget->RenderDefaultFramebuffer(context_);
  }

  // Renders visible widgets on screen.
  const int kWidth = GetWidth();
  const int kHeight = GetHeight();
  const int kScreenScaleFactor = Device::GetScreenScaleFactor();
  glViewport(0, 0, kWidth * kScreenScaleFactor, kHeight * kScreenScaleFactor);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  nvgBeginFrame(context_, kWidth, kHeight, kScreenScaleFactor);
  for (WidgetItem* item : widget_list) {
    nvgSave(context_);
    nvgScissor(context_, item->scissor_origin.x, item->scissor_origin.y,
               item->scissor_width, item->scissor_height);
    nvgTranslate(context_, item->translate_origin.x,
                 item->translate_origin.y);
    item->widget->RenderOnDemand(context_);
    nvgRestore(context_);
  }
  nvgEndFrame(context_);

  // Notifies widgets that the rendering process is done and releases memory.
  for (WidgetItem* item : widget_list) {
    item->widget->WidgetDidRender(context_);
    delete item;
  }
}

void WidgetView::SetBounds(const int x, const int y, const int width,
                           const int height) {
  NativeView::SetBounds(x, y, width, height);
  widget_->SetWidth(Widget::Unit::kPixel, width);
  widget_->SetHeight(Widget::Unit::kPixel, height);
}

// Starts with widget_ to find the event responder recursively.
bool WidgetView::ShouldHandleEvent(const Point location) {
  return ShouldHandleEvent(location, widget_);
}

// Iterates children widgets of the specified widget in reversed order to find
// the event responder recursively.
bool WidgetView::ShouldHandleEvent(const Point location, Widget* widget) {
  for (auto it = widget->children().rbegin();
       it != widget->children().rend(); it++) {
    Widget* child = reinterpret_cast<Widget*>(*it);
    if (ShouldHandleEvent(location, child)) {
      return true;
    } else if (child->ShouldHandleEvent(location)) {
      event_responder_ = child;
      return true;
    }
  }
  return false;
}

}  // namespace moui
