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

#include "moui/core/device.h"

#include <map>

#include "jni.h"  // NOLINT

#include "moui/core/application.h"

namespace {

// The screen scale factor of the current device.
float screen_scale_factor = 0;

// The smallest screen dp that should consider as a tablet. This value can
// be changed through the SetSmallestScreenWidthDpForTablet() method.
float tablet_smallest_screen_width_dp = 600;

std::map<JNIEnv*, jobject> global_devices;

// Returns the instance of the com.ollix.moui.Device class on the Java side.
jobject GetJavaDevice(JNIEnv* env) {
  auto match = global_devices.find(env);
  if (match != global_devices.end()) {
    return match->second;
  }
  jclass device_class = env->FindClass("com/ollix/moui/Device");
  jmethodID constructor = env->GetMethodID(device_class, "<init>",
                                           "(Landroid/content/Context;)V");
  jobject device_obj = env->NewObject(device_class, constructor,
                                      moui::Application::GetMainActivity());
  jobject global_device = env->NewGlobalRef(device_obj);
  env->DeleteLocalRef(device_class);
  env->DeleteLocalRef(device_obj);
  global_devices[env] = global_device;
  return global_device;
}

}  // namespace

namespace moui {

void Device::Init() {
  GetScreenScaleFactor();
}

Device::BatteryState Device::GetBatteryState() {
  JNIEnv* env = moui::Application::GetJNIEnv();
  jobject device = GetJavaDevice(env);
  jclass device_class = env->GetObjectClass(device);
  jmethodID java_method = env->GetMethodID(
      device_class, "getBatteryState", "()I");
  env->DeleteLocalRef(device_class);
  const int kResult = env->CallIntMethod(device, java_method);
  // BatteryManager.BATTERY_STATUS_CHARGING
  if (kResult == 2)
    return BatteryState::kCharging;
  // BatteryManager.BATTERY_STATUS_DISCHARGING
  if (kResult == 3)
    return BatteryState::kUnplugged;
  // BatteryManager.BATTERY_STATUS_NOT_CHARGING
  if (kResult == 4)
    return BatteryState::kNotCharging;
  // BatteryManager.BATTERY_STATUS_FULL
  if (kResult == 5)
    return BatteryState::kFull;
  return BatteryState::kUnknown;
}

// Calls com.ollix.moui.Device.getSmallestScreenWidthDp() on the Java side.
Device::Category Device::GetCategory() {
  static float smallest_screen_width_dp = 0;
  if (smallest_screen_width_dp >= tablet_smallest_screen_width_dp) {
    return Category::kTablet;
  } else if (smallest_screen_width_dp > 0) {
    return Category::kPhone;
  }
  // Determines the smallest_screen_width_dp for the first time calling this
  // method.
  JNIEnv* env = moui::Application::GetJNIEnv();
  jobject device = GetJavaDevice(env);
  jclass device_class = env->GetObjectClass(device);
  jmethodID java_method = env->GetMethodID(
      device_class, "getSmallestScreenWidthDp", "()F");
  env->DeleteLocalRef(device_class);
  smallest_screen_width_dp = env->CallFloatMethod(device, java_method);
  return GetCategory();
}

// Calls com.ollix.moui.Device.getScreenScaleFactor() on the Java side.
float Device::GetScreenScaleFactor() {
  if (screen_scale_factor > 0)
    return screen_scale_factor;

  JNIEnv* env = moui::Application::GetJNIEnv();
  jobject device = GetJavaDevice(env);
  jclass device_class = env->GetObjectClass(device);
  jmethodID java_method = env->GetMethodID(
      device_class, "getScreenScaleFactor", "()F");
  env->DeleteLocalRef(device_class);
  screen_scale_factor = env->CallFloatMethod(device, java_method);
  return screen_scale_factor;
}

void Device::Reset() {
  global_devices.clear();
}

void Device::SetSmallestScreenWidthDpForTablet(float screen_width_dp) {
  tablet_smallest_screen_width_dp = screen_width_dp;
}

}  // namespace moui
