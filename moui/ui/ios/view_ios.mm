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
#import "moui/ui/ios/opengl_view_ios.h"

namespace moui {

// Instantiates the `MOOpenGLViewController` class and uses its view as the
// native handle.
View::View() : BaseView() {
  MOOpenGLView* view = [[MOOpenGLView alloc] initWithMouiView:this];
  SetNativeHandle((__bridge void*)view, true);
}

View::~View() {
}

// Calls `CALayer`'s `setNeedsDisplay` method to trigger the `displayLayer:`
// method defined in `MOOpenGLView`.
void View::Redraw() {
  MOOpenGLView* native_view = (__bridge MOOpenGLView*)native_handle();
  [[native_view layer] setNeedsDisplay];
}

void View::StartUpdatingNativeView() {
  MOOpenGLView* native_view = (__bridge MOOpenGLView*)native_handle();
  [native_view startUpdatingView];
}

void View::StopUpdatingNativeView() {
  MOOpenGLView* native_view = (__bridge MOOpenGLView*)native_handle();
  [native_view stopUpdatingView];
}

}  // namespace moui
