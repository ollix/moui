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

#include <string>

#include "moui/base.h"
#include "moui/opengl_hook.h"
#include "moui/ui/native_view.h"

namespace moui {

// The View class is a wrapper of the platform-specfic native view for
// rendering OpenGL stuff.
class BaseView : public NativeView {
 public:
  BaseView();
  ~BaseView();

  // Schedules a redraw event after the interval in seconds. This method must
  // be implemented in the View subclass and should always call this parent
  // method at the beginning of the overridden method.
  virtual void ScheduleRedraw(double interval) const;

 protected:
  // Compiles the shader string of the specified type. Returns the shader
  // handle on success or 0 on failure.
  GLuint CompileShader(const GLenum shader_type,
                       const std::string& source) const;

  // Compiles the shader file of the specified type. Returns the shader handle
  // on success or 0 on failure.
  GLuint CompileShaderAtPath(const GLenum shader_type,
                             const std::string& source_path) const;

  // Redraws the view. This method must be implmeneted in the View subclass
  // for performing native redraw action. The action should be performed on
  // the main thread that can modify view.
  virtual void Redraw() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(BaseView);
};

}  // namespace moui

#endif  // MOUI_UI_BASE_VIEW_H_
