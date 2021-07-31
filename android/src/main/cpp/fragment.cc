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

#include "jni.h"

#include "moui/moui.h"

extern "C" {

JNIEXPORT void
JNICALL
Java_com_ollix_moui_MouiFragment_launchUserInterfaceFromJNI(
    JNIEnv *env, jobject) {
  moui::Application* moui_app = moui::Application::GetMainApplication();
  moui_app->LaunchUserInterface();
}

JNIEXPORT void JNICALL
Java_com_ollix_moui_MouiFragment_registerMainApplicationFromJNI(
    JNIEnv *env, jobject obj, jlong moui_app_ptr) {
  auto app = reinterpret_cast<moui::Application *>(moui_app_ptr);
  moui::Application::RegisterMainApplication(app);
}

}  // extern "C"
