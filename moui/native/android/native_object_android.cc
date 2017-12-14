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

#include "moui/native/native_object.h"

#include "jni.h"  // NOLINT

#include "moui/core/application.h"

namespace moui {

void NativeObject::ReleaseNativeHandle() {
  if (native_handle_ == nullptr || !releases_on_demand_)
    return;

  JNIEnv* env = Application::GetJNIEnv();
  jobject obj = reinterpret_cast<jobject>(native_handle());
  jobjectRefType ref_type = env->GetObjectRefType(obj);
  if (ref_type == JNIGlobalRefType) {
    env->DeleteGlobalRef(obj);
  } else if (ref_type == JNILocalRefType) {
    env->DeleteLocalRef(obj);
  }
}

}  // namespace moui
