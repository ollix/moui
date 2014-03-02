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

#include "jni.h"

#include "moui/core/application.h"
#include "moui/ui/base_window.h"
#include "moui/ui/native_view.h"

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

std::unique_ptr<Window> Window::GetMainWindow() {
  // JAVA: android_window = activity.getWindow()
  JNIEnv* env = Application::GetJNIEnv();
  jobject activity = Application::GetMainActivity();
  jclass activity_class = env->GetObjectClass(activity);
  jmethodID get_window_method = env->GetMethodID(
      activity_class, "getWindow", "()Landroid/view/Window;");
  jobject android_window = env->CallObjectMethod(activity, get_window_method);
  auto window = new Window(reinterpret_cast<void*>(android_window));
  return std::unique_ptr<Window>(window);
}

std::unique_ptr<NativeView> Window::GetRootView() const {
  jobject native_window = reinterpret_cast<jobject>(native_handle_);
  JNIEnv* env = Application::GetJNIEnv();
  // JAVA: decor_view = native_window.getDecorView()
  jclass window_class = env->GetObjectClass(native_window);
  jmethodID get_decor_view_method = env->GetMethodID(
      window_class, "getDecorView", "()Landroid/view/View;");
  jobject decor_view = env->CallObjectMethod(native_window,
                                             get_decor_view_method);
  // JAVA: content_view = decor_view.findViewById(android.R.id.content)
  jclass view_class = env->GetObjectClass(decor_view);
  jmethodID find_view_by_id_method = env->GetMethodID(
      view_class, "findViewById", "(I)Landroid/view/View;");
  jobject content_view = env->CallObjectMethod(
      decor_view,
      find_view_by_id_method,
      0x01020002);  // android.R.id.content
  return std::unique_ptr<NativeView>(
      new NativeView(reinterpret_cast<void*>(content_view)));
}

}  // namespace moui
