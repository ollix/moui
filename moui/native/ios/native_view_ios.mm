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

#include "moui/native/native_view.h"

#include <cstdlib>

#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>

#include "moui/core/device.h"
#include "moui/native/native_object.h"

namespace moui {

NativeView::NativeView(void* native_handle) : NativeObject(native_handle) {
}

NativeView::NativeView() : NativeView(nullptr) {
  UIView* view = [[UIView alloc] initWithFrame:CGRectZero];
  SetNativeHandle((__bridge_retained void*)view, true);
}

NativeView::~NativeView() {
}

void NativeView::AddSubview(const NativeView* subview) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  UIView* native_subview = (__bridge UIView*)subview->native_handle();
  [native_view addSubview:native_subview];
}

bool NativeView::BecomeFirstResponder() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return [native_view becomeFirstResponder];
}

void NativeView::BringSubviewToFront(const NativeView* view) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  [native_view bringSubviewToFront:native_view];
}

float NativeView::GetAlpha() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return native_view.alpha;
}

int NativeView::GetHeight() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return native_view.frame.size.height;
}

unsigned char* NativeView::GetSnapshot() const {
  const float kScreenScaleFactor = Device::GetScreenScaleFactor();
  const int kWidth = GetWidth() * kScreenScaleFactor;
  const int kHeight = GetHeight() * kScreenScaleFactor;
  const int kBytesPerRow = kWidth * 4;
  unsigned char* image = \
      reinterpret_cast<unsigned char*>(std::malloc(kBytesPerRow * kHeight));
  CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(
      image,
      kWidth,
      kHeight,
      8,  // bits per component
      kBytesPerRow,  // bytes per row
      color_space,
      kCGBitmapAlphaInfoMask & kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(color_space);
  UIView* native_view = (__bridge UIView*)native_handle();
  CGContextScaleCTM(context, kScreenScaleFactor, kScreenScaleFactor);
  [native_view.layer renderInContext:context];
  CGContextRelease(context);
  return image;
}

NativeView* NativeView::GetSuperview() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return new NativeView((__bridge void*)native_view.superview);
}

int NativeView::GetWidth() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return native_view.frame.size.width;
}

bool NativeView::IsHidden() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return native_view.hidden;
}

void NativeView::RemoveFromSuperview() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  [native_view removeFromSuperview];
}

void NativeView::ResignFirstResponder() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  [native_view resignFirstResponder];
}

void NativeView::SendSubviewToBack(const NativeView* subview) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  UIView* native_subview = (__bridge UIView*)subview->native_handle();
  [native_view sendSubviewToBack:native_subview];
}

void NativeView::SetAlpha(const float alpha) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  native_view.alpha = alpha;
}

void NativeView::SetBounds(const int x, const int y, const int width,
                           const int height) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  native_view.frame = CGRectMake(x, y, width, height);
}

bool NativeView::SetHidden(const bool hidden) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  native_view.hidden = hidden;
}

void NativeView::SetUserInteractionEnabled(const bool enabled) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  native_view.userInteractionEnabled = enabled;
}

}  // namespace moui
