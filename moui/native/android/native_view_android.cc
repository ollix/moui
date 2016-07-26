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

#include "moui/native/native_view.h"

#include "jni.h"  // NOLINT

#include "moui/core/application.h"
#include "moui/native/native_object.h"

namespace {

// Returns the instance of the com.ollix.moui.NativeView class on the Java side.
jobject GetJavaNativeView() {
  static jobject java_native_view = nullptr;
  if (java_native_view != nullptr)
    return java_native_view;

  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass native_view_class = env->FindClass("com/ollix/moui/NativeView");
  jmethodID native_view_constructor = env->GetMethodID(
      native_view_class, "<init>", "()V");
  jobject native_view_obj = env->NewObject(native_view_class,
                                           native_view_constructor);
  java_native_view = env->NewGlobalRef(native_view_obj);
  return java_native_view;
}

}  // namespace

namespace moui {

NativeView::NativeView(void* native_handle) : NativeObject(native_handle) {
}

NativeView::NativeView() {
  JNIEnv* env = Application::GetJNIEnv();
  SetNativeHandle(env->NewGlobalRef(reinterpret_cast<jobject>(native_handle)),
                  true);  // releases on demand
}

NativeView::~NativeView() {
}

// Calls com.ollix.moui.NativeView.addSubview() on the Java side.
void NativeView::AddSubview(const NativeView* subview) {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass native_view_class = env->GetObjectClass(native_view);
  jmethodID add_subview_method = env->GetMethodID(
      native_view_class, "addSubview",
      "(Landroid/view/View;Landroid/view/View;)V");
  env->CallVoidMethod(native_view, add_subview_method,
                      reinterpret_cast<jobject>(native_handle()),
                      reinterpret_cast<jobject>(subview->native_handle()));
}

// Calls android.view.View.getHeight() on the Java side.
int NativeView::GetHeight() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID get_height_method = env->GetMethodID(
      view_class, "getHeight", "()I");
  return env->CallIntMethod(native_view, get_height_method);
}

// Calls android.view.View.getWidth() on the Java side.
int NativeView::GetWidth() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID get_width_method = env->GetMethodID(
      view_class, "getWidth", "()I");
  return env->CallIntMethod(native_view, get_width_method);
}

// Calls com.ollix.moui.NativeView.setBounds() on the Java side.
void NativeView::SetBounds(const int x, const int y, const int width,
                           const int height) const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass native_view_class = env->GetObjectClass(native_view);
  jmethodID set_bounds_method = env->GetMethodID(
      native_view_class, "setBounds", "(Landroid/view/View;IIII)V");
  env->CallVoidMethod(native_view, set_bounds_method,
                      reinterpret_cast<jobject>(native_handle()),
                      x, y, width, height);
}

}  // namespace moui
