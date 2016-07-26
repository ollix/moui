// Copyright (c) 2014 Ollix. All rights reserved.
//
// ---
// Author: olliwang@ollix.com (Olli Wang)

#ifndef MOUI_UI_BASE_WINDOW_H_
#define MOUI_UI_BASE_WINDOW_H_

#include "moui/base.h"
#include "moui/native/native_window.h"

namespace moui {

// The `BaseWindow` class is the base class of the `Window` class. This class
// should only be inherited by the `Window` class and never instantiated
// directly. The purpose of this class is to provide a way for registering and
// retrieving the main window.
class BaseWindow : public NativeWindow {
 public:
  explicit BaseWindow(void* native_handle);
  ~BaseWindow();

  // Registers the specified native handle that can later be retrieved
  // throguh the `GetMainNativeHandle()` method.
  static void RegisterMainNativeHandle(void* native_handle);

 protected:
  // Returns the native handle previously registered through the
  // `RegisterMainNativeHandle()` method.
  static void* GetMainNativeHandle();

 private:
  DISALLOW_COPY_AND_ASSIGN(BaseWindow);
};

}  // namespace moui

#endif  // MOUI_UI_BASE_WINDOW_H_
