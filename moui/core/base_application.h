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

#ifndef MOUI_CORE_BASE_APPLICATION_H_
#define MOUI_CORE_BASE_APPLICATION_H_

#include "moui/base.h"

namespace moui {

// The BaseApplication class is the base class of the Application class. It is
// responsible for setting and getting the main application. This class should
// only be inherited by the Application class.
class BaseApplication {
 public:
  BaseApplication();
  ~BaseApplication();

  // Returns the pointer to the application that previously set through the
  // RegisterMainApplication() method.
  static BaseApplication* GetMainApplication();

  // Registers the current instance as the main application. The instance can
  // later be retrieved through the GetMainApplication() method. If this method
  // is called multiple times, the last instance calling this method will be
  // the main application.
  void RegisterMainApplication();

 private:
  DISALLOW_COPY_AND_ASSIGN(BaseApplication);
};

}  // namespace moui

#endif  // MOUI_CORE_BASE_APPLICATION_H_
