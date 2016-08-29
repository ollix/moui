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

#include "moui/defines.h"
#include "moui/ui/base_view.h"

#if defined(MOUI_METAL)
#  import "moui/ui/ios/metal_view_ios.h"
#elif defined(MOUI_GLES2) || defined(MOUI_GLES3)
#  import "moui/ui/ios/opengl_view_ios.h"
#endif

namespace moui {

View::View() : BaseView() {
#if defined(MOUI_METAL)
  MOMetalView* view = [[MOMetalView alloc] initWithMouiView:this];
#elif defined(MOUI_GLES2) || defined(MOUI_GLES3)
  MOOpenGLView* view = [[MOOpenGLView alloc] initWithMouiView:this];
#endif
  SetNativeHandle((__bridge_retained void*)view, true);
}

View::~View() {
}

// Calls `CALayer`'s `setNeedsDisplay` method to trigger the `displayLayer:`
// method defined in `MOMetalView`.
void View::Redraw() {
#if defined(MOUI_METAL)
	MOMetalView* native_view = (__bridge MOMetalView*)native_handle();
#elif defined(MOUI_GLES2) || defined(MOUI_GLES3)
	MOOpenGLView* native_view = (__bridge MOOpenGLView*)native_handle();
#endif
  [[native_view layer] setNeedsDisplay];
}

void View::StartUpdatingNativeView() {
#if defined(MOUI_METAL)
	MOMetalView* native_view = (__bridge MOMetalView*)native_handle();
#elif defined(MOUI_GLES2) || defined(MOUI_GLES3)
	MOOpenGLView* native_view = (__bridge MOOpenGLView*)native_handle();
#endif
  [native_view startUpdatingView];
}

void View::StopUpdatingNativeView() {
#if defined(MOUI_METAL)
	MOMetalView* native_view = (__bridge MOMetalView*)native_handle();
#elif defined(MOUI_GLES2) || defined(MOUI_GLES3)
	MOOpenGLView* native_view = (__bridge MOOpenGLView*)native_handle();
#endif
  [native_view stopUpdatingView];
}

}  // namespace moui
