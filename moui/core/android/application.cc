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

#include "moui/core/application.h"

#include "jni.h"

namespace {

JavaVM* java_vm = nullptr;
jobject main_activity = nullptr;
moui::Application* shared_application = nullptr;

}  // namespace

namespace moui {

Application* Application::SharedApplication() {
  if (shared_application == nullptr)
    shared_application = new Application();
  return shared_application;
}

Application::Application() {
}

Application::~Application() {
}

void Application::Init(JNIEnv* env, jobject activity) {
  if (main_activity != nullptr)
    env->DeleteGlobalRef(main_activity);
  main_activity = reinterpret_cast<jobject>(env->NewGlobalRef(activity));
  env->GetJavaVM(&java_vm);
}

JNIEnv* Application::GetJNIEnv() const {
  JNIEnv* env;
  java_vm->GetEnv((void **)&env, JNI_VERSION_1_6);
  return env;
}

jobject Application::GetMainActivity() const {
  return main_activity;
}

}  // namespace moui
