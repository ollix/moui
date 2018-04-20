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

#import "moui/ui/mac/MOOpenGLView.h"

#include "moui/defines.h"

#ifdef MOUI_GL

#include <OpenGL/gl.h>

@implementation MOOpenGLView {
 @private
  NSOpenGLContext* _context;
  NSOpenGLPixelFormat* _pixelFormat;
}

- (BOOL)backgroundIsOpaque {
  GLint isOpaque;
  [_context getValues:&isOpaque forParameter:NSOpenGLCPSurfaceOpacity];
  return isOpaque == 0 ? NO : YES;
}

- (void)createDrawableWithSize:(NSSize)size {
  if (_context != nil) {
    [_context update];
    return;
  }

  const NSOpenGLPixelFormatAttribute kAttributes[] =  {
      NSOpenGLPFAAccelerated,
      NSOpenGLPFANoRecovery,
      NSOpenGLPFATripleBuffer,
      NSOpenGLPFAAlphaSize, 8,
      NSOpenGLPFAColorSize, 24,
      NSOpenGLPFADepthSize, 0,
      NSOpenGLPFAStencilSize, 8,
      (NSOpenGLPixelFormatAttribute)0};
  _pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:kAttributes];
  _context = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat
                                        shareContext:nil];

  // Sets sync to VBL to eliminate tearing.
  GLint vblSync = 1;
  [_context setValues:&vblSync forParameter:NSOpenGLCPSwapInterval];
  // Allows for transparent background.
  GLint opaque = 0;
  [_context setValues:&opaque forParameter:NSOpenGLCPSurfaceOpacity];

  [_context makeCurrentContext];
}

- (void)lockFocus {
  [super lockFocus];
  if ([_context view] != self) {
    [_context setView:self];
  }
  [_context makeCurrentContext];
}

- (void)presentDrawable {
  [_context flushBuffer];
  CGLUnlockContext([_context CGLContextObj]);
}

- (void)prepareDrawable {
  CGLLockContext([_context CGLContextObj]);
  [_context makeCurrentContext];
}

- (void)setBackgroundOpaque:(BOOL)isOpaque {
  GLint opaque = (GLint)isOpaque;
  [_context setValues:&opaque forParameter:NSOpenGLCPSurfaceOpacity];
}

@end

#endif  // MOUI_GL
