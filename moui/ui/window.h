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

#ifndef MOUI_UI_WINDOW_H_
#define MOUI_UI_WINDOW_H_

#include <memory>

#include "moui/base.h"
#include "moui/ui/base_window.h"

namespace moui {

class NativeView;

// The Window class is a wrapper of the platform-specific native window.
class Window : public BaseWindow {
 public:
  explicit Window(void* native_handle);
  ~Window();

  // Returns the main window. For iOS, it's the keyWindow of UIApplication.
  // For Android, it returns the current activity's window. For Mac, it
  // returns the window previously registered through the
  // RegisterMainNativeHandle() method.
  static std::unique_ptr<Window> GetMainWindow();

  // Returns the root view of the current window.
  std::unique_ptr<NativeView> GetRootView() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace moui

#endif  // MOUI_UI_WINDOW_H_
