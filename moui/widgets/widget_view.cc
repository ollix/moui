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

#include <algorithm>
#include <stack>
#include <string>
#include <vector>

#include "moui/core/device.h"
#include "moui/core/event.h"
#include "moui/defines.h"
#include "moui/native/native_view.h"
#include "moui/nanovg_hook.h"
#include "moui/ui/view.h"
#include "moui/widgets/scroll_view.h"
#include "moui/widgets/widget.h"

namespace moui {

WidgetView::WidgetView(const int context_flags)
    : context_(nullptr), context_flags_(context_flags), is_ready_(false),
      preparing_for_rendering_(false), requests_redraw_(false),
      root_widget_(new Widget) {
  root_widget_->set_widget_view(this);
}

WidgetView::WidgetView() : WidgetView(nvgContextFlags(true, true, 3)) {
}

WidgetView::~WidgetView() {
  moui::Widget::SmartRelease(root_widget_);
  if (context_ != nullptr)
    nvgDeleteContext(context_);
}

void WidgetView::HandleEvent(Event* event) {
  const bool kEventTypeIsUpOrCancel = event->type() == Event::Type::kUp ||
                                      event->type() == Event::Type::kCancel;

  bool scroll_view_horizontal_scrolling_is_acceptable = true;
  bool scroll_view_vertical_scrolling_is_acceptable = true;

  for (Widget* responder : event_responders_) {
    // Determines if the responder is a instance of the `ScrollView` class.
    const bool kResponderIsScrollView = \
        dynamic_cast<ScrollView*>(responder) != nullptr;
    // Skips the current scroll view responder if its acceptable scrolling
    // directions are already handled by another scroll view responder.
    if (kResponderIsScrollView) {
      auto scroll_view = reinterpret_cast<ScrollView*>(responder);
      if ((!scroll_view_horizontal_scrolling_is_acceptable &&
           scroll_view->HorizontalScrollingIsAcceptable()) ||
          (!scroll_view_vertical_scrolling_is_acceptable &&
           scroll_view->VerticalScrollingIsAcceptable())) {
        continue;
      }
    }

    // Updates `effective_event_responders_` for the current responder.
    auto match = std::find(effective_event_responders_.begin(),
                           effective_event_responders_.end(), responder);
    const bool kIsEffective = match != effective_event_responders_.end();
    if (kEventTypeIsUpOrCancel) {
      if (kIsEffective)
        effective_event_responders_.erase(match);
    } else if (!kIsEffective) {
      effective_event_responders_.push_back(responder);
    }

    // Asks the `responder` to handle the current event and stops propagates
    // event if returned `false`.
    const bool kPropagatesEvent = responder->HandleEvent(event);
    if (!kPropagatesEvent) {
      break;
    }

    // Marks the scrolling directions of the current scroll view responder
    // no longer acceptable.
    if (kResponderIsScrollView) {
      auto scroll_view = reinterpret_cast<ScrollView*>(responder);
      if (scroll_view->HorizontalScrollingIsAcceptable())
        scroll_view_horizontal_scrolling_is_acceptable = false;
      if (scroll_view->VerticalScrollingIsAcceptable())
        scroll_view_vertical_scrolling_is_acceptable = false;
    }
  }

  // Sends a `cancel` event to all effective responders that does not handle
  // the `up` or `cancel` event.
  if (!kEventTypeIsUpOrCancel || effective_event_responders_.empty()) {
    return;
  }
  // Creates a `cancel` event if the received one is not.
  Event* cancel_event;
  bool creates_cancel_event = false;
  if (event->type() == Event::Type::kCancel) {
    cancel_event = event;
  } else {
    creates_cancel_event = true;
    cancel_event = new Event(Event::Type::kCancel);
    for (Point location : *(event->locations()))
      cancel_event->locations()->push_back(location);
  }
  for (Widget* responder : effective_event_responders_) {
    responder->HandleEvent(cancel_event);
  }
  if (creates_cancel_event)
    delete cancel_event;
  effective_event_responders_.clear();
}

void WidgetView::HandleMemoryWarning() {
  HandleMemoryWarningRecursively(root_widget_);
}

void WidgetView::HandleMemoryWarningRecursively(moui::Widget* widget) {
  widget->HandleMemoryWarning(context_);
  for (Widget* child_widget : *(widget->children()))
    HandleMemoryWarningRecursively(child_widget);
}

void WidgetView::OnSurfaceDestroyed() {
  if (context_ == nullptr)
    return;

  SetWidgetContextRecursively(root_widget_, context_, nullptr);
  nvgDeleteContext(context_);
  context_ = nullptr;
}

void WidgetView::PopAndFinalizeWidgetItems(const int level,
                                           WidgetItemStack* stack) {
  while (!stack->empty()) {
    WidgetItem* top_item = stack->top();
    if (top_item->level < level)
      break;

    stack->pop();
    top_item->widget->WidgetDidRender(context_);
    reusable_widget_items_.push(top_item);
    nvgRestore(context_);
  }
}

// If the passed widget is visible on screen. Creates a WidgetItem object for
// the widget and adds it to the `widget_list`. Then repeats this process for
// its child widgets.
void WidgetView::PopulateWidgetList(const int level, const float scale,
                                    WidgetList* widget_list, Widget* widget,
                                    WidgetItem* parent_item) {
  static float widget_view_width;
  static float widget_view_height;
  if (level == 0) {
    widget_view_width = GetWidth();
    widget_view_height = GetHeight();
  } else if (widget->IsHidden()) {
    return SetWidgetAndDescendantsInvisible(widget);
  }

  WidgetItem* item;
  if (reusable_widget_items_.empty()) {
    item = new WidgetItem;
    reusable_widget_items_.push(item);
  } else {
    item = reusable_widget_items_.front();
  }

  item->width = widget->GetWidth();
  if (item->width <= 0) return;
  item->height = widget->GetHeight();
  if (item->height <= 0) return;
  item->origin.x = level == 0 ? 0 : widget->GetX();
  item->origin.y = level == 0 ? 0 : widget->GetY();
  const float kScaledWidgetWidth = item->width * scale * widget->scale();
  const float kScaledWidgetHeight = item->height * scale * widget->scale();

  // Determines the translate origin and the scissor area.
  item->translated_origin = {0.0f, 0.0f};
  item->scissor_origin = {0.0f, 0.0f};
  item->scissor_width = kScaledWidgetWidth;
  item->scissor_height = kScaledWidgetHeight;
  if (parent_item == nullptr) {
    item->alpha = widget->alpha();
  } else {
    item->translated_origin.x = parent_item->translated_origin.x
                                + item->origin.x * scale;
    item->translated_origin.y = parent_item->translated_origin.y
                                + item->origin.y * scale;
    // Determines the scissor's horizontal position.
    item->scissor_origin.x = std::max(parent_item->scissor_origin.x,
                                      item->translated_origin.x);
    if (item->scissor_origin.x >= widget_view_width)
      return SetWidgetAndDescendantsInvisible(widget);
    // Determines the scissor's vertical position.
    item->scissor_origin.y = std::max(parent_item->scissor_origin.y,
                                      item->translated_origin.y);
    if (item->scissor_origin.y >= widget_view_height)
      return SetWidgetAndDescendantsInvisible(widget);
    // Stops if the widget is invisible on the scissor's left or top.
    if ((item->translated_origin.x + kScaledWidgetWidth - 1)
            < item->scissor_origin.x ||
        (item->translated_origin.y + kScaledWidgetHeight - 1)
            < item->scissor_origin.y)
      return SetWidgetAndDescendantsInvisible(widget);
    // Determines the scissor width.
    const float kParentOriginX = parent_item->scissor_origin.x;
    item->scissor_width = std::min(
        item->scissor_width,
        kParentOriginX + parent_item->scissor_width - item->scissor_origin.x);
    item->scissor_width = std::min(
        item->scissor_width,
        item->scissor_origin.x + kScaledWidgetWidth - kParentOriginX);
    if (item->scissor_width <= 0 ||
        (item->scissor_origin.x + item->scissor_width - 1) < 0)
      return SetWidgetAndDescendantsInvisible(widget);
    // Determines the scissor height.
    const float kParentOriginY = parent_item->scissor_origin.y;
    item->scissor_height = std::min(
        item->scissor_height,
        kParentOriginY + parent_item->scissor_height - item->scissor_origin.y);
    item->scissor_height = std::min(
        item->scissor_height,
        item->scissor_origin.y + kScaledWidgetHeight - kParentOriginY);
    if (item->scissor_height <= 0 ||
        (item->scissor_origin.y + item->scissor_height - 1) < 0)
      return SetWidgetAndDescendantsInvisible(widget);
    // Determines the alpha value.
    item->alpha = widget->alpha() * parent_item->alpha;
  }

  // The widget is visible. Adds it to the widget list and checks its children.
  visible_widgets_.push_back(widget);
  widget->set_is_visible(true);
  item->widget = widget;
  item->level = level;
  item->parent_item = parent_item;
  reusable_widget_items_.pop();
  widget_list->push_back(item);
  for (Widget* child : *(widget->children())) {
    if (child->IsHidden()) {
      SetWidgetAndDescendantsInvisible(child);
    } else {
      PopulateWidgetList(level + 1, scale * widget->scale(), widget_list,
                         child, item);
    }
  }
}

void WidgetView::Redraw() {
  if (!IsAnimating() && preparing_for_rendering_) {
    requests_redraw_ = true;
  } else {
    View::Redraw();
  }
}

void WidgetView::Redraw(Widget* widget) {
  if (!widget->IsHidden() &&
      std::find(visible_widgets_.begin(), visible_widgets_.end(), widget) != \
      visible_widgets_.end()) {
    Redraw();
  }
}

void WidgetView::RemoveResponder(Widget* widget) {
  for (auto iterator = event_responders_.begin();
       iterator != event_responders_.end();
       ++iterator) {
    if (*iterator == widget) {
      event_responders_.erase(iterator);
      break;
    }
  }
  for (auto iterator = effective_event_responders_.begin();
       iterator != effective_event_responders_.end();
       ++iterator) {
    if (*iterator == widget) {
      effective_event_responders_.erase(iterator);
      break;
    }
  }
}

void WidgetView::Render() {
  Render(root_widget_, nullptr);
}

void WidgetView::Render(Widget* widget, NVGframebuffer* framebuffer) {
  preparing_for_rendering_ = true;
  NVGcontext* context = this->context();
  visible_widgets_.clear();

  // Determines widgets to render in order and filters invisible onces.
  requests_redraw_ = true;
  int count = 0;
  while (requests_redraw_) {
    if (++count == 1000) {
#ifdef DEBUG
      printf("!! WidgetView::Render: Too many redraws.\n");
#endif
      break;
    }
    requests_redraw_ = false;
    WidgetViewWillRender(widget);
  }
  preparing_for_rendering_ = false;
  std::vector<WidgetItem*> widget_list;
  PopulateWidgetList(0, widget->GetMeasuredScale(), &widget_list, widget,
                     nullptr);

  // Renders offscreen stuff here so it won't interfere the onscreen rendering.
  if (framebuffer != nullptr)
    nvgBindFramebuffer(NULL);
  for (WidgetItem* item : widget_list) {
    item->widget->RenderFramebuffer(context);
    item->widget->RenderDefaultFramebuffer(context);
  }
  if (framebuffer != nullptr)
    nvgBindFramebuffer(framebuffer);

  // Clears the render buffer.
  const float kWidth = widget->GetWidth();
  const float kHeight = widget->GetHeight();
  const float kScreenScaleFactor = \
      Device::GetScreenScaleFactor() * widget->GetMeasuredScale();
#ifdef MOUI_GL
  glViewport(0, 0, kWidth * kScreenScaleFactor, kHeight * kScreenScaleFactor);
#endif  // MOUI_GL
  if (!BackgroundIsOpaque()) {
    moui::nvgClearColor(context,
                        kWidth * kScreenScaleFactor,
                        kHeight * kScreenScaleFactor,
                        nvgRGBAf(0, 0, 0, 0));
  }

  // Renders visible widgets on screen.
  nvgBeginFrame(context, kWidth , kHeight, kScreenScaleFactor);
  WidgetItemStack rendering_stack;
  for (WidgetItem* item : widget_list) {
    PopAndFinalizeWidgetItems(item->level, &rendering_stack);
    rendering_stack.push(item);
    nvgSave(context);
    nvgGlobalAlpha(context, item->alpha);
    nvgTranslate(context, item->origin.x, item->origin.y);
    nvgScale(context, item->widget->scale(), item->widget->scale());
    nvgIntersectScissor(context, 0, 0, item->width, item->height);
    item->widget->WidgetWillRender(context);
    nvgSave(context);
    item->widget->RenderOnDemand(context);
    nvgRestore(context);
  }
  PopAndFinalizeWidgetItems(0, &rendering_stack);
  nvgEndFrame(context);

  // Notifies all attached widgets that the rendering process is done.
  WidgetViewDidRender(widget);
  is_ready_ = true;
}

void WidgetView::SetBounds(const float x, const float y, const float width,
                           const float height) {
  NativeView::SetBounds(x, y, width, height);
  root_widget_->SetBounds(0, 0, width, height);
}

void WidgetView::SetWidgetAndDescendantsInvisible(Widget* widget) {
  widget->set_is_visible(false);
  for (Widget* child : *widget->children())
    SetWidgetAndDescendantsInvisible(child);
}

void WidgetView::SetWidgetContextRecursively(Widget* widget,
                                             NVGcontext* oldContext,
                                             NVGcontext* newContext) {
  widget->NotifyContextChange(oldContext, newContext);
  for (Widget* child_widget : *widget->children())
    SetWidgetContextRecursively(child_widget, oldContext, newContext);
}

bool WidgetView::ShouldHandleEvent(const Point location) {
  UpdateEventResponders(location, nullptr);
  return !event_responders_.empty();
}

// Iterates children widgets of the specified widget in reversed order to find
// the event responder recursively.
bool WidgetView::UpdateEventResponders(const Point location, Widget* widget) {
  if (widget == nullptr) {
    widget = root_widget_;
    event_responders_.clear();
  } else if (widget->IsHidden()) {
    return false;
  }

  bool result = false;
  for (auto it = widget->children()->rbegin();
       it != widget->children()->rend(); it++) {
    Widget* child = reinterpret_cast<Widget*>(*it);
    if (UpdateEventResponders(location, child))
      result = true;
  }

  if (widget->ShouldHandleEvent(location)) {
    event_responders_.push_back(widget);
    return true;
  }
  return result;
}

void WidgetView::WidgetViewDidRender(Widget* widget) {
  NVGcontext* context = this->context();
  widget->WidgetViewDidRender(context);
  for (Widget* child : *(widget->children()))
    WidgetViewDidRender(child);
}

void WidgetView::WidgetViewWillRender(Widget* widget) {
  NVGcontext* context = this->context();
  bool result = false;
  while (!result) {
    result = widget->WidgetViewWillRender(context);

    for (Widget* child : *(widget->children()))
      WidgetViewWillRender(child);
  }
}

NVGcontext* WidgetView::context() {
  if (context_ == nullptr) {
#ifdef MOUI_METAL
    context_ = nvgCreateContext(GetLayer(), context_flags_);
#else
    context_ = nvgCreateContext(context_flags_);
#endif  // MOUI_METAL
    SetWidgetContextRecursively(root_widget_, nullptr, context_);
  }
  return context_;
}

}  // namespace moui
