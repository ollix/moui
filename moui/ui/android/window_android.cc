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

#include <map>

#include "jni.h"  // NOLINT

#include "moui/core/application.h"
#include "moui/native/native_view.h"
#include "moui/ui/base_window.h"

namespace {

std::map<JNIEnv*, jobject> global_windows;

// Returns the instance of the com.ollix.moui.Window class on the Java side.
jobject GetJavaWindow(JNIEnv* env) {
  auto match = global_windows.find(env);
  if (match != global_windows.end()) {
    return match->second;
  }
  jclass window_class = env->FindClass("com/ollix/moui/Window");
  jmethodID window_constructor = env->GetMethodID(
      window_class, "<init>", "()V");
  jobject window_obj = env->NewObject(window_class, window_constructor);
  jobject global_window = env->NewGlobalRef(window_obj);
  env->DeleteLocalRef(window_class);
  env->DeleteLocalRef(window_obj);
  global_windows[env] = global_window;
  return global_window;
}

}  // namespace

namespace moui {

// Instantiates the JAVA OpenGLView class and sets it as the native handle.
Window::Window(void* native_handle) : BaseWindow(nullptr) {
  SetNativeHandle(native_handle,
                  true);  // releases on demand
}

Window::~Window() {
}

// Calls com.ollix.moui.Window.getMainWindow() on the Java side.
std::unique_ptr<Window> Window::GetMainWindow() {
  JNIEnv* env = moui::Application::GetJNIEnv();
  jobject java_window = GetJavaWindow(env);
  jclass window_class = env->GetObjectClass(java_window);
  jmethodID get_main_window_method = env->GetMethodID(
      window_class, "getWindow",
      "(Landroid/app/Activity;)Landroid/view/Window;");
  jobject window = env->CallObjectMethod(java_window, get_main_window_method,
                                         moui::Application::GetMainActivity());
  void* global_ref = reinterpret_cast<void*>(env->NewGlobalRef(window));
  env->DeleteLocalRef(window);
  env->DeleteLocalRef(window_class);
  return std::unique_ptr<Window>(new Window(global_ref));
}

// Calls com.ollix.moui.Window.getRootView() on the Java side.
std::unique_ptr<NativeView> Window::GetRootView() const {
  jobject native_window = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = moui::Application::GetJNIEnv();
  jobject java_window = GetJavaWindow(env);
  jclass window_class = env->GetObjectClass(java_window);
  jmethodID get_root_view_method = env->GetMethodID(
      window_class, "getRootView",
      "(Landroid/app/Activity;)Landroid/view/View;");
  jobject view = env->CallObjectMethod(java_window, get_root_view_method,
                                       moui::Application::GetMainActivity());
  void* global_ref = reinterpret_cast<void*>(env->NewGlobalRef(view));
  env->DeleteLocalRef(view);
  env->DeleteLocalRef(window_class);
  return std::unique_ptr<NativeView>(new NativeView(global_ref));
}

}  // namespace moui
