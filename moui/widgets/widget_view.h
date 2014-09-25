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

#ifndef MOUI_WIDGETS_WIDGET_VIEW_H_
#define MOUI_WIDGETS_WIDGET_VIEW_H_

#include "moui/base.h"
#include "moui/core/event.h"
#include "moui/nanovg_hook.h"
#include "moui/ui/view.h"

namespace moui {

class Widget;

// The WidgetView class is designed specifically for rendering Widget instances.
// It sets up the nanovg context for rendering widgets, and comes with a
// managed widget as the root widget. All other widgets should be added to the
// managed widget for rendering.
class WidgetView : public View {
 public:
  WidgetView();
  ~WidgetView();

  // Inherited from BaseView class.
  virtual void HandleEvent(std::unique_ptr<Event> event) override final;

  // Inherited from BaseView class.
  virtual void Render() override final;

  // Sets the bounds for the view and its managed widget.
  void SetBounds(const int x, const int y, const int width,
                 const int height);

  // Inherited from BaseView class.
  virtual bool ShouldHandleEvent(const Point location) override final;

  // Accessor.
  Widget* widget() { return widget_; }

 private:
  // Renders the specified widget and its children recursively. Note that
  // hidden widgets won't be rendered.
  void RenderWidget(Widget* widget);

  // Inherited from BaseView class.
  bool ShouldHandleEvent(const Point location, Widget* widget);

  // The nanovg context for drawing.
  NVGcontext* context_;

  // The widget that is responsible to respond events passed to the
  // HandleEvent() method. The value is updated in the ShouldHandleEvent()
  // method.
  Widget* event_responder_;

  // The root widget for rendering. All its children will be rendered as well.
  Widget* widget_;

  DISALLOW_COPY_AND_ASSIGN(WidgetView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_WIDGET_VIEW_H_
