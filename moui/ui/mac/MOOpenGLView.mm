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

#import "moui/ui/mac/MOOpenGLView.h"

#include <OpenGL/gl.h>

#include "moui/ui/view.h"


@interface MOOpenGLView (PrivateDelegateHandling)

- (NSOpenGLContext *)openGLContext;

@end

@implementation MOOpenGLView (PrivateDelegateHandling)

- (NSOpenGLContext *)openGLContext {
  if (_openGLContext != nil)
    return _openGLContext;

  _openGLContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat
                                              shareContext:nil];
  [_openGLContext setView:self];

  // Sets synch to VBL to eliminate tearing
  GLint vblSync = 1;
  [_openGLContext setValues:&vblSync forParameter:NSOpenGLCPSwapInterval];
  // Allows for transparent background.
  GLint opaque = 0;
  [_openGLContext setValues:&opaque forParameter:NSOpenGLCPSurfaceOpacity];
}

@end

@implementation MOOpenGLView

- (id)initWithMouiView:(moui::View*)mouiView {
  if((self = [super initWithFrame:NSMakeRect(0, 0, 0, 0)])) {
    _mouiView = mouiView;

    const NSOpenGLPixelFormatAttribute attributes[] =  {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAWindow,
        (NSOpenGLPixelFormatAttribute)nil};
    _pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
  }
  return self;
}

- (void)dealloc {
  [_pixelFormat dealloc];
  if (_openGLContext != nil)
    [_openGLContext dealloc];
  [super dealloc];
}

- (void)drawRect:(NSRect)rect {
  [self render];
}

// Allows click through this view.
- (NSView *)hitTest:(NSPoint)aPoint {
  return nil;
}

- (void)lockFocus {
  [super lockFocus];
  [[self openGLContext] makeCurrentContext];
}

- (void)render {
  if (self.frame.size.width == 0 || self.frame.size.height == 0 ||
      [self isHidden])
    return;

  NSOpenGLContext* context = [self openGLContext];
  [context makeCurrentContext];
  _mouiView->Render();
  [context flushBuffer];
}

@end
