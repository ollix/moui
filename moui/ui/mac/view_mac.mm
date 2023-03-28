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

#include "moui/ui/view.h"

#include "moui/ui/base_view.h"

#if defined(MOUI_GL)
#  import "moui/ui/mac/MOOpenGLView.h"
#elif defined(MOUI_METAL)
#  import "moui/ui/mac/MOMetalView.h"
#endif

namespace moui {

View::View() : BaseView() {
#if defined(MOUI_GL)
  MOOpenGLView* view = [[MOOpenGLView alloc] initWithMouiView:this];
#elif defined(MOUI_METAL)
  MOMetalView* view = [[MOMetalView alloc] initWithMouiView:this];
#endif
  SetNativeHandle((__bridge_retained void*)view, true);
}

View::~View() {
}

bool View::BackgroundIsOpaque() const {
  MOView* native_view = (__bridge MOView*)native_handle();
  return [native_view backgroundIsOpaque];
}

moui::Point View::GetMouseLocation() {
  MOView* native_view = (__bridge MOView*)native_handle();
  return [native_view mouseLocation];
}

#ifdef MOUI_METAL
void View::PresentMetalLayerWithTransaction(const bool value) const {
  MOView* native_view = (__bridge MOView*)native_handle();
  [native_view presentMetalLayerWithTransaction:value];
}
#endif  // MOUI_METAL

void View::Redraw() {
  MOView* native_view = (__bridge MOView*)native_handle();
  [native_view setNeedsRedraw];
}

void View::SetBackgroundOpaque(const bool is_opaque) const {
  MOView* native_view = (__bridge MOView*)native_handle();
  [native_view setBackgroundOpaque:is_opaque];
}

void View::StartUpdatingNativeView() {
  MOView* native_view = (__bridge MOView*)native_handle();
  [native_view startUpdatingView];
}

void View::StopUpdatingNativeView() {
  MOView* native_view = (__bridge MOView*)native_handle();
  [native_view stopUpdatingView];
}

}  // namespace moui
