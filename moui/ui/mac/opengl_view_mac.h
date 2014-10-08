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

#import <Cocoa/Cocoa.h>

namespace moui {
class View;
}  // namespace moui

// The native iOS view for rendering OpenGL stuff.
@interface MOOpenGLView : NSView {
 @private
  CVDisplayLinkRef _displayLink;
  moui::View* _mouiView;
  NSOpenGLContext* _openGLContext;
  NSOpenGLPixelFormat* _pixelFormat;
  BOOL _stopsUpdatingView;
}

- (id)initWithMouiView:(moui::View*)mouiView;

// Sets up the OpenGL context and asks corresponded moui view to render.
// This method should never be called directly. Instead, calling
// `startUpdatingView` to execute this method automatically.
- (void)render;

// Starts updating the view synchronized to the refresh rate of the display.
// The view will be updated continuously until `stopUpdatingView` is called.
// Calling this method guarantees the view will be updated at least once.
- (void)startUpdatingView;

// Requests to stop updating the view previously activated by
// `startUpdatingView`. The updates will actually stop when the latest
// rendering is done.
- (void)stopUpdatingView;

@end
