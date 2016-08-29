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

#include "moui/native/ios/native_application.h"

#import <UIKit/UIKit.h>

#include "moui/native/native_window.h"

namespace moui {

NativeApplication::NativeApplication(void* native_handle,
                                     const bool releases_on_demand)
    : NativeObject(native_handle, releases_on_demand) {
}

NativeApplication::NativeApplication(void* native_handle)
    : NativeObject(native_handle) {
}

NativeApplication::~NativeApplication() {
}

NativeWindow* NativeApplication::GetKeyWindow() const {
  UIApplication* application = (__bridge UIApplication*)native_handle();
  return new NativeWindow((__bridge void*)application.keyWindow);
}

NativeApplication* NativeApplication::GetSharedApplication() {
  return new NativeApplication(
      (__bridge void*)[UIApplication sharedApplication]);
}

}  // namespace moui
