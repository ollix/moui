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

void WidgetView::Render() {
  if (context_ == nullptr) {
    context_ = nvgCreateGL(NVG_ANTIALIAS);
    ContextDidCreate(context_);
  }

  // Notifies belonged widgets the rendering will occur.
  RenderWidget(kWidgetRenderMethod::kWidgetWillRender, widget_);

  // Render offscreen stuff before on-screen rendering.
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  RenderWidget(kWidgetRenderMethod::kRenderOffscreen, widget_);
  RenderWidget(kWidgetRenderMethod::kRenderDefaultFramebuffer, widget_);

  // Renders on screen.
  const int kWidth = GetWidth();
  const int kHeight = GetHeight();
  const int kScreenScaleFactor = Device::GetScreenScaleFactor();

  glViewport(0, 0, kWidth * kScreenScaleFactor, kHeight * kScreenScaleFactor);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  nvgBeginFrame(context_, kWidth, kHeight, kScreenScaleFactor);
  RenderWidget(kWidgetRenderMethod::kRenderOnDemand, widget_);
  nvgEndFrame(context_);
}

void WidgetView::RenderWidget(const kWidgetRenderMethod render_method,
                              Widget* widget) {
  if (widget->IsHidden())
    return;
  const int kWidgetWidth = widget->GetWidth();
  if (kWidgetWidth <= 0)
    return;
  const int kWidgetHeight = widget->GetHeight();
  if (kWidgetHeight <= 0)
    return;
  const int kWidgetX = widget->GetX();
  if ((kWidgetX + kWidgetWidth) < 0)
    return;
  const int kWidgetY = widget->GetY();
  if ((kWidgetY + kWidgetHeight) < 0)
    return;

  if (render_method == kWidgetRenderMethod::kWidgetWillRender) {
    widget->WidgetWillRender(context_);
  } else {
    nvgSave(context_);
    nvgScissor(context_, kWidgetX, kWidgetY, kWidgetWidth, kWidgetHeight);
    nvgTranslate(context_, kWidgetX, kWidgetY);
    widget->UpdateContext(context_);
    switch (render_method) {
      case kWidgetRenderMethod::kRenderDefaultFramebuffer:
        widget->RenderDefaultFramebuffer(context_);
        break;
      case kWidgetRenderMethod::kRenderOnDemand:
        widget->RenderOnDemand(context_);
        break;
      case kWidgetRenderMethod::kRenderOffscreen:
        widget->RenderOffscreen(context_);
        break;
      default:
        assert(false);
    }
  }

  // Renders visible children.
  for (Widget* child : widget->children()) {
    if (child->GetX() > kWidgetWidth || child->GetY() > kWidgetHeight)
      continue;
    RenderWidget(render_method, child);
  }

  if (render_method != kWidgetRenderMethod::kWidgetWillRender)
    nvgRestore(context_);
}

void WidgetView::SetBounds(const int x, const int y, const int width,
                           const int height) {
  NativeView::SetBounds(x, y, width, height);
  widget_->SetWidth(Widget::Unit::kPixel, width);
  widget_->SetHeight(Widget::Unit::kPixel, height);
  Redraw();
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
