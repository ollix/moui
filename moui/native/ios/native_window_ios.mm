// Copyright (c) 2016 Ollix. All rights reserved.
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

#include "moui/native/native_window.h"

#import <UIKit/UIKit.h>

#include "moui/native/native_view.h"
#include "moui/native/ios/native_view_controller.h"

namespace moui {

NativeWindow::NativeWindow(void* native_handle) : NativeView(native_handle) {
}

NativeWindow::~NativeWindow() {
}

NativeViewController* NativeWindow::GetRootViewController() const {
  UIWindow* window = (__bridge UIWindow*)native_handle();
  return new NativeViewController(window.rootViewController, false);
}

void NativeWindow::MakeKeyAndVisible() const {
  UIWindow* window = (__bridge UIWindow*)native_handle();
  [window makeKeyAndVisible];
}

void NativeWindow::SetRootViewController(
    const NativeViewController* controller) const {
  UIWindow* window = (__bridge UIWindow*)native_handle();
  UIViewController* native_controller = (__bridge UIViewController*)
      controller->native_handle();
  window.rootViewController = native_controller;
}

}  // namespace moui
