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

#include <functional>
#include <map>

#include "jni.h"  // NOLINT

#include "moui/core/application.h"

namespace {

std::map<JNIEnv*, jobject> global_clocks;

jobject GetJavaClock(JNIEnv* env) {
  auto match = global_clocks.find(env);
  if (match != global_clocks.end()) {
    return match->second;
  }
  jclass clock_class = env->FindClass("com/ollix/moui/Clock");
  jmethodID constructor = env->GetMethodID(clock_class, "<init>", "()V");
  jobject clock_obj = env->NewObject(clock_class, constructor);
  jobject global_clock = env->NewGlobalRef(clock_obj);
  env->DeleteLocalRef(clock_class);
  env->DeleteLocalRef(clock_obj);
  global_clocks[env] = global_clock;
  return global_clock;
}

}  // namespace

namespace moui {

void Clock::DispatchAfter(const float delay, std::function<void()> func) {
  JNIEnv* env = Application::GetJNIEnv();
  jobject java_clock = GetJavaClock(env);
  jclass clock_class = env->GetObjectClass(java_clock);
  jmethodID java_method = env->GetMethodID(
      clock_class, "dispatchAfter", "(FJ)V");
  env->DeleteLocalRef(clock_class);
  env->CallVoidMethod(java_clock, java_method, delay,
                      reinterpret_cast<jlong>(new Callback{func}));
}

void Clock::ExecuteCallbackOnMainThread(const float delay,
                                        std::function<void()> func) {
  JNIEnv* env = Application::GetJNIEnv();
  jobject java_clock = GetJavaClock(env);
  jclass clock_class = env->GetObjectClass(java_clock);
  jmethodID java_method = env->GetMethodID(
      clock_class, "executeCallbackOnMainThread", "(FJ)V");
  env->DeleteLocalRef(clock_class);
  env->CallVoidMethod(java_clock, java_method, delay,
                      reinterpret_cast<jlong>(new Callback{func}));
}

void Clock::ExecuteCallbackOnMainThread(std::function<void()> callback) {
  ExecuteCallbackOnMainThread(0, callback);
}

}  // namespace moui
