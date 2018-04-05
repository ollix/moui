// Copyright (c) 2017 Ollix. All rights reserved.
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

#include "moui/core/path.h"

#include "jni.h"  // NOLINT

#include "moui/core/application.h"
#include "moui/core/log.h"

namespace {

// Returns the instance of the com.ollix.moui.Path class on the Java side.
jobject GetJavaPath() {
  static jobject java_path = nullptr;
  if (java_path != nullptr)
    return java_path;

  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass path_class = env->FindClass("com/ollix/moui/Path");
  jmethodID constructor = env->GetMethodID(path_class, "<init>",
                                           "(Landroid/content/Context;)V");
  jobject path_obj = env->NewObject(path_class, constructor,
                                    moui::Application::GetMainActivity());
  java_path = env->NewGlobalRef(path_obj);
  env->DeleteLocalRef(path_class);
  env->DeleteLocalRef(path_obj);
  return java_path;
}

}  // namespace

namespace moui {

std::string Path::GetDirectory(const Directory directory) {
  if (directory == Path::Directory::kResource) {
    return "file:///android_assets";
  }

  std::string java_method_name;
  if (directory == Path::Directory::kDocument) {
    java_method_name = "getExternalFilesDir";
  } else if (directory == Path::Directory::kLibrary) {
    java_method_name = "getFilesDir";
  } else if (directory == Path::Directory::kTemporary) {
    java_method_name = "getCacheDir";
  } else {
    return "";
  }

  jobject path_object = GetJavaPath();
  JNIEnv* env = moui::Application::GetJNIEnv();
  jclass path_class = env->GetObjectClass(path_object);
  jmethodID java_method = env->GetMethodID(
      path_class, java_method_name.c_str(), "()Ljava/lang/String;");
  env->DeleteLocalRef(path_class);
  jstring path_string = reinterpret_cast<jstring>(
      env->CallObjectMethod(path_object, java_method));
  const char* path = env->GetStringUTFChars(path_string, 0);
  std::string result = path;
  env->ReleaseStringUTFChars(path_string, path);
  return result;
}

}  // namespace moui
