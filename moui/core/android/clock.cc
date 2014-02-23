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

#include "moui/core/clock.h"

#include "jni.h"
#include "moui/core/application.h"

namespace {

jobject GetJavaClock() {
  static jobject java_clock = nullptr;
  if (java_clock != nullptr)
    return java_clock;

  auto application = moui::Application::SharedApplication();
  JNIEnv* env = application->GetJNIEnv();
  jclass clock_class = env->FindClass("com/ollix/moui/Clock");
  jmethodID clock_constructor = env->GetMethodID(
      clock_class, "<init>", "()V");
  jobject clock_obj = env->NewObject(clock_class, clock_constructor);
  java_clock = env->NewGlobalRef(clock_obj);
  return java_clock;
}

}  // namespace

namespace moui {

void Clock::ExecuteCallback(Callback* callback) {
  bool func_result = callback->func();
  if (!func_result || callback->interval < 0) {
    delete callback;
    return;
  }

  auto application = Application::SharedApplication();
  JNIEnv* env = application->GetJNIEnv();
  jobject java_clock = GetJavaClock();
  jclass clock_class = env->GetObjectClass(java_clock);
  jmethodID execute_callback_method = env->GetMethodID(
      clock_class, "executeCallback", "(FJ)V");
  env->CallVoidMethod(java_clock, execute_callback_method, callback->interval,
                      (jlong)callback);
}

}  // namespace moui
