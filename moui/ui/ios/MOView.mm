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

#import "moui/ui/ios/MOView.h"

#import <QuartzCore/QuartzCore.h>

#include "moui/core/event.h"
#include "moui/ui/view.h"
#include "moui/widgets/widget.h"

@interface MOView (PrivateDelegateHandling)

// Resumes view updates.
- (void)applicationDidBecomeActive;

// Stops updating the view.
- (void)applicationWillResignActive;

// Dispatches received events to the corresponded moui view.
- (void)handleEvent:(UIEvent *)event withType:(moui::Event::Type)type;

// This method will get called when receiving the
// `UIApplicationDidReceiveMemoryWarningNotification`.
- (void)handleMemoryWarning:(NSNotification *)notification;

// Asks corresponded moui view to render. This method should never be called
// directly. Instead, calling `startUpdatingView` to execute this method
// automatically.
- (void)render;

@end

@implementation MOView (PrivateDelegateHandling)

- (void)applicationDidBecomeActive {
  _applicationIsActive = YES;
  if (_isUpdatingView)
    [self startUpdatingView];
}

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

- (void)handleMemoryWarning:(NSNotification *)notification {
  _mouiView->HandleMemoryWarning();
}

- (void)render {
  if (self.frame.size.width == 0 || self.frame.size.height == 0 ||
      self.isHidden ||
      [UIApplication sharedApplication].applicationState == \
          UIApplicationStateBackground) {
    return;
  }

  [self prepareDrawable];
  _mouiView->Render();
  [self presentDrawable];

  // Pauses display link if stopped updating view and no redraw request.
  @synchronized(self) {
    if (_needsRedraw)
      _needsRedraw = NO;
    else if (!_isUpdatingView)
      _displayLink.paused = YES;
  }
}

@end

@implementation MOView

- (id)initWithMouiView:(moui::View *)mouiView {
  if((self = [super initWithFrame:CGRectZero])) {
    _drawableSize = CGSizeMake(0, 0);
    _isHandlingEvents = NO;
    _isUpdatingView = NO;
    _mouiView = mouiView;
    _needsRedraw = NO;

    self.layer.opaque = NO;
    self.layer.contentsScale = [UIScreen mainScreen].scale;
    [self createDrawableWithSize:CGSizeMake(0, 0)];

    // Initializes the display link.
    _displayLink = [CADisplayLink displayLinkWithTarget:self
                                                selector:@selector(render)];
    [_displayLink addToRunLoop:[NSRunLoop mainRunLoop]
                       forMode:NSRunLoopCommonModes];

    // Registers notifications.
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
  [self destroyDrawable];
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

// Recreates the drawable whenever the size changes.
- (void)layoutSubviews {
  if (self.bounds.size.width != _drawableSize.width ||
      self.bounds.size.height != _drawableSize.height) {
    _drawableSize = self.bounds.size;
    [self destroyDrawable];
    [self createDrawableWithSize:
        CGSizeMake(_drawableSize.width * self.layer.contentsScale,
                   _drawableSize.height * self.layer.contentsScale)];
    [self render];
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

- (void)createDrawableWithSize:(CGSize)size {
  // Implmenets this method in subclass.
}

- (void)destroyDrawable {
  // Implmenets this method in subclass.
}

- (void)presentDrawable {
  // Implmenets this method in subclass.
}

- (void)prepareDrawable {
  // Implmenets this method in subclass.
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
