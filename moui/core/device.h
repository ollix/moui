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

#ifndef MOUI_CORE_DEVICE_H_
#define MOUI_CORE_DEVICE_H_

#include "moui/base.h"

namespace moui {

class Device {
 public:
  enum class Category {
    kDesktop,
    kPhone,
    kTablet,
  };

  Device() {}
  ~Device() {}

  // Returns the device category the app is currently running on.
  static Category GetCategory();

  // Returns the natural scale factor associated with the screen. This value
  // reflacts the scale factor needed to convert from the default logical
  // coordinate space into the device coordinate space of the screen.
  static float GetScreenScaleFactor();

#ifdef MOUI_ANDROID
  // Sets the required minimum screen width dp for tablet. The value will
  // affect the returned result of the GetCategory() method. The default
  // value is 600 dp.
  static void SetSmallestScreenWidthDpForTablet(float screen_width_dp);
#endif

 private:
  DISALLOW_COPY_AND_ASSIGN(Device);
};

}  // namespace moui

#endif  // MOUI_CORE_DEVICE_H_
