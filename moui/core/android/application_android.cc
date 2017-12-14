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

#include "moui/core/application.h"

#include "jni.h"  // NOLINT

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "aasset.h"

namespace {

JavaVM* java_vm = nullptr;
jobject main_activity = nullptr;

}  // namespace

namespace moui {

void Application::InitJNI(JNIEnv* env, jobject activity,
                          jobject asset_manager) {
  if (main_activity != nullptr) {
    env->DeleteGlobalRef(main_activity);
  }
  main_activity = reinterpret_cast<jobject>(env->NewGlobalRef(activity));
  env->GetJavaVM(&java_vm);

  // Initializes the aasset library.
  aasset_init(AAssetManager_fromJava(env, asset_manager));
}

JNIEnv* Application::GetJNIEnv() {
  JNIEnv* env;
  java_vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
  return env;
}

jobject Application::GetMainActivity() {
  return main_activity;
}

}  // namespace moui
