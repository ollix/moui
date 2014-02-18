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
#include "moui/core/android/application.h"
#include "moui/ui/base_view.h"

namespace moui {

// Instantiates the JAVA OpenGLView class and sets it as the native handle.
View::View() : BaseView() {
  auto application = Application::SharedApplication();
  JNIEnv* env = application->GetJNIEnv();
  jclass opengl_view_class = env->FindClass("com/ollix/moui/OpenGLView");
  jmethodID opengl_view_constructor = env->GetMethodID(
      opengl_view_class, "<init>", "(Landroid/content/Context;J)V");
  jobject java_opengl_view = env->NewObject(opengl_view_class,
                                            opengl_view_constructor,
                                            application->GetMainActivity(),
                                            (long)this);
  native_handle_ = env->NewGlobalRef(java_opengl_view);
}

View::~View() {
}

// The implmenetation simulates the JAVA code as same as
// activity.getResources().getDisplayMetrics().density.
float View::GetContentScaleFactor() const {
  static float content_scale_factor = 0;
  if (content_scale_factor > 0)
    return content_scale_factor;

  auto application = Application::SharedApplication();
  JNIEnv* env = application->GetJNIEnv();
  // JAVA: Resources resources = activity.getResources()
  jobject activity = application->GetMainActivity();
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

int View::GetHeight() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle_);
  auto application = moui::Application::SharedApplication();
  JNIEnv* env = application->GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID get_height_method = env->GetMethodID(
      view_class, "getHeight", "()I");
  return env->CallIntMethod(native_view, get_height_method);
}

int View::GetWidth() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle_);
  auto application = moui::Application::SharedApplication();
  JNIEnv* env = application->GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID get_width_method = env->GetMethodID(
      view_class, "getWidth", "()I");
  return env->CallIntMethod(native_view, get_width_method);
}

void View::Redraw() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle_);
  auto application = moui::Application::SharedApplication();
  JNIEnv* env = application->GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID request_render_method = env->GetMethodID(view_class,
                                                     "requestRender", "()V");
  env->CallVoidMethod(native_view, request_render_method);
}

void View::ScheduleRedraw(double interval) const {
  Redraw();
}

}  // namespace moui
