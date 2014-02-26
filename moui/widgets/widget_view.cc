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

#include <vector>

#include "nanovg_gl2.h"

#include "moui/nanovg_hook.h"
#include "moui/opengl_hook.h"
#include "moui/ui/view.h"
#include "moui/widgets/widget.h"

namespace moui {

WidgetView::WidgetView() : View(), context_(nullptr) {
}

WidgetView::~WidgetView() {
  if (context_ != nullptr)
    nvgDeleteGLES2(context_);
}

void WidgetView::AddWidget(Widget* widget) {
  widgets_.push_back(widget);
}

void WidgetView::Render() {
  if (context_ == nullptr) {
#if MOUI_IOS || MOUI_ANDROID
    context_ = nvgCreateGLES2(GetWidth(), GetHeight(), NVG_ANTIALIAS);
#else
    context_ = nvgCreateGL2(GetWidth(), GetHeight(), NVG_ANTIALIAS);
#endif
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

#if MOUI_IOS
  int alpha_blend = NVG_PREMULTIPLIED_ALPHA;
#else
  int alpha_blend = NVG_STRAIGHT_ALPHA;
#endif

  nvgBeginFrame(context_, GetWidth(), GetHeight(), GetContentScaleFactor(),
                alpha_blend);
  RenderWidgets(widgets_);
  nvgEndFrame(context_);
}

void WidgetView::RenderWidgets(std::vector<Widget*>& widgets) {
  for (Widget* widget : widgets) {
    if (widget->hidden())
      continue;

    nvgSave(context_);
    nvgScissor(context_, widget->x(), widget->y(), widget->width(),
               widget->height());
    nvgTranslate(context_, widget->x(), widget->y());
    widget->Render(context_);
    RenderWidgets(widget->children());
    nvgRestore(context_);
  }
}

}  // namespace moui
