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

#include <cassert>
#include <vector>

#include "jni.h"

#include "moui/moui.h"

extern "C" {

JNIEXPORT bool
JNICALL
Java_com_ollix_moui_View_drawFrameFromJNI(
    JNIEnv* env, jobject, jlong moui_view_ptr) {
  auto view = reinterpret_cast<moui::View*>(moui_view_ptr);
  return view->Render();
}

// Handles the event in moui view.
JNIEXPORT void
JNICALL
Java_com_ollix_moui_View_handleEventFromJNI(
    JNIEnv* env, jobject, jlong moui_view_ptr, jint action,
    jfloatArray raw_locations) {
  // Converts the received MotionEvent action to moui event type.
  moui::Event::Type event_type;
  switch (action) {
    case 0x00000003:  // MotionEvent.ACTION_CANCEL
      event_type = moui::Event::Type::kCancel; break;
    case 0x00000000:  // MotionEvent.ACTION_DOWN
      event_type = moui::Event::Type::kDown; break;
    case 0x00000002:  // MotionEvent.ACTION_MOVE
      event_type = moui::Event::Type::kMove; break;
    case 0x00000001:  // MotionEvent.ACTION_UP
      event_type = moui::Event::Type::kUp; break;
    default:
      event_type = moui::Event::Type::kUnknown;
  }
  // Initializes the event object and asks moui view to handle the event.
  moui::Event moui_event(event_type);
  const int kNumberOfLocations = \
      static_cast<int>(env->GetArrayLength(raw_locations)) / 2;
  jfloat* locations = env->GetFloatArrayElements(raw_locations, 0);
  for (int i = 0; i < kNumberOfLocations; ++i) {
    moui_event.locations()->push_back({*locations++, *locations++});
  }
  auto moui_view = reinterpret_cast<moui::View*>(moui_view_ptr);
  moui_view->HandleEvent(&moui_event);
}

JNIEXPORT void
JNICALL
Java_com_ollix_moui_View_onSurfaceDestroyedFromJNI(
    JNIEnv* env, jobject, jlong moui_view_ptr) {
  auto view = reinterpret_cast<moui::View*>(moui_view_ptr);
  view->OnSurfaceDestroyed();
}

// Asks moui view if the event should be handled.
JNIEXPORT jboolean
JNICALL
Java_com_ollix_moui_View_shouldHandleEventFromJNI(
    JNIEnv* env, jobject, jlong moui_view_ptr, jfloat x, jfloat y) {
  auto view = reinterpret_cast<moui::View*>(moui_view_ptr);
  return view->ShouldHandleEvent({x, y});
}

}  // extern "C"
