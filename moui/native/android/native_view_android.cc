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

#include <cstdlib>

#include "jni.h"  // NOLINT

#include "moui/core/application.h"
#include "moui/core/device.h"
#include "moui/native/native_object.h"

namespace {

// Returns the instance of the com.ollix.moui.NativeView class on the Java side.
jobject GetJavaNativeView() {
  static jobject java_native_view = nullptr;
  if (java_native_view != nullptr)
    return java_native_view;

  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass native_view_class = env->FindClass("com/ollix/moui/NativeView");
  jmethodID constructor = env->GetMethodID(native_view_class, "<init>",
                                           "(Landroid/content/Context;)V");
  jobject native_view = env->NewObject(native_view_class, constructor,
                                       moui::Application::GetMainActivity());
  java_native_view = env->NewGlobalRef(native_view);
  env->DeleteLocalRef(native_view_class);
  env->DeleteLocalRef(native_view);
  return java_native_view;
}

}  // namespace

namespace moui {

NativeView::NativeView(void* native_handle, const bool releases_on_demand)
    : NativeObject(native_handle, releases_on_demand) {
}

NativeView::NativeView(void* native_handle) : NativeView(native_handle, false) {
}

NativeView::NativeView() : NativeView(nullptr) {
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->FindClass("android/view/View");
  jmethodID constructor = env->GetMethodID(view_class, "<init>",
                                           "(Landroid/content/Context;)V");
  jobject view_obj = env->NewObject(view_class, constructor,
                                    moui::Application::GetMainActivity());
  jobject global_ref = env->NewGlobalRef(view_obj);
  env->DeleteLocalRef(view_class);
  env->DeleteLocalRef(view_obj);
  SetNativeHandle(reinterpret_cast<void*>(global_ref),
                  true);  // releases on demand
}

NativeView::~NativeView() {
}

// Calls com.ollix.moui.NativeView.addSubview() on the Java side.
void NativeView::AddSubview(const NativeView* subview) const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(
      java_class, "addSubview", "(Landroid/view/View;Landroid/view/View;)V");
  env->DeleteLocalRef(java_class);
  env->CallVoidMethod(native_view, java_method,
                      reinterpret_cast<jobject>(native_handle()),
                      reinterpret_cast<jobject>(subview->native_handle()));
}

// Calls android.view.View.bringChildToFront() on the Java side.
void NativeView::BringSubviewToFront(const NativeView* subview) const {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(
      view_class, "bringChildToFront", "(Landroid/view/View;)V");
  env->DeleteLocalRef(view_class);
  env->CallVoidMethod(native_view, java_method,
                      reinterpret_cast<jobject>(subview->native_handle()));
}

// Calls android.view.View.getAlpha() on the Java side.
float NativeView::GetAlpha() const {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(view_class, "getAlpha", "()F");
  env->DeleteLocalRef(view_class);
  return env->CallFloatMethod(native_view, java_method);
}

// Calls com.ollix.moui.NativeView.getHeight() on the Java side.
float NativeView::GetHeight() const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(
      java_class, "getHeight", "(Landroid/view/View;)F");
  env->DeleteLocalRef(java_class);
  return env->CallFloatMethod(native_view, java_method,
                              reinterpret_cast<jobject>(native_handle()));
}

// Calls com.ollix.moui.NativeView.getSnapshot() on the Java side.
unsigned char* NativeView::GetSnapshot() const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(
      java_class, "getSnapshot", "(Landroid/view/View;)[I");
  env->DeleteLocalRef(java_class);
  jintArray pixels = (jintArray)env->CallObjectMethod(
      native_view, java_method, reinterpret_cast<jobject>(native_handle()));
  if (pixels == NULL) {
    return nullptr;
  }
  const jsize kJavaArrayLength = env->GetArrayLength(pixels);
  const float kScreenScaleFactor = Device::GetScreenScaleFactor();
  const float kWidth = GetWidth();
  const float kHeight = GetHeight();
  const int kScaledWidth = kWidth * kScreenScaleFactor;
  const int kScaledHeight = kHeight * kScreenScaleFactor;
  const int kNumberOfPixels = kScaledWidth * kScaledHeight;
  const int kNumberOfBytes = kNumberOfPixels * 4;

  unsigned char* image = nullptr;
  if (kJavaArrayLength == kNumberOfPixels) {
    image = reinterpret_cast<unsigned char*>(std::malloc(kNumberOfBytes));
    if (image != nullptr) {
      jint* colors = env->GetIntArrayElements(pixels, 0);
      unsigned char* color_byte = image;
      for (int i = 0; i < kNumberOfPixels; ++i) {
        const int kColor = colors[i];
        color_byte[0] = (kColor >> 16) & 0xFF;  // red
        color_byte[1] = (kColor >> 8) & 0xFF;   // green
        color_byte[2] = kColor & 0xFF;          // blue
        color_byte[3] = (kColor >> 24) & 0xFF;  // alpha
        color_byte += 4;
      }
      env->ReleaseIntArrayElements(pixels, colors, 0);
    }
  }
  env->DeleteLocalRef(pixels);
  return image;
}

