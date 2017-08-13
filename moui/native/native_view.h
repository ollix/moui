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

#ifndef MOUI_NATIVE_NATIVE_VIEW_H_
#define MOUI_NATIVE_NATIVE_VIEW_H_

#include "moui/base.h"
#include "moui/defines.h"
#include "moui/native/native_object.h"

namespace moui {

// The `NativeView` class behaves as a bridge to the platform-specific native
// view. In iOS, the native view would be `UIView` or its subclasses.
// In Android, the native view would be `android.view.View` or its subclasses.
class NativeView : public NativeObject {
 public:
  explicit NativeView(void* native_handle);
  NativeView();
  ~NativeView();

  // Adds a subview to the current view.
  void AddSubview(const NativeView* subview) const;

#ifdef MOUI_APPLE
  // Notifies that it is about to become first responder in its window.
  bool BecomeFirstResponder() const;
#endif

#ifdef MOUI_IOS
  // Moves the specified subview so that it appears on top of its siblings.
  void BringSubviewToFront(const NativeView* subview) const;

  // Returns the view's alpha value.
  float GetAlpha() const;
#endif

  // Returns the height of the view.
  int GetHeight() const;

#ifdef MOUI_APPLE
  // Returns the view's Core Animation layer used for rendering.
  void* GetLayer() const;

  // Returns the snapshot of the native view. The returned snapshot is a
  // bitmap data with the width and height matched to the screen sacle,
  // and each pixel is represented by 4 consective bytes in the RGBA format.
  unsigned char* GetSnapshot() const;

  // Returns the view that is the parent of the current view.
  NativeView* GetSuperview() const;
#endif

  // Returns the width of the view.
  int GetWidth() const;

#ifdef MOUI_APPLE
  // Returns `true` if the view is hidden.
  bool IsHidden() const;

  // Unlinks the view from its superview and its window, removes it from the
  // responder chain and invalidates its cursor rectangles.
  void RemoveFromSuperview() const;

  // Asks to relinquish the status as first responder in its window.
  void ResignFirstResponder() const;

  // Moves the specified subview so that it appears behind its siblings.
  void SendSubviewToBack(const NativeView* subview) const;
#endif

#ifdef MOUI_IOS
  // Sets the view's alpha value.
  void SetAlpha(const float alpha) const;
#endif

  // Sets the bounds of the view.
  void SetBounds(const int x, const int y, const int width,
                 const int height) const;

#ifdef MOUI_APPLE
  // Shows or hides the view.
  bool SetHidden(const bool hidden) const;
#endif

#ifdef MOUI_IOS
  // Sets whether user events are ignored and removed from the event queue.
  void SetUserInteractionEnabled(const bool enabled) const;
#endif

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeView);
};

}  // namespace moui

#endif  // MOUI_NATIVE_NATIVE_VIEW_H_
