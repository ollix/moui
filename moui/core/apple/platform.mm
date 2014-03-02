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

#include "moui/core/platform.h"

#ifdef MOUI_IOS
#import <UIKit/UIKit.h>
#endif

namespace moui {

DeviceCategory Platform::GetDeviceCategory() {
#if MOUI_MAC
  return DeviceCategory::kDesktop;
#elif MOUI_IOS
  UIUserInterfaceIdiom user_interface_idiom = UI_USER_INTERFACE_IDIOM();
  if (user_interface_idiom == UIUserInterfaceIdiomPhone)
    return DeviceCategory::kPhone;
  else if (user_interface_idiom == UIUserInterfaceIdiomPad)
    return DeviceCategory::kTablet;
#endif
  return DeviceCategory::kUnknown;
}

}  // namespace moui
