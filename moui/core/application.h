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

#ifndef MOUI_CORE_APPLICATION_H_
#define MOUI_CORE_APPLICATION_H_

#include "moui/base.h"
#include "moui/core/base_application.h"

#ifdef MOUI_ANDROID
#include "jni.h"  // NOLINT
#endif  // MOUI_ANDROID

namespace moui {

// The `Application` class is the base class for custom applications. This
// class should never be instantiated directly. Instaed, creating a subclass
// and implement virtual methods there. The custom application instance should
// call the `SetAsMainApplication`() method once right after instantiated.
// For Android support, the `InitJNI()` method should also be called before
// instantiating.
class Application : public BaseApplication {
 public:
  Application() {}
  ~Application() {}

  // Calls this method to destroy the user interface.
  virtual void DestroyUserInterface() {}

  // Returns the main application. The main application must be registered
  // before calling this method. To register the main application, call the
  // `BaseApplication::RegisterMainApplication()` method on an application
  // instance.
  static Application* GetMainApplication() {
    BaseApplication* application = BaseApplication::GetMainApplication();
    return reinterpret_cast<Application*>(application);
  }

  // This method is designed to be called whenever the application receives a
  // memory warning.
  virtual void HandleMemoryWarning() {}

  // Calls this method to launch the user interface.
  virtual void LaunchUserInterface() {}

#ifdef MOUI_ANDROID
  // Initializes the JNI environemnt. This method must be called at least once.
  static void InitJNI(JNIEnv* env, jobject activity, jobject asset_manager);

  // Returns the `JNIEnv` variable that set in the `InitJNI()` method.
  static JNIEnv* GetJNIEnv();

  // Returns the `Activity` object that set in the `InitJNI()` method.
  static jobject GetMainActivity();
#endif  // MOUI_ANDROID

 private:
  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace moui

#endif  // MOUI_CORE_APPLICATION_H_
