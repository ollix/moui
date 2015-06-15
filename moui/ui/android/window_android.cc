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

#include "moui/ui/window.h"

#include "jni.h"  // NOLINT

#include "moui/core/application.h"
#include "moui/ui/base_window.h"
#include "moui/ui/native_view.h"

namespace {

// Returns the instance of the com.ollix.moui.Window class on the Java side.
jobject GetJavaWindow() {
  static jobject java_window = nullptr;
  if (java_window != nullptr)
    return java_window;

  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass window_class = env->FindClass("com/ollix/moui/Window");
  jmethodID window_constructor = env->GetMethodID(
      window_class, "<init>", "(Landroid/app/Activity;)V");
  jobject window_obj = env->NewObject(window_class, window_constructor,
                                      moui::Application::GetMainActivity());
  java_window = env->NewGlobalRef(window_obj);
  return java_window;
}

}  // namespace

namespace moui {

// Instantiates the JAVA OpenGLView class and sets it as the native handle.
Window::Window(void* native_handle) : BaseWindow(nullptr) {
  JNIEnv* env = Application::GetJNIEnv();
  native_handle_ = env->NewGlobalRef(reinterpret_cast<jobject>(native_handle));
}

Window::~Window() {
  JNIEnv* env = Application::GetJNIEnv();
  env->DeleteGlobalRef(reinterpret_cast<jobject>(native_handle_));
}

// Calls com.ollix.moui.Window.getMainWindow() on the Java side.
std::unique_ptr<Window> Window::GetMainWindow() {
  jobject java_window = GetJavaWindow();
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass window_class = env->GetObjectClass(java_window);
  jmethodID get_main_window_method = env->GetMethodID(
      window_class, "getMainWindow", "()Landroid/view/Window;");
  jobject window = env->CallObjectMethod(java_window, get_main_window_method);
  return std::unique_ptr<Window>(new Window(reinterpret_cast<void*>(window)));
}

// Calls com.ollix.moui.Window.getRootView() on the Java side.
std::unique_ptr<NativeView> Window::GetRootView() const {
  jobject native_window = reinterpret_cast<jobject>(native_handle_);
  jobject java_window = GetJavaWindow();
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass window_class = env->GetObjectClass(java_window);
  jmethodID get_root_view_method = env->GetMethodID(
      window_class, "getRootView",
      "(Landroid/view/Window;)Landroid/view/View;");
  jobject view = env->CallObjectMethod(java_window, get_root_view_method,
                                       native_window);
  return std::unique_ptr<NativeView>(
      new NativeView(reinterpret_cast<void*>(view)));
}

}  // namespace moui
