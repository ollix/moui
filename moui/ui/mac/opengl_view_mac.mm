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

#import "moui/ui/mac/opengl_view_mac.h"

#include <memory>
#include <OpenGL/gl.h>

#include "moui/ui/view.h"

namespace {

// The callback function of MOOpenGLView's `_displayLink` that marks the view's
// content as needing to be updated.
static CVReturn renderCallback(CVDisplayLinkRef displayLink,
                               const CVTimeStamp* now,
                               const CVTimeStamp* outputTime,
                               CVOptionFlags flagsIn,
                               CVOptionFlags* flagsOut,
                               void *view) {
  [(__bridge MOOpenGLView *)view setNeedsDisplay:YES];
  return kCVReturnSuccess;
}

}  // namespace

@interface MOOpenGLView (PrivateDelegateHandling)

- (NSPoint)convertToInternalPoint:(NSPoint)parentViewPoint;
- (void)handleEvent:(NSEvent*)event withType:(moui::Event::Type)type;
- (void)initOpenGLContext;

@end

@implementation MOOpenGLView (PrivateDelegateHandling)

- (NSPoint)convertToInternalPoint:(NSPoint)parentViewPoint {
  NSPoint point;
  point.x = parentViewPoint.x - self.frame.origin.x,
  point.y = self.frame.size.height - (parentViewPoint.y - self.frame.origin.y);
  return point;
}

- (void)handleEvent:(NSEvent *)event withType:(moui::Event::Type)type {
  auto mouiEvent = new moui::Event(type);
  // Adds the event location.
  NSPoint locationInWindow = [event locationInWindow];
  NSPoint locationInView = [self convertPoint:locationInWindow fromView:self];
  NSPoint location = [self convertToInternalPoint:locationInView];
  mouiEvent->locations()->push_back({static_cast<float>(location.x),
                                     static_cast<float>(location.y)});
  _mouiView->HandleEvent(std::unique_ptr<moui::Event>(mouiEvent));
}

- (void)initOpenGLContext {
  const NSOpenGLPixelFormatAttribute kAttributes[] =  {
      NSOpenGLPFAAccelerated,
      NSOpenGLPFAAlphaSize, 8,
      NSOpenGLPFAColorSize, 32,
      NSOpenGLPFADepthSize, 24,
      NSOpenGLPFADoubleBuffer,
      NSOpenGLPFANoRecovery,
      NSOpenGLPFAStencilSize, 8,
      (NSOpenGLPixelFormatAttribute)0};
  NSOpenGLPixelFormat* pixelFormat = [[[NSOpenGLPixelFormat alloc]
      initWithAttributes:kAttributes] autorelease];

  NSOpenGLContext* context = [[[NSOpenGLContext alloc]
      initWithFormat:pixelFormat
      shareContext:nil] autorelease];

  // Sets sync to VBL to eliminate tearing.
  GLint vblSync = 1;
  [context setValues:&vblSync forParameter:NSOpenGLCPSwapInterval];
  // Allows for transparent background.
  GLint opaque = 0;
  [context setValues:&opaque forParameter:NSOpenGLCPSurfaceOpacity];

  // Supports Retina resolution.
  self.wantsBestResolutionOpenGLSurface = YES;

  [self setPixelFormat:pixelFormat];
  [self setOpenGLContext:context];
}

@end

@implementation MOOpenGLView

- (id)initWithMouiView:(moui::View *)mouiView {
  if((self = [super initWithFrame:NSMakeRect(0, 0, 0, 0)])) {
    _mouiView = mouiView;
    _needsRedraw = NO;
    _stopsUpdatingView = YES;
    [self initOpenGLContext];
  }
  return self;
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

- (void)prepareOpenGL {
  [super prepareOpenGL];

  // Creates a display link capable of being used with all active displays
  CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);

  // Sets the renderer output callback function.
  CVDisplayLinkSetOutputCallback(_displayLink, renderCallback, self);

  // Sets the display link for the current renderer
  CGLContextObj cglContext = [self.openGLContext CGLContextObj];
  CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
  CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_displayLink, cglContext,
                                                    cglPixelFormat);
}

- (void)render {
  if (self.frame.size.width == 0 || self.frame.size.height == 0 ||
      [self isHidden])
    return;

  [self.openGLContext makeCurrentContext];
  CGLLockContext([self.openGLContext CGLContextObj]);

  _mouiView->Render();

  CGLFlushDrawable([self.openGLContext CGLContextObj]);
  CGLUnlockContext([self.openGLContext CGLContextObj]);

  @synchronized(self) {
    if (_stopsUpdatingView && !_needsRedraw)
      CVDisplayLinkStop(_displayLink);
    else
      _needsRedraw = NO;
  }
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
