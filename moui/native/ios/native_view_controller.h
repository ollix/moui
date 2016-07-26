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

#ifndef MOUI_NATIVE_IOS_NATIVE_VIEW_CONTROLLER_H_
#define MOUI_NATIVE_IOS_NATIVE_VIEW_CONTROLLER_H_

#include <functional>

#include "moui/base.h"
#include "moui/native/native_object.h"

namespace moui {

// Forward declaration.
class NativeView;

// This class is a C++ wrapper for the native `UITextView` class in iOS SDK.
class NativeViewController : public NativeObject {
 public:
  NativeViewController(void* native_handle, const bool releases_on_demand);
  explicit NativeViewController(void* native_handle);
  NativeViewController();
  ~NativeViewController();

  // Returns the view that controller manages.
  NativeView* GetView();

  // Presents a view controller modally.
  void PresentViewController(NativeViewController* view_controller,
                             const bool animated,
                             std::function<void()> completion) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeViewController);
};

}  // namespace moui

#endif  // MOUI_NATIVE_IOS_NATIVE_VIEW_CONTROLLER_H_
