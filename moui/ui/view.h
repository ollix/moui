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

#ifndef MOUI_UI_VIEW_H_
#define MOUI_UI_VIEW_H_

#include "moui/base.h"
#include "moui/ui/base_view.h"

namespace moui {

// The View class is a wrapper of the platform-specfic native view for
// rendering OpenGL stuff.
class View : public BaseView {
 public:
  View();
  ~View();

  // Returns the scale factor used for rendering.
  float GetContentScaleFactor() const;

  // Returns the height of the view.
  int GetHeight() const;

  // Returns the width of the view.
  int GetWidth() const;

  // The place for writing rendering code.
  virtual void Render() {};

  // Inherited from BaseView class.
  virtual void ScheduleRedraw(double interval) const override final;

 private:
  // Inherited from BaseView class.
  virtual void Redraw() const override final;

  DISALLOW_COPY_AND_ASSIGN(View);
};

}  // namespace moui

#endif  // MOUI_UI_VIEW_H_
