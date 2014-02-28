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

#include "moui/ui/base_view.h"

#include "jni.h"

#include "moui/core/application.h"

namespace moui {

NativeView::NativeView(void* native_handle) {
  if (native_handle == nullptr)
    return;

  JNIEnv* env = moui::Application::GetJNIEnv();
  native_handle_ = env->NewGlobalRef(reinterpret_cast<jobject>(native_handle));
}

NativeView::~NativeView() {
  if (native_handle_ == nullptr)
    return;

  JNIEnv* env = moui::Application::GetJNIEnv();
  env->DeleteGlobalRef(reinterpret_cast<jobject>(native_handle_));
}

void NativeView::AddSubview(const NativeView* subview) {
  jobject native_view = reinterpret_cast<jobject>(native_handle_);
  jobject native_subview = reinterpret_cast<jobject>(subview->native_handle());

  // Skips if the native view is not a subclass of ViewGroup.
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass group_view_class = env->FindClass("android/view/ViewGroup");
  if (!env->IsInstanceOf(native_view, group_view_class))
    return;

  jmethodID add_view_method = env->GetMethodID(
      group_view_class, "addView", "(Landroid/view/View;)V");
  env->CallVoidMethod(native_view, add_view_method, native_subview);
}

// The implmenetation simulates the JAVA code as same as
// activity.getResources().getDisplayMetrics().density.
float NativeView::GetContentScaleFactor() const {
  static float content_scale_factor = 0;
  if (content_scale_factor > 0)
    return content_scale_factor;

  // JAVA: Resources resources = activity.getResources()
  JNIEnv* env = moui::Application::GetJNIEnv();
  jobject activity = moui::Application::GetMainActivity();
  jclass activity_class = env->GetObjectClass(activity);
  jmethodID get_resources_method = env->GetMethodID(
      activity_class, "getResources", "()Landroid/content/res/Resources;");
  jobject resources = env->CallObjectMethod(activity, get_resources_method);
  // JAVA: DisplayMetrics metrics = resources.getDisplayMetrics()
  jclass resources_class = env->GetObjectClass(resources);
  jmethodID get_display_metrics_method = env->GetMethodID(
      resources_class, "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
  jobject metrics = env->CallObjectMethod(resources,
                                          get_display_metrics_method);
  // JAVA: metrics.density
  jclass metrics_class = env->GetObjectClass(metrics);
  jfieldID density_field = env->GetFieldID(metrics_class, "density", "F");
  content_scale_factor = env->GetFloatField(metrics, density_field);
  return content_scale_factor;
}

int NativeView::GetHeight() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle_);
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID get_height_method = env->GetMethodID(
      view_class, "getHeight", "()I");
  return env->CallIntMethod(native_view, get_height_method);
}

int NativeView::GetWidth() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle_);
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID get_width_method = env->GetMethodID(
      view_class, "getWidth", "()I");
  return env->CallIntMethod(native_view, get_width_method);
}

void NativeView::SetBounds(const int x, const int y, const int width,
                           const int height) const {
  // TODO(olliwang): Implmenets SetBounds() for Android.
}

}  // namespace moui
