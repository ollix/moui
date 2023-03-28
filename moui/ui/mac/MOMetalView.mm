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
  self.wantsLayer = YES;
}

- (CALayer *)makeBackingLayer {
  CAMetalLayer *metalLayer = [CAMetalLayer layer];
  metalLayer.allowsNextDrawableTimeout = YES;
  metalLayer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
  metalLayer.needsDisplayOnBoundsChange = YES;
  metalLayer.opaque = YES;
  metalLayer.presentsWithTransaction = NO;
  self.layerContentsPlacement =  NSViewLayerContentsPlacementTopLeft;
  self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
  return metalLayer;
}

- (void)presentMetalLayerWithTransaction:(bool)value {
  CAMetalLayer* layer = (CAMetalLayer*)self.layer;
  layer.presentsWithTransaction = value;
}

- (void)setBackgroundOpaque:(BOOL)isOpaque {
  self.layer.opaque = isOpaque;
  for (CALayer* layer in self.layer.sublayers) {
    layer.opaque = isOpaque;
  }
}

- (void)setFrameSize:(NSSize)newSize {
  [super setFrameSize:newSize];
  CAMetalLayer *metalLayer = (CAMetalLayer *)self.layer;
  metalLayer.drawableSize = [self convertSizeToBacking:newSize];;
}

@end

#endif  // MOUI_METAL
