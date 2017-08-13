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

#include "moui/defines.h"

#ifdef MOUI_GL

#import <QuartzCore/QuartzCore.h>

#if defined(MOUI_GLES2)
#  include <OpenGLES/ES2/gl.h>
#  include <OpenGLES/ES2/glext.h>
#elif defined(MOUI_GLES3)
#  include <OpenGLES/ES3/gl.h>
#  include <OpenGLES/ES3/glext.h>
#endif

#if defined MOUI_GLES2
#  define EAGL_RENDERING_API_OPENGLES kEAGLRenderingAPIOpenGLES2
#  define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#elif defined MOUI_GLES3
#  define EAGL_RENDERING_API_OPENGLES kEAGLRenderingAPIOpenGLES3
#endif

@implementation MOOpenGLView {
 @private
  GLuint _colorRenderbuffer;
  EAGLContext* _context;
  GLuint _framebuffer;
  GLuint _stencilAndDepthRenderbuffer;
}

+ (Class)layerClass {
  return [CAEAGLLayer class];
}

- (void)createDrawableWithSize:(CGSize)size {
  if (_context == nil)
    _context = [[EAGLContext alloc] initWithAPI:EAGL_RENDERING_API_OPENGLES];

  [EAGLContext setCurrentContext:_context];

  glGenFramebuffers(1, &_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

  // Creates the color render buffer.
  glGenRenderbuffers(1, &_colorRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
  [_context renderbufferStorage:GL_RENDERBUFFER
                   fromDrawable:(CAEAGLLayer *)self.layer];
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, _colorRenderbuffer);

  // Creates both stencil and depth render buffers.
  glGenRenderbuffers(1, &_stencilAndDepthRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.width,
                        size.height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, _stencilAndDepthRenderbuffer);
}

- (void)destroyDrawable {
  if (_colorRenderbuffer != 0) {
    glDeleteRenderbuffers(1, &_colorRenderbuffer);
    _colorRenderbuffer = 0;
  }
  if (_stencilAndDepthRenderbuffer != 0) {
    glDeleteFramebuffers(1, &_stencilAndDepthRenderbuffer);
    _stencilAndDepthRenderbuffer = 0;
  }
  if (_framebuffer != 0) {
    glDeleteFramebuffers(1, &_framebuffer);
    _framebuffer = 0;
  }
}

- (void)presentDrawable {
  glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
  [_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)prepareDrawable {
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
}

@end

#endif  // MOUI_GL
