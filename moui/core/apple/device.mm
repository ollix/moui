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

#include "moui/core/device.h"

#include <cassert>

#if MOUI_IOS
#import <UIKit/UIKit.h>
#elif MOUI_MAC
#import <Cocoa/Cocoa.h>
#endif

namespace moui {

Device::Category Device::GetCategory() {
#if MOUI_MAC
  return Category::kDesktop;
#elif MOUI_IOS
  UIUserInterfaceIdiom user_interface_idiom = UI_USER_INTERFACE_IDIOM();
  if (user_interface_idiom == UIUserInterfaceIdiomPhone)
    return Category::kPhone;
  else if (user_interface_idiom == UIUserInterfaceIdiomPad)
    return Category::kTablet;
#endif
  assert(false);
}

float Device::GetScreenScaleFactor() {
#if MOUI_IOS
  return [[UIScreen mainScreen] scale];
#elif MOUI_MAC
  return [[NSScreen mainScreen] backingScaleFactor];
#endif
  assert(false);
}

}  // namespace moui
