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

#include <memory>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#import <QuartzCore/QuartzCore.h>

#include "moui/core/event.h"
#include "moui/ui/view.h"
#include "moui/widgets/widget.h"


@interface MOOpenGLView (PrivateDelegateHandling)

- (void)applicationDidBecomeActive;
- (void)applicationWillResignActive;
- (void)handleEvent:(UIEvent *)event withType:(moui::Event::Type)type;
- (void)setupFrameBuffer;

@end

@implementation MOOpenGLView (PrivateDelegateHandling)

// Resumes view updates.
- (void)applicationDidBecomeActive {
  if (!_stopsUpdatingView)
    [self startUpdatingView];
}

// Unregisters the display link to stops updating the view.
- (void)applicationWillResignActive {
  [_display_link invalidate];
  _display_link = nil;
}

- (void)handleEvent:(UIEvent *)event withType:(moui::Event::Type)type {
  auto mouiEvent = new moui::Event(type);
  auto locations = mouiEvent->locations();
  for (UITouch* touch in [event allTouches]) {
    CGPoint location = [touch locationInView:self];
    locations->push_back({static_cast<float>(location.x),
                          static_cast<float>(location.y)});
  }
  _mouiView->HandleEvent(std::unique_ptr<moui::Event>(mouiEvent));
}

- (void)setupFrameBuffer {
  // Creates frame buffer.
  if (_framebuffer == 0)
    glGenFramebuffers(1, &_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

  // Creates both stencil and depth render buffers.
  if (_stencilAndDepthRenderbuffer == 0)
    glGenRenderbuffers(1, &_stencilAndDepthRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES,
                        self.frame.size.width * self.contentScaleFactor,
                        self.frame.size.height * self.contentScaleFactor);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);

  // Creates the color render buffer.
  if (_colorRenderbuffer == 0)
    glGenRenderbuffers(1, &_colorRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, _colorRenderbuffer);

  [_eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:_eaglLayer];
}

@end

@implementation MOOpenGLView

@synthesize viewController = _viewController;

+ (Class)layerClass {
  return [CAEAGLLayer class];
}

- (id)initWithViewController:(MOOpenGLViewController *)viewController
                    mouiView:(moui::View *)mouiView {
  if((self = [super initWithFrame:CGRectMake(0, 0, 0, 0)])) {
    _colorRenderbuffer = 0;
    _eaglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    _framebuffer = 0;
    _mouiView = mouiView;
    _needsRedraw = NO;
    _stopsUpdatingView = YES;
    _viewController = viewController;

    // Initializes CAEAGLLayer.
    _eaglLayer = (CAEAGLLayer *)self.layer;
    _eaglLayer.delegate = self;
    _eaglLayer.opaque = NO;

    // Makes sure the rendering resolution is matched to the screen.
    self.contentScaleFactor = [UIScreen mainScreen].scale;

    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(applicationDidBecomeActive)
        name:UIApplicationDidBecomeActiveNotification
        object:[UIApplication sharedApplication]];
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(applicationWillResignActive)
        name:UIApplicationWillResignActiveNotification
        object:[UIApplication sharedApplication]];
  }
  return self;
}

- (void)dealloc {
  if (_colorRenderbuffer != 0)
    glDeleteRenderbuffers(1, &_colorRenderbuffer);
  if (_framebuffer != 0)
    glDeleteFramebuffers(1, &_framebuffer);

  [_display_link dealloc];
  [_eaglContext dealloc];
  [super dealloc];
}

// Triggered by `setNeedsDisplay` from _mouiView's Redraw(). This method
// guarantees the view will be updated in the next refresh cycle of the display.
- (void)displayLayer:(CALayer *)layer {
  @synchronized(self) {
    if (_needsRedraw)
      return;

    _needsRedraw = YES;
    // Attaches `_display_link` to the run loop if it's invalidated.
    if (_stopsUpdatingView) {
      [self startUpdatingView];
      [self stopUpdatingView];
    }
  }
}

// If this method returns NO, the event will be passed to the next responder.
-(BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event {
  return _mouiView->ShouldHandleEvent({static_cast<float>(point.x),
                                       static_cast<float>(point.y)});
}

- (void)render {
  if (self.frame.size.width == 0 || self.frame.size.height == 0 ||
      self.isHidden)
    return;

  [EAGLContext setCurrentContext:_eaglContext];
  [self setupFrameBuffer];
  _mouiView->Render();
  [_eaglContext presentRenderbuffer:GL_RENDERBUFFER];
  [EAGLContext setCurrentContext:nil];

  // Removes the display link from the run loop if no need to update the view
  // continuously.
  @synchronized(self) {
    if (_stopsUpdatingView && !_needsRedraw) {
      [_display_link invalidate];  // this also releases `_display_link`
      _display_link = nil;
    } else {
      _needsRedraw = NO;
    }
  }
}

// Sets `_stopsUpdatingView` to NO to make sure `_display_link` stays in the
// run loop and keep updating the view continuously. If the updating mechanism
// was not activated yet or stopped previously, a new CADisplayLink object will
// be created and added to the run loop.
- (void)startUpdatingView {
  @synchronized(self) {
    _stopsUpdatingView = NO;

    // Registers the display link only if the app is active. Or the display
    // link will be registered automatically when the app becomes active again.
    UIApplication* application = [UIApplication sharedApplication];
    if ([application applicationState] != UIApplicationStateActive ||
        _display_link != nil)
      return;

    _display_link = [CADisplayLink displayLinkWithTarget:self
                                                selector:@selector(render)];
    [_display_link addToRunLoop:[NSRunLoop mainRunLoop]
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
  [self handleEvent:event withType:moui::Event::Type::kDown];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kCancel];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kUp];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  [self handleEvent:event withType:moui::Event::Type::kMove];
}

@end
