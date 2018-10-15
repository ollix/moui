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

#import "moui/ui/mac/MOView.h"

#include "moui/ui/view.h"

@interface MOView (PrivateDelegateHandling)

// Converts NSView's point to moui's coordinate system.
- (NSPoint)convertToInternalPoint:(NSPoint)parentViewPoint;

// Dispatches received events to the corresponded moui view.
- (void)handleEvent:(NSEvent*)event withType:(moui::Event::Type)type;

// The callback function of `CVDisplayLink`.
static CVReturn renderCallback(CVDisplayLinkRef displayLink,
                               const CVTimeStamp* now,
                               const CVTimeStamp* outputTime,
                               CVOptionFlags flagsIn,
                               CVOptionFlags* flagsOut,
                               void *view);

// Asks corresponded moui view to render. This method should never be called
// directly. Instead, calling `startUpdatingView` to execute this method
// automatically.
- (void)render;

// Updates drawable if the view size is changed. Returns `YES` if drawable
// is updated.
- (BOOL)updateDrawableIfSizeChanged;

@end

@implementation MOView (PrivateDelegateHandling)

- (NSPoint)convertToInternalPoint:(NSPoint)parentViewPoint {
  NSPoint point;
  point.x = parentViewPoint.x - self.frame.origin.x,
  point.y = self.frame.size.height - (parentViewPoint.y - self.frame.origin.y);
  return point;
}

- (void)handleEvent:(NSEvent *)event withType:(moui::Event::Type)type {
  moui::Event mouiEvent(type);
  // Adds the event location.
  NSPoint locationInWindow = [event locationInWindow];
  NSPoint locationInView = [self convertPoint:locationInWindow fromView:self];
  NSPoint location = [self convertToInternalPoint:locationInView];
  mouiEvent.locations()->push_back({static_cast<float>(location.x),
                                    static_cast<float>(location.y)});
  _mouiView->HandleEvent(&mouiEvent);
}

// The callback function of MOView's `_displayLink` that marks the view's
// content as needing to be updated.
static CVReturn renderCallback(CVDisplayLinkRef displayLink,
                               const CVTimeStamp* now,
                               const CVTimeStamp* outputTime,
                               CVOptionFlags flagsIn,
                               CVOptionFlags* flagsOut,
                               void *view) {
  [(__bridge MOView *)view renderOnMainThread];
  return kCVReturnSuccess;
}

- (void)render {
  if ([self updateDrawableIfSizeChanged])
    return;

  if (_drawableSize.width == 0 || _drawableSize.height == 0 ||
      [self isHidden]) {
    return;
  }

  [self prepareDrawable];
  _mouiView->Render();
  [self presentDrawable];

  @synchronized(self) {
    if (_stopsUpdatingView && !_needsRedraw)
      CVDisplayLinkStop(_displayLink);
    else
      _needsRedraw = NO;
  }
}

- (void)renderOnMainThread {
  [self performSelectorOnMainThread:@selector(render)
                         withObject:nil
                      waitUntilDone:NO];
}

- (BOOL)updateDrawableIfSizeChanged {
  NSRect backingRect = [self convertRectToBacking:self.bounds];
  if (backingRect.size.width == _drawableSize.width &&
      backingRect.size.height == _drawableSize.height) {
    return NO;
  }

  _drawableSize = backingRect.size;
  _mouiView->SetBounds(0, 0, self.bounds.size.width, self.bounds.size.height);
  [self destroyDrawable];
  [self createDrawableWithSize:_drawableSize];
  [self render];
  return YES;
}

@end

@implementation MOView

- (id)initWithMouiView:(moui::View *)mouiView {
  if((self = [super initWithFrame:NSMakeRect(0, 0, 0, 0)])) {
    _mouiView = mouiView;
    _needsRedraw = NO;
    _stopsUpdatingView = YES;

    // Supports Retina resolution.
    self.wantsBestResolutionOpenGLSurface = YES;

    // Enables autoresizing.
    self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    // Initializes a drawable.
    [self createDrawableWithSize:NSMakeSize(0, 0)];

    // Creates a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);

    // Sets the renderer output callback function.
    CVDisplayLinkSetOutputCallback(_displayLink, renderCallback,
                                   (__bridge void*)self);

    // Observes view size changes.
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(updateDrawableIfSizeChanged)
        name:NSViewGlobalFrameDidChangeNotification
        object:self];
  }
  return self;
}

- (void)dealloc {
  CVDisplayLinkRelease(_displayLink);

  [[NSNotificationCenter defaultCenter] removeObserver:self
      name:NSViewGlobalFrameDidChangeNotification
      object:self];
}

- (void)drawRect:(NSRect)rect {
  [self render];
}

- (NSView *)hitTest:(NSPoint)parentViewPoint {
  NSPoint point = [self convertToInternalPoint:parentViewPoint];
  if (_mouiView->ShouldHandleEvent({static_cast<float>(point.x),
                                    static_cast<float>(point.y)}))
    return self;
  return nil;  // passes to the next responder
}

- (void)mouseDown:(NSEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kDown];
}

- (void)mouseDragged:(NSEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kMove];
}

- (void)mouseUp:(NSEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kUp];
}

- (void)setNeedsRedraw {
  @synchronized(self) {
    if (!_stopsUpdatingView && _needsRedraw)
      return;

    _needsRedraw = YES;
    // Rusumes `displayLink` if it's stopped.
    if (_stopsUpdatingView == YES) {
      [self startUpdatingView];
      [self stopUpdatingView];
    }
  }
}

- (void)createDrawableWithSize:(NSSize)size {
  // Implmenets this method in subclass.
}

- (void)destroyDrawable {
  // Implmenets this method in subclass.
}

- (moui::Point)mouseLocation {
  NSPoint mouseLocation = [NSEvent mouseLocation];
  NSRect localRect = [[self window]
      convertRectFromScreen:NSMakeRect(mouseLocation.x, mouseLocation.y, 0, 0)];
  NSPoint locationInView = [self convertPoint:localRect.origin fromView:self];
  mouseLocation = [self convertToInternalPoint:locationInView];
  return {static_cast<float>(mouseLocation.x),
          static_cast<float>(mouseLocation.y)};
}

- (void)presentDrawable {
  // Implmenets this method in subclass.
}

- (void)prepareDrawable {
  // Implmenets this method in subclass.
}

- (void)startUpdatingView {
  @synchronized(self) {
    _stopsUpdatingView = NO;
    CVDisplayLinkStart(_displayLink);
  }
}

- (void)stopUpdatingView {
  @synchronized(self) {
    _stopsUpdatingView = YES;
  }
}

@end
