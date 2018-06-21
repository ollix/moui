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

#include "moui/ui/view.h"

#include "jni.h"  // NOLINT

#include "moui/core/application.h"
#include "moui/ui/base_view.h"

namespace moui {

// Instantiates the JAVA OpenGLView class and sets it as the native handle.
View::View() : BaseView() {
  JNIEnv* env = Application::GetJNIEnv();
  jclass opengl_view_class = env->FindClass("com/ollix/moui/OpenGLView");
  jmethodID opengl_view_constructor = env->GetMethodID(
      opengl_view_class, "<init>", "(Landroid/content/Context;J)V");
  jobject java_opengl_view = env->NewObject(opengl_view_class,
                                            opengl_view_constructor,
                                            Application::GetMainActivity(),
                                            reinterpret_cast<jlong>(this));
  jobject global_ref = env->NewGlobalRef(java_opengl_view);
  env->DeleteLocalRef(opengl_view_class);
  env->DeleteLocalRef(java_opengl_view);
  SetNativeHandle(reinterpret_cast<void*>(global_ref),
                  true);  // releases on demand
}

View::~View() {
}

bool View::BackgroundIsOpaque() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID method = env->GetMethodID(view_class, "backgroundIsOpaque", "()Z");
  return env->CallBooleanMethod(native_view, method);
}

// Calls com.ollix.OpenGLView.requestRender() on the Java side.
void View::Redraw() {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID request_render_method = env->GetMethodID(view_class,
                                                     "redrawView", "()V");
  env->CallVoidMethod(native_view, request_render_method);
}

void View::SetBackgroundOpaque(const bool is_opaque) const {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID method = env->GetMethodID(view_class, "setBackgroundOpaque",
                                      "(Z)V");
  env->CallVoidMethod(native_view, method, is_opaque);
}

// Calls com.ollix.OpenGLView.startUpdatingView() on the Java side.
void View::StartUpdatingNativeView() {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID start_updating_view_method = env->GetMethodID(
      view_class, "startUpdatingView", "()V");
  env->CallVoidMethod(native_view, start_updating_view_method);
}

// Calls com.ollix.OpenGLView.stopUpdatingView() on the Java side.
void View::StopUpdatingNativeView() {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID stop_updating_view_method = env->GetMethodID(
      view_class, "stopUpdatingView", "()V");
  env->CallVoidMethod(native_view, stop_updating_view_method);
}

}  // namespace moui
