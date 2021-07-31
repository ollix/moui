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

#include "moui/core/base_application.h"
#include "moui/core/clock.h"
#include "moui/core/device.h"
#include "moui/core/path.h"
#include "moui/native/native_view.h"
#include "moui/ui/window.h"

namespace {

moui::BaseApplication* main_application = nullptr;

}  // namespace

namespace moui {

BaseApplication::BaseApplication() {
}

BaseApplication::~BaseApplication() {
}

BaseApplication* BaseApplication::GetMainApplication() {
  return main_application;
}

void BaseApplication::ResetMainApplication() {
  Clock::Reset();
  Device::Reset();
  Path::Reset();
  NativeView::Reset();
  Window::Reset();
  main_application = nullptr;
}

void BaseApplication::RegisterMainApplication(BaseApplication *app) {
  main_application = app;
}

}  // namespace moui
