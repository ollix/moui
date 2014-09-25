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
#import "moui/ui/ios/MOOpenGLView.h"
#import "moui/ui/ios/MOOpenGLViewController.h"

namespace moui {

// Instantiates the MOOpenGLViewController class and uses its view as the
// native handle.
View::View() : BaseView() {
  MOOpenGLViewController* view_controller = \
      [[MOOpenGLViewController alloc] initWithMouiView:this];
  native_handle_ = (__bridge void*)view_controller.view;
}

// Releases the native view if it's a moui-customized MOOpenGLView class.
View::~View() {
  MOOpenGLView* native_view = (__bridge MOOpenGLView*)native_handle_;
  [native_view.viewController dealloc];
}

void View::RenderNativeView() const {
  MOOpenGLView* native_view = (__bridge MOOpenGLView*)native_handle_;
  [native_view render];
}

}  // namespace moui
