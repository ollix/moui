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

#import <Cocoa/Cocoa.h>

#include "moui/core/device.h"
#include "moui/native/native_object.h"

namespace {

// The comparator function for `NativeView::BringSubviewToFront()`.
static NSComparisonResult BringSubviewToFrontComparator(__kindof NSView* view1,
                                                        __kindof NSView* view2,
                                                        void* context) {
  NSView* subview = (__bridge NSView*)context;
  return view1 == subview ? NSOrderedDescending : NSOrderedAscending;
}

// The comparator function for `NativeView::SendSubviewToBack()`.
static NSComparisonResult SendSubviewToBackComparator(__kindof NSView* view1,
                                                      __kindof NSView* view2,
                                                      void* context) {
  NSView* subview = (__bridge NSView*)context;
  return view1 == subview ?  NSOrderedAscending : NSOrderedDescending;
}

}  // namespace

namespace moui {

NativeView::NativeView(void* native_handle, const bool releases_on_demand)
    : NativeObject(native_handle, releases_on_demand) {
}

NativeView::NativeView(void* native_handle) : NativeView(native_handle, false) {
}

NativeView::NativeView() : NativeView(nullptr) {
  NSView* view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
  SetNativeHandle((__bridge_retained void*)view, true);
}

NativeView::~NativeView() {
}

void NativeView::AddSubview(const NativeView* subview) const {
  NSView* native_view = (__bridge NSView*)native_handle();
  NSView* native_subview = (__bridge NSView*)subview->native_handle();
  [native_view addSubview:native_subview];
}

bool NativeView::BecomeFirstResponder() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  return [native_view becomeFirstResponder];
}

void NativeView::BringSubviewToFront(const NativeView* subview) const {
  NSView* native_view = (__bridge NSView*)native_handle();
  [native_view sortSubviewsUsingFunction:BringSubviewToFrontComparator
                                 context:subview->native_handle()];
}

float NativeView::GetAlpha() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  return native_view.alphaValue;
}

int NativeView::GetHeight() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  return native_view.frame.size.height;
}

void* NativeView::GetLayer() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  return (__bridge void*)native_view.layer;
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
  NSView* native_view = (__bridge NSView*)native_handle();
  CGContextScaleCTM(context, kScreenScaleFactor, kScreenScaleFactor);
  [native_view.layer renderInContext:context];
  CGContextRelease(context);
  return image;
}

NativeView* NativeView::GetSuperview() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  return new NativeView((__bridge void*)native_view.superview);
}

int NativeView::GetWidth() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  return native_view.frame.size.width;
}

bool NativeView::IsHidden() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  return native_view.hidden;
}

void NativeView::RemoveFromSuperview() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  [native_view removeFromSuperview];
}

void NativeView::ResignFirstResponder() const {
  NSView* native_view = (__bridge NSView*)native_handle();
  [native_view resignFirstResponder];
}

void NativeView::SendSubviewToBack(const NativeView* subview) const {
  NSView* native_view = (__bridge NSView*)native_handle();
  [native_view sortSubviewsUsingFunction:SendSubviewToBackComparator
                                 context:subview->native_handle()];
}

void NativeView::SetAlpha(const float alpha) const {
  NSView* native_view = (__bridge NSView*)native_handle();
  native_view.alphaValue = alpha;
}

void NativeView::SetBounds(const int x, const int y, const int width,
                           const int height) const {
  NSView* native_view = (__bridge NSView*)native_handle();
  native_view.frame = CGRectMake(x, y, width, height);
}

bool NativeView::SetHidden(const bool hidden) const {
  NSView* native_view = (__bridge NSView*)native_handle();
  native_view.hidden = hidden;
}

}  // namespace moui
