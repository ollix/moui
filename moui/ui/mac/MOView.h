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

#import <Cocoa/Cocoa.h>

#include "moui/base.h"

namespace moui {
class View;
}  // namespace moui

@interface MOView : NSView {
 @private
  CVDisplayLinkRef _displayLink;
  dispatch_source_t _displaySource;
  NSSize _drawableSize;
  moui::View* _mouiView;
  BOOL _needsRedraw;  // requests the view to update in the next refresh cycle
  BOOL _stopsUpdatingView;
}

- (id)initWithMouiView:(moui::View *)mouiView;

// Returns `YES` if the view's background is opaque.
- (BOOL)backgroundIsOpaque;

// Creates the drawable for presenting on the screen. This method should be
// implemented in the subclass.
- (void)createDrawableWithSize:(NSSize)size;

// Destroies the drawable. This method should be implemented in the subclass.
- (void)destroyDrawable;

// Returns the current mouse location related to the view's top-left origin.
- (moui::Point)mouseLocation;

// Presents the drawable. This method should be implemented in the subclass.
- (void)presentDrawable;

// Prepares the environment for drawable to render. This method should be
// implemented in the subclass.
- (void)prepareDrawable;

// Sets whether the view's background is opaque.
- (void)setBackgroundOpaque:(BOOL)isOpaque;

// Guarantees the view to update in the next refresh cycle of the display.
- (void)setNeedsRedraw;

// Starts updating the view synchronized to the refresh rate of the display.
// The view will be updated continuously until `stopUpdatingView` is called.
- (void)startUpdatingView;

// Requests to stop updating the view previously activated by
// `startUpdatingView`.
- (void)stopUpdatingView;

@end
