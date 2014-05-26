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

#import "moui/ui/ios/MOOpenGLView.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "moui/ui/view.h"


@interface MOOpenGLView (PrivateDelegateHandling)

- (void)setupFrameBuffer;

@end

@implementation MOOpenGLView (PrivateDelegateHandling)

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

- (id)initWithViewController:(MOOpenGLViewController*)viewController
                    mouiView:(moui::View*)mouiView {
  if((self = [super initWithFrame:CGRectMake(0, 0, 0, 0)])) {
    _colorRenderbuffer = 0;
    _eaglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    _framebuffer = 0;
    _mouiView = mouiView;
    _viewController = viewController;

    // Initializes CAEAGLLayer.
    _eaglLayer = (CAEAGLLayer*)self.layer;
    _eaglLayer.opaque = NO;

    // Makes sure the rendering resolution is matched to the screen.
    self.contentScaleFactor = [UIScreen mainScreen].scale;
  }
  return self;
}

- (void)dealloc {
  if (_colorRenderbuffer != 0)
    glDeleteRenderbuffers(1, &_colorRenderbuffer);
  if (_framebuffer != 0)
    glDeleteFramebuffers(1, &_framebuffer);

  [_eaglContext dealloc];
  [super dealloc];
}

// Allows click through this view.
-(BOOL)pointInside:(CGPoint)point withEvent:(UIEvent*)event {
  return NO;
}

- (void)render {
  if (self.frame.size.width == 0 || self.frame.size.height == 0 ||
      self.isHidden)
    return;

  [EAGLContext setCurrentContext:_eaglContext];
  [self setupFrameBuffer];
  glViewport(0, 0,
             self.frame.size.width * self.contentScaleFactor,
             self.frame.size.height * self.contentScaleFactor);
  _mouiView->Render();
  [_eaglContext presentRenderbuffer:GL_RENDERBUFFER];
  [EAGLContext setCurrentContext:nil];
}

@end
