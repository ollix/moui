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

#ifndef MOUI_NATIVE_NATIVE_WINDOW_H_
#define MOUI_NATIVE_NATIVE_WINDOW_H_

#include "moui/base.h"
#include "moui/native/native_view.h"

namespace moui {

// Forward declaration.
class NativeViewController;

class NativeWindow : public NativeView {
 public:
  explicit NativeWindow(void* native_handle);
  ~NativeWindow();

#ifdef MOUI_IOS
  // Returns the root view controller for the window.
  NativeViewController* GetRootViewController() const;

  // Shows the window and makes it the key window.
  void MakeKeyAndVisible() const;

  // Sets the root view controller for the window.
  void SetRootViewController(const NativeViewController* controller) const;
#endif

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeWindow);
};

}  // namespace moui

#endif  // MOUI_NATIVE_NATIVE_WINDOW_H_
