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

#ifndef MOUI_UI_NATIVE_VIEW_H_
#define MOUI_UI_NATIVE_VIEW_H_

#include "moui/base.h"

namespace moui {

// The NativeView behaves as a bridge to the platform-specific native view.
// In iOS, the native view would be UIView or its subclasses. In Android, the
// native view would be android.view.View or its subclasses. The native_handle_
// class member stores the pointer to the real native view.
class NativeView {
 public:
  explicit NativeView(void* native_handle);
  ~NativeView();

  // Adds a subview to the current view.
  void AddSubview(const NativeView* subview);

  // Returns the scale factor used for rendering.
  float GetContentScaleFactor() const;

  // Returns the height of the view.
  int GetHeight() const;

  // Returns the width of the view.
  int GetWidth() const;

  // Sets the bounds of the view.
  void SetBounds(const int x, const int y, const int width,
                 const int height) const;

  // Returns the pointer to the platform-specifc native view.
  void* native_handle() const { return native_handle_; }

 protected:
  // The pointer to the platform-sepcific native view.
  void* native_handle_;

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeView);
};

}  // namespace moui

#endif  // MOUI_UI_NATIVE_VIEW_H_
