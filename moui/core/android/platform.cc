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

#include "moui/core/platform.h"

#include "jni.h"

#include "moui/core/application.h"

namespace {

// The smallest screen dp that should consider as a tablet. This value can
// be changed through the SetSmallestScreenWidthDpForTablet() method.
float tablet_smallest_screen_width_dp = 600;

// Returns the instance of the com.ollix.moui.Platform class on the Java side.
jobject GetJavaPlatform() {
  static jobject java_platform = nullptr;
  if (java_platform != nullptr)
    return java_platform;

  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass platform_class = env->FindClass("com/ollix/moui/Platform");
  jmethodID platform_constructor = env->GetMethodID(
      platform_class, "<init>", "(Landroid/app/Activity;)V");
  jobject platform_obj = env->NewObject(platform_class, platform_constructor,
                                        moui::Application::GetMainActivity());
  java_platform = env->NewGlobalRef(platform_obj);
  return java_platform;
}

}  // namespace

namespace moui {

// Calls com.ollix.moui.Platform.getSmallestScreenWidthDp() on the Java side.
DeviceCategory Platform::GetDeviceCategory() {
  static float smallest_screen_width_dp = 0;
  if (smallest_screen_width_dp >= tablet_smallest_screen_width_dp) {
    return DeviceCategory::kTablet;
  } else if (smallest_screen_width_dp > 0) {
    return DeviceCategory::kPhone;
  }
  // Determines the smallest_screen_width_dp for the first time calling this
  // method.
  jobject platform = GetJavaPlatform();
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass platform_class = env->GetObjectClass(platform);
  jmethodID get_smallest_screen_width_dp_method = env->GetMethodID(
      platform_class, "getSmallestScreenWidthDp", "()F");
  smallest_screen_width_dp = env->CallFloatMethod(
      platform, get_smallest_screen_width_dp_method);
  return GetDeviceCategory();
}

void Platform::SetSmallestScreenWidthDpForTablet(float screen_width_dp) {
  tablet_smallest_screen_width_dp = screen_width_dp;
}

}  // namespace moui
