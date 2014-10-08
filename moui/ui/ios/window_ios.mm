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

#include "moui/ui/window.h"

#include <memory>
#import <UIKit/UIKit.h>

#include "moui/ui/base_window.h"
#include "moui/ui/native_view.h"

namespace moui {

Window::Window(void* native_handle) : BaseWindow(native_handle) {
}

Window::~Window() {
}

std::unique_ptr<Window> Window::GetMainWindow() {
  UIApplication* application = [UIApplication sharedApplication];
  UIWindow* key_window = [application keyWindow];
  return std::unique_ptr<Window>(new Window((__bridge void*)key_window));
}

std::unique_ptr<NativeView> Window::GetRootView() const {
  UIWindow* native_window = (__bridge UIWindow*)native_handle_;
  UIViewController* root_view_controller = [native_window rootViewController];
  UIView* native_view = (UIView*)[root_view_controller view];
  return std::unique_ptr<NativeView>(
      new NativeView((__bridge void*)native_view));
}

}  // namespace moui
