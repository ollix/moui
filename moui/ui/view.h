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

// The `View` class is a wrapper of the platform-specfic native view for
// rendering OpenGL stuff.
class View : public BaseView {
 public:
  View();
  ~View();

#ifdef MOUI_MAC
  moui::Point GetMouseLocation();
#endif

  // Returns `true` if the background is opaque.
  bool BackgroundIsOpaque() const;

  // This method is designed to be called whenever the application receives a
  // memory warning. Currently this method only works for iOS.
  virtual void HandleMemoryWarning() {}

  // This method is called immediately before the underlying graphics surface
  // is being destroyed.
  virtual void OnSurfaceDestroyed() {}

  // Inherited from `BaseView` class.
  void Redraw() override;

#ifdef MOUI_METAL
  void PresentMetalLayerWithTransaction(const bool value) const final;
#endif  // MOUI_METAL

  // Sets whether the view's background is opaque.
  void SetBackgroundOpaque(const bool is_opaque) const;

 private:
  // Inherited from `BaseView` class.
  void StartUpdatingNativeView() final;

  // Inherited from `BaseView` class.
  void StopUpdatingNativeView() final;

  DISALLOW_COPY_AND_ASSIGN(View);
};

}  // namespace moui

#endif  // MOUI_UI_VIEW_H_
