// Copyright (c) 2014 Ollix. All rights reserved.
//
// ---
// Author: olliwang@ollix.com (Olli Wang)

#include "moui/ui/base_window.h"

namespace {

moui::BaseWindow* main_window = nullptr;

}  // namespace

namespace moui {

BaseWindow::BaseWindow() {}

BaseWindow::~BaseWindow() {
  if (this == main_window)
    main_window = nullptr;
}

BaseWindow* BaseWindow::GetMainWindow() {
  return main_window;
}

void BaseWindow::RegisterMainWindow() {
  main_window = this;
}

}  // namespace moui
