// Copyright (c) 2014 Ollix. All rights reserved.
//
// ---
// Author: olliwang@ollix.com (Olli Wang)

#ifndef MOUI_UI_BASE_WINDOW_H_
#define MOUI_UI_BASE_WINDOW_H_

#include "moui/base.h"

namespace moui {

// The BaseWindow class is the base class of the Window class. This class
// should only be inherited by the Window class and never instantiated
// directly. The purpose of this class is to provide a way for registering and
// retrieving the main window.
class BaseWindow {
 public:
  BaseWindow();
  ~BaseWindow();

  // Returns the main window previously registered through the
  // RegisterMainWindow() method. This method should only be called in the
  // Window class.
  static BaseWindow* GetMainWindow();

  // Registers the instance as the main window that can later be retrieved
  // throguh the GetMainWindow() method.
  void RegisterMainWindow();

 private:
  DISALLOW_COPY_AND_ASSIGN(BaseWindow);
};

}  // namespace moui

#endif  // MOUI_UI_BASE_WINDOW_H_
