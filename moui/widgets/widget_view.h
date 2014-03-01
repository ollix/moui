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

#include <vector>

#include "moui/base.h"
#include "moui/ui/view.h"

struct NVGcontext;

namespace moui {

class Widget;

class WidgetView : public View {
 public:
  WidgetView();
  ~WidgetView();

  // Adds a widget to this view for rendering.
  void AddWidget(Widget* widget);

  // Inherited from BaseView class.
  virtual void Render() override final;

 private:
  // Renders all child widgets recursively.
  void RenderWidgets(std::vector<Widget*>& widgets);

  // Updates the widget_view property of the specified widget and its children.
  void UpdateWidgetViewForWidget(Widget* widget);

  // The nanovg context for drawing.
  struct NVGcontext* context_;

  // A list of widgets that will be rendered to this view.
  std::vector<Widget*> widgets_;

  DISALLOW_COPY_AND_ASSIGN(WidgetView);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_WIDGET_VIEW_H_
