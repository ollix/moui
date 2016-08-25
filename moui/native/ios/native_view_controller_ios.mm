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

#include "moui/native/ios/native_view_controller.h"

#import <UIKit/UIKit.h>

#include "moui/native/native_view.h"

namespace moui {

NativeViewController::NativeViewController(void* native_handle,
                                           const bool releases_on_demand)
    : NativeObject(native_handle, releases_on_demand) {
}

NativeViewController::NativeViewController(void* native_handle)
    : NativeObject(native_handle) {
}

NativeViewController::NativeViewController() : NativeViewController(nullptr) {
  UIViewController* view_controller = [[UIViewController alloc] init];
  SetNativeHandle((__bridge void*)view_controller, true);
}

NativeViewController::~NativeViewController() {
}

NativeView* NativeViewController::GetView() {
  UIViewController* controller = (__bridge UIViewController*)native_handle();
  return new NativeView(controller.view);
}

void NativeViewController::PresentViewController(
    NativeViewController* view_controller, const bool animated,
    std::function<void()> completion) const {
  UIViewController* controller = (__bridge UIViewController*)native_handle();
  [controller presentViewController:
          (__bridge UIViewController*)view_controller->native_handle()
      animated:animated
      completion:^(void) { if (completion != NULL) completion(); }];
}

}  // namespace moui
