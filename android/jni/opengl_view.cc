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

#include <cassert>
#include <memory>

#include "jni.h"

#include "moui/moui.h"

extern "C" {

// Handles the event in moui view.
JNIEXPORT void JNICALL Java_com_ollix_moui_OpenGLView_handleEvent
    (JNIEnv* env, jobject obj, jlong moui_view_pointer, jint action, jfloat x,
     jfloat y) {
  // Converts the received MotionEvent action to moui event type.
  moui::Event::Type event_type;
  switch (action) {
    // MotionEvent.ACTION_CANCEL
    case 0x00000003: event_type = moui::Event::Type::kCancel; break;
    // MotionEvent.ACTION_DOWN
    case 0x00000000: event_type = moui::Event::Type::kDown; break;
    // MotionEvent.ACTION_MOVE
    case 0x00000002: event_type = moui::Event::Type::kMove; break;
    // MotionEvent.ACTION_UP
    case 0x00000001: event_type = moui::Event::Type::kUp; break;
    default: assert(false);
  }
  // Initializes the event object and asks moui view to handle the event.
  auto moui_event = new moui::Event(event_type);
  moui_event->locations()->push_back({static_cast<float>(x),
                                      static_cast<float>(y)});
  moui::View* moui_view = (moui::View*)moui_view_pointer;
  moui_view->HandleEvent(std::unique_ptr<moui::Event>(moui_event));
}

// Asks moui view if the event should be handled.
JNIEXPORT jboolean JNICALL Java_com_ollix_moui_OpenGLView_shouldHandleEvent
    (JNIEnv* env, jobject obj, jlong moui_view_pointer, jfloat x, jfloat y) {
  moui::View* view = (moui::View*)moui_view_pointer;
  return view->ShouldHandleEvent({x, y});
}

}  // extern "C"
