// Copyright (c) 2014 Ollix. All rights reserved.
//
// ---
// Author: olliwang@ollix.com (Olli Wang)

#include "moui/ui/base_window.h"

#include "moui/native/native_window.h"

namespace {

void* main_native_handle = nullptr;

}  // namespace

namespace moui {

BaseWindow::BaseWindow(void* native_handle) : NativeWindow(native_handle) {
}

BaseWindow::~BaseWindow() {
}

void* BaseWindow::GetMainNativeHandle() {
  return main_native_handle;
}

void BaseWindow::RegisterMainNativeHandle(void* native_handle) {
  main_native_handle = native_handle;
}

}  // namespace moui
