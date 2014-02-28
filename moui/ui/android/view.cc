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

#include "jni.h"

#include "moui/core/application.h"
#include "moui/ui/base_view.h"

namespace moui {

// Instantiates the JAVA OpenGLView class and sets it as the native handle.
View::View() : BaseView() {
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass opengl_view_class = env->FindClass("com/ollix/moui/OpenGLView");
  jmethodID opengl_view_constructor = env->GetMethodID(
      opengl_view_class, "<init>", "(Landroid/content/Context;J)V");
  jobject java_opengl_view = env->NewObject(
      opengl_view_class, opengl_view_constructor,
      moui::Application::GetMainActivity(), (jlong)this);
  native_handle_ = env->NewGlobalRef(java_opengl_view);
}

View::~View() {
}

void View::Redraw() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle_);
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID request_render_method = env->GetMethodID(view_class,
                                                     "requestRender", "()V");
  env->CallVoidMethod(native_view, request_render_method);
}

}  // namespace moui
