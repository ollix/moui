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

#ifndef MOUI_NATIVE_NATIVE_OBJECT_H_
#define MOUI_NATIVE_NATIVE_OBJECT_H_

#include "moui/base.h"

namespace moui {

// The `NativeObject` behaves as a bridge to the platform-specific native
// object.
class NativeObject {
 public:
  NativeObject(void* native_handle, const bool releases_on_demand)
      : native_handle_(native_handle),
        releases_on_demand_(releases_on_demand) {}
  explicit NativeObject(void* native_handle)
      : NativeObject(native_handle, false) {}
  ~NativeObject() {
    if (releases_on_demand_)
      ReleaseNativeHandle();
  }

  // Accessors.
  void* native_handle() const { return native_handle_; }

 protected:
  // Updates the native handle and sets if the handle should be released
  // automatically when no longer needed.
  void SetNativeHandle(void* native_handle, const bool releases_on_demand) {
    if (releases_on_demand_)
      ReleaseNativeHandle();
    native_handle_ = native_handle;
    releases_on_demand_ = releases_on_demand;
  }

 private:
  // Releases the native handle.
  void ReleaseNativeHandle();

  // Indicates the pointer to the platform-sepcific native object.
  void* native_handle_;

  // Indicates whether the `native_handle_` should be released on demand.
  bool releases_on_demand_;

  DISALLOW_COPY_AND_ASSIGN(NativeObject);
};

}  // namespace moui

#endif  // MOUI_NATIVE_NATIVE_OBJECT_H_
