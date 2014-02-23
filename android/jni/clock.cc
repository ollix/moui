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

#include <memory>

#include "jni.h"
#include "moui/moui.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_ollix_moui_Clock_executeCallback
    (JNIEnv* env, jobject obj, jlong callback_pointer) {
  auto callback = reinterpret_cast<moui::Clock::Callback*>(callback_pointer);
  moui::Clock::ExecuteCallback(callback);
}

#ifdef __cplusplus
}
#endif
