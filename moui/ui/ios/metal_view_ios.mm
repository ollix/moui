// Copyright (c) 2016 Ollix. All rights reserved.
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

#if defined MOUI_METAL

#import "moui/ui/ios/metal_view_ios.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include "bgfx/bgfx.h"
#include "bgfx/bgfxplatform.h"

#include "moui/core/event.h"
#include "moui/ui/view.h"
#include "moui/widgets/widget.h"

@interface MOMetalView (PrivateDelegateHandling)

- (void)applicationDidBecomeActive;
- (void)applicationWillResignActive;
- (void)handleEvent:(UIEvent *)event withType:(moui::Event::Type)type;
- (void)handleMemoryWarning:(NSNotification *)notification;

@end

@implementation MOMetalView (PrivateDelegateHandling)

// Resumes view updates.
- (void)applicationDidBecomeActive {
  _applicationIsActive = YES;
  if (_isUpdatingView)
    [self startUpdatingView];
}

// Stops updating the view.
- (void)applicationWillResignActive {
  _applicationIsActive = NO;
  _displayLink.paused = YES;
}

- (void)handleEvent:(UIEvent *)event withType:(moui::Event::Type)type {
  moui::Event mouiEvent(type);
  for (UITouch* nativeTouch in [event allTouches]) {
    CGPoint location = [nativeTouch locationInView:self];
    mouiEvent.locations()->push_back({static_cast<float>(location.x),
                                      static_cast<float>(location.y)});
  }
  _mouiView->HandleEvent(&mouiEvent);
}

// This method will get called when receiving the
// `UIApplicationDidReceiveMemoryWarningNotification`.
- (void)handleMemoryWarning:(NSNotification *)notification {
  _mouiView->HandleMemoryWarning();
}

@end

@implementation MOMetalView

+ (Class)layerClass {
  return [CAMetalLayer class];
}

- (id)initWithMouiView:(moui::View *)mouiView {
  if((self = [super initWithFrame:CGRectZero])) {
    _initializedBGFX = NO;
    _isHandlingEvents = NO;
    _isUpdatingView = NO;
    _mouiView = mouiView;
    _needsRedraw = NO;

    // Initializes the display link.
    _displayLink = [CADisplayLink displayLinkWithTarget:self
                                                selector:@selector(render)];
    [_displayLink addToRunLoop:[NSRunLoop mainRunLoop]
                       forMode:NSRunLoopCommonModes];
    _displayLink.paused = YES;

    CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
    metalLayer.opaque = NO;
    metalLayer.contentsScale = [UIScreen mainScreen].scale;

    bgfx::PlatformData platformData;
    platformData.ndt = NULL;
    platformData.nwh = metalLayer;
    platformData.context = MTLCreateSystemDefaultDevice();
    platformData.backBuffer = NULL;
    platformData.backBufferDS = NULL;
    bgfx::setPlatformData(platformData);

    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(applicationDidBecomeActive)
        name:UIApplicationDidBecomeActiveNotification
        object:[UIApplication sharedApplication]];
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(applicationWillResignActive)
        name:UIApplicationWillResignActiveNotification
        object:[UIApplication sharedApplication]];
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(handleMemoryWarning:)
        name:UIApplicationDidReceiveMemoryWarningNotification
        object:[UIApplication sharedApplication]];
  }
  return self;
}

- (void)dealloc {
  if (_initializedBGFX)
    bgfx::shutdown();
  [_displayLink invalidate];
  [[NSNotificationCenter defaultCenter] removeObserver:self
      name:UIApplicationDidBecomeActiveNotification
      object:[UIApplication sharedApplication]];
  [[NSNotificationCenter defaultCenter] removeObserver:self
      name:UIApplicationWillResignActiveNotification
      object:[UIApplication sharedApplication]];
  [[NSNotificationCenter defaultCenter] removeObserver:self
      name:UIApplicationDidReceiveMemoryWarningNotification
      object:[UIApplication sharedApplication]];

  [super dealloc];
}

// Triggered by `setNeedsDisplay` from _mouiView's Redraw(). This method
// guarantees the view will be updated in the next refresh cycle of the display.
- (void)displayLayer:(CALayer *)layer {
  @synchronized(self) {
    _needsRedraw = YES;
    if (_isUpdatingView)
      return;
    [self startUpdatingView];
    [self stopUpdatingView];
  }
}

// If this method returns `NO`, the event will be passed to the next responder.
-(BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event {
  if (![super pointInside:point withEvent:event])
    return NO;

  // Caches the result of last call to `ShouldHandleEvent()`.
  static int result;
  // Skips if the event's timestamp is not newer than the last processed one.
  static NSTimeInterval lastEventTimestamp = -1;
  if (event.timestamp <= lastEventTimestamp)
    return result;
  lastEventTimestamp = event.timestamp;

  // Prevents from calling the `ShouldHandleEvent` method multiple times.
  if (_isHandlingEvents)
    return YES;

  result = _mouiView->ShouldHandleEvent({static_cast<float>(point.x),
                                         static_cast<float>(point.y)});
  return result;
}

- (void)render {
  if (self.frame.size.width == 0 || self.frame.size.height == 0 ||
      self.isHidden ||
      [UIApplication sharedApplication].applicationState == \
          UIApplicationStateBackground) {
    return;
  }

  if (!_initializedBGFX) {
    bgfx::renderFrame();
    _initializedBGFX = bgfx::init(bgfx::RendererType::Metal, BGFX_PCI_ID_NONE);
    if (!_initializedBGFX)
      return;

    bgfx::reset(self.bounds.size.width * self.layer.contentsScale,
                self.bounds.size.height * self.layer.contentsScale,
                BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
  	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000,
                       1.0f, 0);
    bgfx::setViewSeq(0, true);
  }
  // Rendering.
  _mouiView->Render();
  bgfx::frame();

  // Pauses display link if stopped updating view and no redraw request.
  @synchronized(self) {
    if (_needsRedraw)
      _needsRedraw = NO;
    else if (!_isUpdatingView)
      _displayLink.paused = YES;
  }
}

- (void)startUpdatingView {
  @synchronized(self) {
    _isUpdatingView = YES;
    if (_applicationIsActive)
      _displayLink.paused = NO;
  }
}

- (void)stopUpdatingView {
  @synchronized(self) {
    _isUpdatingView = NO;
  }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  _isHandlingEvents = YES;
  [self handleEvent:event withType:moui::Event::Type::kDown];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kCancel];
  _isHandlingEvents = NO;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kUp];
  _isHandlingEvents = NO;
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kMove];
}

@end

#endif