// Calls com.ollix.moui.NativeView.getSuperview() on the Java side.
NativeView* NativeView::GetSuperview() const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(
      java_class, "getSuperview",
      "(Landroid/view/View;)Landroid/view/View;");
  env->DeleteLocalRef(java_class);
  jobject superview = env->CallObjectMethod(
      native_view, java_method, reinterpret_cast<jobject>(native_handle()));
  if (superview == NULL)
    return nullptr;
  jobject global_superview = env->NewGlobalRef(superview);
  env->DeleteLocalRef(superview);
  return new NativeView(reinterpret_cast<void*>(global_superview),
                        true);  // releases on demand
}

// Calls com.ollix.moui.NativeView.getWidth() on the Java side.
float NativeView::GetWidth() const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(
      java_class, "getWidth", "(Landroid/view/View;)F");
  env->DeleteLocalRef(java_class);
  return env->CallFloatMethod(native_view, java_method,
                              reinterpret_cast<jobject>(native_handle()));
}

// Calls com.ollix.moui.NativeView.isHidden() on the Java side.
bool NativeView::IsHidden() const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(
      java_class, "isHidden", "(Landroid/view/View;)Z");
  env->DeleteLocalRef(java_class);
  return env->CallBooleanMethod(native_view, java_method,
                                reinterpret_cast<jobject>(native_handle()));
}

// Calls com.ollix.moui.NativeView.removeFromSuperview() on the Java side.
void NativeView::RemoveFromSuperview() const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(java_class, "removeFromSuperview",
                                           "(Landroid/view/View;)V");
  env->DeleteLocalRef(java_class);
  env->CallVoidMethod(native_view, java_method,
                      reinterpret_cast<jobject>(native_handle()));
}

// Calls com.ollix.moui.NativeView.sendSubviewToBack() on the Java side.
void NativeView::SendSubviewToBack(const NativeView* subview) const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(
      java_class, "sendSubviewToBack",
      "(Landroid/view/View;Landroid/view/View;)V");
  env->DeleteLocalRef(java_class);
  env->CallVoidMethod(native_view, java_method,
                      reinterpret_cast<jobject>(native_handle()),
                      reinterpret_cast<jobject>(subview->native_handle()));
}

// Calls android.view.View.setAlpha() on the Java side.
void NativeView::SetAlpha(const float alpha) const {
  jobject native_view = reinterpret_cast<jobject>(native_handle());
  JNIEnv* env = Application::GetJNIEnv();
  jclass view_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(view_class, "setAlpha", "(F)V");
  env->DeleteLocalRef(view_class);
  env->CallVoidMethod(native_view, java_method, alpha);
}

// Calls com.ollix.moui.NativeView.setBounds() on the Java side.
void NativeView::SetBounds(const float x, const float y, const float width,
                           const float height) const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(java_class, "setBounds",
                                           "(Landroid/view/View;FFFF)V");
  env->DeleteLocalRef(java_class);
  env->CallVoidMethod(native_view, java_method,
                      reinterpret_cast<jobject>(native_handle()),
                      x, y, width, height);
}

// Calls com.ollix.moui.NativeView.setHidden() on the Java side.
void NativeView::SetHidden(const bool hidden) const {
  jobject native_view = GetJavaNativeView();
  JNIEnv* env = Application::GetJNIEnv();
  jclass java_class = env->GetObjectClass(native_view);
  jmethodID java_method = env->GetMethodID(java_class, "setHidden",
                                           "(Landroid/view/View;Z)V");
  env->DeleteLocalRef(java_class);
  env->CallVoidMethod(native_view, java_method,
                      reinterpret_cast<jobject>(native_handle()), hidden);
}

}  // namespace moui
