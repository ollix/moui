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

#ifndef MOUI_UI_BASE_VIEW_H_
#define MOUI_UI_BASE_VIEW_H_

#include <memory>
#include <string>

#include "moui/base.h"
#include "moui/core/event.h"
#include "moui/opengl_hook.h"
#include "moui/ui/native_view.h"

namespace moui {

// This is the base class for the View class and should never be instantiated
// directly. Unlike the View class, which is deisgned to implmenet functions
// for a specific paltform, this base class implement functions with
// corss-platform support.
class BaseView : public NativeView {
 public:
  BaseView();
  ~BaseView();

  // This method gets called when the view received an event. To receive events,
  // the ShouldHandleEvent() method should be overriden to return true.
  virtual void HandleEvent(std::unique_ptr<Event> event) {}

  // Returns true if the view is animating, that is, the view is keeping
  // updated continuously on the refresh rate of the display.
  bool IsAnimating() const;

  // Redraws the view on the next display refresh.
  virtual void Redraw() {};

  // The place for writing rendering code.
  virtual void Render() {}

  // Starts updating the view synchronized to the refresh rate of the display
  // continuously. StopAnimation() must be called for each StartAnimation().
  void StartAnimation();

  // Ends previous StartAnimation() call. The animation will actually stop if
  // all StartAnimation() calls are ended.
  void StopAnimation();

  // This method gets called when an event is about to occur. The returned
  // boolean indicates whether the view should handle the event. By default
  // it returns false so it simply ignores any coming event and the platform
  // will pass the event to the next responder. This method could be
  // implemented in the subclass to change the default behavior.
  virtual bool ShouldHandleEvent(const Point location) { return false; }

 protected:
  // Compiles the shader string of the specified type. Returns the shader
  // handle on success or 0 on failure.
  GLuint CompileShader(const GLenum shader_type,
                       const std::string& source) const;

  // Compiles the shader file of the specified type. Returns the shader handle
  // on success or 0 on failure.
  GLuint CompileShaderAtPath(const GLenum shader_type,
                             const std::string& source_path) const;

 private:
  // This is a bridge method for calling the corresponded function implmeneted
  // in native OpenGL view. This method is implmeneted in the View subclass
  // and the actual native implmenetation starts updating the native view
  // synchronized to the refresh rate of the display continuously.
   virtual void StartUpdatingNativeView() {}

  // This is a bridge method for calling the corresponded function implmeneted
  // in native OpenGL view. This method is implmeneted in the View subclass
  // and the actual native implmenetation requests stoping native view updates.
  virtual void StopUpdatingNativeView() {}

  // The number of animation requests. The default value is 0. StartAnimation()
  // increases the value and StopAnimation() decreases the value. The view is
  // animating if this value is greater than 0.
  int animation_count_;

  DISALLOW_COPY_AND_ASSIGN(BaseView);
};

}  // namespace moui

#endif  // MOUI_UI_BASE_VIEW_H_
