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

#include <map>

#include "jni.h"  // NOLINT

#include "moui/core/application.h"

namespace {

// Caches the directory paths.
std::string document_directory_path;
std::string library_directory_path;
std::string temporary_directory_path;

std::map<JNIEnv*, jobject> global_paths;

// Returns the instance of the com.ollix.moui.Path class on the Java side.
jobject GetJavaPath(JNIEnv* env) {
  auto match = global_paths.find(env);
  if (match != global_paths.end()) {
    return match->second;
  }
  jclass path_class = env->FindClass("com/ollix/moui/Path");
  jmethodID constructor = env->GetMethodID(path_class, "<init>",
                                           "(Landroid/content/Context;)V");
  jobject path_obj = env->NewObject(path_class, constructor,
                                    moui::Application::GetMainActivity());
  jobject global_path = env->NewGlobalRef(path_obj);
  env->DeleteLocalRef(path_class);
  env->DeleteLocalRef(path_obj);
  global_paths[env] = global_path;
  return global_path;
}

}  // namespace

namespace moui {

void Path::Init() {
  Path::GetDirectory(Path::Directory::kDocument);
  Path::GetDirectory(Path::Directory::kLibrary);
  Path::GetDirectory(Path::Directory::kTemporary);
}

std::string Path::GetDirectory(const Directory directory) {
  if (directory == Path::Directory::kResource) {
    return "file:///android_assets";
  }

  std::string java_method_name;
  std::string* path_cache;
  if (directory == Path::Directory::kDocument) {
    java_method_name = "getDocumentDir";
    path_cache = &document_directory_path;
  } else if (directory == Path::Directory::kLibrary) {
    java_method_name = "getFilesDir";
    path_cache = &library_directory_path;
  } else if (directory == Path::Directory::kTemporary) {
    java_method_name = "getCacheDir";
    path_cache = &temporary_directory_path;
  } else {
    return "";
  }
  if (!path_cache->empty()) {
    return *path_cache;
  }

  JNIEnv* env = moui::Application::GetJNIEnv();
  jobject path_object = GetJavaPath(env);
  jclass path_class = env->GetObjectClass(path_object);
  jmethodID java_method = env->GetMethodID(
      path_class, java_method_name.c_str(), "()Ljava/lang/String;");
  env->DeleteLocalRef(path_class);
  jstring path_string = reinterpret_cast<jstring>(
      env->CallObjectMethod(path_object, java_method));
  const char* path = env->GetStringUTFChars(path_string, 0);
  *path_cache = std::string(path);
  env->ReleaseStringUTFChars(path_string, path);
  env->DeleteLocalRef(path_string);
  return *path_cache;
}

void Path::Reset() {
  global_paths.clear();
}

}  // namespace moui
