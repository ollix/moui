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

#ifndef MOUI_CORE_PLATFORM_H_
#define MOUI_CORE_PLATFORM_H_

#include "moui/base.h"

namespace moui {

enum class DeviceCategory {
  kDesktop,
  kPhone,
  kTablet,
  kUnknown,
};

class Platform {
 public:
  Platform() {}
  ~Platform() {}

  // Returns the device category the app is currently running on.
  static DeviceCategory GetDeviceCategory();

#ifdef MOUI_ANDROID
  // Sets the required minimum screen width dp for tablet. The value will
  // affect the returned result of the GetDeviceCategory() method.
  // The default value is 600 dp.
  static void SetSmallestScreenWidthDpForTablet(float screen_width_dp);
#endif

 private:
  DISALLOW_COPY_AND_ASSIGN(Platform);
};

}  // namespace moui

#endif  // MOUI_CORE_PLATFORM_H_
