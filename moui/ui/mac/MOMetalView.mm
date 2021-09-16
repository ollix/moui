// Copyright (c) 2017 Ollix. All rights reserved.
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

#ifdef MOUI_METAL

#import "moui/ui/mac/MOMetalView.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

@implementation MOMetalView

- (BOOL)backgroundIsOpaque {
  return self.layer.opaque;
}

- (void)createDrawableWithSize:(NSSize)size {
  if (self.layer == nil) {
    NSScreen* screen = [NSScreen mainScreen];
    self.layer = [CAMetalLayer new];
    self.wantsLayer = YES;
    self.layer.backgroundColor = [NSColor clearColor].CGColor;
    self.layer.contentsScale = screen.backingScaleFactor;

    // Creates the `CAMetalLayer` as a sublayer with large drawable size to
    // avoid unwanted effect while resizing the drawable. Learn more about
    // this issue at https://goo.gl/bXV9s9
    CAMetalLayer* metalLayer = [CAMetalLayer new];
    metalLayer.contentsScale = screen.backingScaleFactor;
    metalLayer.autoresizingMask = kCALayerMinYMargin;
    metalLayer.backgroundColor = self.layer.backgroundColor;
    metalLayer.bounds = screen.frame;
    metalLayer.drawableSize = CGSizeMake(
        metalLayer.bounds.size.width * screen.backingScaleFactor,
        metalLayer.bounds.size.height * screen.backingScaleFactor);
    metalLayer.opaque = self.layer.opaque;
    metalLayer.presentsWithTransaction = NO;
    CGFloat y = self.frame.size.height - CGRectGetHeight(metalLayer.bounds);
    metalLayer.frame = CGRectMake(0, y, CGRectGetWidth(metalLayer.bounds),
                                  CGRectGetHeight(metalLayer.bounds));
    [self.layer addSublayer:metalLayer];
  }
}

- (void)setBackgroundOpaque:(BOOL)isOpaque {
  self.layer.opaque = isOpaque;
  for (CALayer* layer in self.layer.sublayers) {
    layer.opaque = isOpaque;
  }
}

@end

#endif  // MOUI_METAL
