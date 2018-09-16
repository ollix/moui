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

#if defined(MOUI_IOS)
#import <UIKit/UIKit.h>
#elif defined(MOUI_MAC)
#import <Cocoa/Cocoa.h>
#endif

namespace moui {

Device::BatteryState Device::GetBatteryState() {
#ifdef MOUI_MAC
  return BatteryState::kUnknown;
#else
  switch ([UIDevice currentDevice].batteryState) {
    case UIDeviceBatteryStateUnplugged:
      return BatteryState::kUnplugged;
    case UIDeviceBatteryStateCharging:
      return BatteryState::kCharging;
    case UIDeviceBatteryStateFull:
      return BatteryState::kFull;
    default:
      return BatteryState::kUnknown;
  }
#endif
}

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
  static float screen_scale_factor = 0;
  if (screen_scale_factor > 0)
    return screen_scale_factor;

#if MOUI_IOS
  screen_scale_factor = [[UIScreen mainScreen] scale];
#elif MOUI_MAC
  screen_scale_factor = [[NSScreen mainScreen] backingScaleFactor];
#else
  assert(false);
#endif  // MOUI_IOS
  return screen_scale_factor;
}

void Device::Reset() {
}

}  // namespace moui
