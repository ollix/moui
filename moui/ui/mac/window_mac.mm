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

#import <Cocoa/Cocoa.h>
#include <memory>

#include "moui/native/native_view.h"
#include "moui/ui/base_window.h"

namespace moui {

Window::Window(void* native_handle) : BaseWindow(native_handle) {
}

Window::~Window() {
}

std::unique_ptr<Window> Window::GetMainWindow() {
  void* window = BaseWindow::GetMainNativeHandle();
  return std::unique_ptr<Window>(new Window(window));
}

std::unique_ptr<NativeView> Window::GetRootView() const {
  NSWindow* native_window = (__bridge NSWindow*)native_handle();
  NSView* native_view = (NSView*)[native_window contentView];
  return std::unique_ptr<NativeView>(
      new NativeView((__bridge void*)native_view));
}

}  // namespace moui
