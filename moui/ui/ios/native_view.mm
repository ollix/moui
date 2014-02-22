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

#include "moui/ui/native_view.h"

#import <UIKit/UIKit.h>

namespace moui {

NativeView::NativeView(void* native_handle) : native_handle_(native_handle) {
}

NativeView::~NativeView() {
}

void NativeView::AddSubview(const NativeView* subview) {
  UIView* native_view = (__bridge UIView*)native_handle_;
  UIView* native_subview = (__bridge UIView*)subview->native_handle();
  [native_view addSubview:native_subview];
}

float NativeView::GetContentScaleFactor() const {
  UIView* native_view = (__bridge UIView*)native_handle_;
  return native_view.contentScaleFactor;
}

int NativeView::GetHeight() const {
  UIView* native_view = (__bridge UIView*)native_handle_;
  return native_view.frame.size.height;
}

int NativeView::GetWidth() const {
  UIView* native_view = (__bridge UIView*)native_handle_;
  return native_view.frame.size.width;
}

void NativeView::SetBounds(const int x, const int y, const int width,
                           const int height) const {
  UIView* native_view = (__bridge UIView*)native_handle_;
  native_view.frame = CGRectMake(x, y, width, height);
}

}  // namespace moui
