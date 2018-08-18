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

NativeView::NativeView(void* native_handle, const bool releases_on_demand)
    : NativeObject(native_handle, releases_on_demand) {
}

NativeView::NativeView(void* native_handle) : NativeView(native_handle, false) {
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

void NativeView::BringSubviewToFront(const NativeView* subview) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  UIView* native_subview = (__bridge UIView*)subview->native_handle();
  [native_view bringSubviewToFront:native_subview];
}

float NativeView::GetAlpha() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return native_view.alpha;
}

float NativeView::GetHeight() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return native_view.frame.size.height;
}

void* NativeView::GetLayer() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return (__bridge void*)native_view.layer;
}

unsigned char* NativeView::GetSnapshot() const {
  const float kScreenScaleFactor = Device::GetScreenScaleFactor();
  const int kWidth = GetWidth();
  const int kHeight = GetHeight();
  const int kScaledWidth = kWidth * kScreenScaleFactor;
  const int kScaledHeight = kHeight * kScreenScaleFactor;
  const int kBytesPerRow = kScaledWidth * 4;
  unsigned char* image = reinterpret_cast<unsigned char*>(
      std::malloc(kBytesPerRow * kScaledHeight));
  CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(
      image,
      kScaledWidth,
      kScaledHeight,
      8,  // bits per component
      kBytesPerRow,  // bytes per row
      color_space,
      kCGBitmapAlphaInfoMask & kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(color_space);
  UIView* native_view = (__bridge UIView*)native_handle();
  CGContextScaleCTM(context, kScreenScaleFactor, kScreenScaleFactor);
#ifdef MOUI_METAL
  CGAffineTransform flipVertical = CGAffineTransformMake(
      1, 0, 0, -1, 0, kHeight);
  CGContextConcatCTM(context, flipVertical);
#endif
  [native_view.layer renderInContext:context];
  CGContextRelease(context);
  return image;
}

NativeView* NativeView::GetSuperview() const {
  UIView* native_view = (__bridge UIView*)native_handle();
  return new NativeView((__bridge void*)native_view.superview);
}

float NativeView::GetWidth() const {
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

void NativeView::Reset() {
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

void NativeView::SetBounds(const float x, const float y, const float width,
                           const float height) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  native_view.frame = CGRectMake(x, y, width, height);
}

void NativeView::SetHidden(const bool hidden) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  native_view.hidden = hidden;
}

void NativeView::SetUserInteractionEnabled(const bool enabled) const {
  UIView* native_view = (__bridge UIView*)native_handle();
  native_view.userInteractionEnabled = enabled;
}

}  // namespace moui
