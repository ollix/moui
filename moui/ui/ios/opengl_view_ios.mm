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

#import "moui/ui/ios/opengl_view_ios.h"

#include <vector>
#import <QuartzCore/QuartzCore.h>

#include "moui/core/event.h"
#include "moui/ui/view.h"
#include "moui/widgets/widget.h"

#if defined MOUI_GLES2
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif defined MOUI_GLES3
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#endif

#if defined MOUI_GLES2
#  define EAGL_RENDERING_API_OPENGLES kEAGLRenderingAPIOpenGLES2
#  define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#elif defined MOUI_GLES3
#  define EAGL_RENDERING_API_OPENGLES kEAGLRenderingAPIOpenGLES3
#endif


@interface MOOpenGLView (PrivateDelegateHandling)

- (void)applicationDidBecomeActive;
- (void)applicationWillResignActive;
- (void)handleEvent:(UIEvent *)event withType:(moui::Event::Type)type;
- (void)handleMemoryWarning:(NSNotification *)notification;
- (void)setupFramebuffer;

@end

@implementation MOOpenGLView (PrivateDelegateHandling)

// Resumes view updates.
- (void)applicationDidBecomeActive {
  _isActive = YES;
  if (!_stopsUpdatingView)
    [self startUpdatingView];
}

// Unregisters the display link to stops updating the view.
- (void)applicationWillResignActive {
  _isActive = NO;
  [_displayLink invalidate];
  _displayLink = nil;
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

- (void)setupFramebuffer {
  [EAGLContext setCurrentContext:_context];

  glGenFramebuffers(1, &_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

  // Creates the color render buffer.
  glGenRenderbuffers(1, &_colorRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, _colorRenderbuffer);

  // Creates both stencil and depth render buffers.
  glGenRenderbuffers(1, &_stencilAndDepthRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
}

@end

@implementation MOOpenGLView

+ (Class)layerClass {
  return [CAEAGLLayer class];
}

- (id)initWithMouiView:(moui::View *)mouiView {
  if((self = [super initWithFrame:CGRectMake(0, 0, 0, 0)])) {
    _colorRenderbuffer = 0;
    _context = [[EAGLContext alloc] initWithAPI:EAGL_RENDERING_API_OPENGLES];
    _framebuffer = 0;
    _isActive = YES;
    _isHandlingEvents = NO;
    _mouiView = mouiView;
    _needsRedraw = NO;
    _stencilAndDepthRenderbuffer = 0;
    _stopsUpdatingView = YES;

    // Initializes the CAEAGLLayer.
    CAEAGLLayer* eaglLayer = (CAEAGLLayer *)self.layer;
    eaglLayer.opaque = NO;

    self.contentScaleFactor= [UIScreen mainScreen].scale;
    [self setupFramebuffer];

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
  if (_colorRenderbuffer != 0)
    glDeleteRenderbuffers(1, &_colorRenderbuffer);
  if (_stencilAndDepthRenderbuffer != 0)
    glDeleteFramebuffers(1, &_stencilAndDepthRenderbuffer);
  if (_framebuffer != 0)
    glDeleteFramebuffers(1, &_framebuffer);
  if (_displayLink != nil)
    [_displayLink invalidate];
  [_context release];

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
    if (!_stopsUpdatingView && _needsRedraw)
      return;

    _needsRedraw = YES;
    // Attaches `_displayLink` to the run loop if it's invalidated.
    if (_stopsUpdatingView) {
      [self startUpdatingView];
      [self stopUpdatingView];
    }
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

  // Binds the framebuffer and renderbuffers.
  [EAGLContext setCurrentContext:_context];
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                        self.frame.size.width * self.contentScaleFactor,
                        self.frame.size.height * self.contentScaleFactor);
  glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
  [_context renderbufferStorage:GL_RENDERBUFFER
                   fromDrawable:(CAEAGLLayer *)self.layer];
  // Rendering.
  _mouiView->Render();
  // Displays the renderbuffer's contents on screen.
  [_context presentRenderbuffer:GL_RENDERBUFFER];

  // Removes the display link from the run loop if no need to update the view
  // continuously.
  @synchronized(self) {
    if (_stopsUpdatingView && !_needsRedraw) {
      [_displayLink invalidate];  // this also releases `_displayLink`
      _displayLink = nil;
    } else {
      _needsRedraw = NO;
    }
  }
}

// Sets `_stopsUpdatingView` to NO to make sure `_displayLink` stays in the
// run loop and keep updating the view continuously. If the updating mechanism
// was not activated yet or stopped previously, a new `CADisplayLink` object
// will be created and added to the run loop.
- (void)startUpdatingView {
  @synchronized(self) {
    _stopsUpdatingView = NO;

    // Registers the display link only if the app is active. Or the display
    // link will be registered automatically when the app becomes active again.
    if (!_isActive || _displayLink != nil)
      return;

    _displayLink = [CADisplayLink displayLinkWithTarget:self
                                                selector:@selector(render)];
    [_displayLink addToRunLoop:[NSRunLoop mainRunLoop]
                       forMode:NSRunLoopCommonModes];
  }
}

// Simply sets `_stopsUpdatingView` to YES to stop updating when the latest
// rendering is done.
- (void)stopUpdatingView {
  @synchronized(self) {
    _stopsUpdatingView = YES;
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
