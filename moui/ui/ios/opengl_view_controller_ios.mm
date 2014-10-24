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

#import "moui/ui/ios/opengl_view_controller_ios.h"

#import "moui/ui/ios/opengl_view_ios.h"
#import "moui/ui/view.h"


@implementation MOOpenGLViewController

- (id)initWithMouiView:(moui::View*)mouiView {
  if((self = [super init])) {
    _mouiView = mouiView;
  }
  return self;
}

- (void)loadView {
  [super loadView];
  self.view = [[MOOpenGLView alloc] initWithViewController:self
                                                  mouiView:_mouiView];
  [self.view autorelease];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  MOOpenGLView* openGLView = (MOOpenGLView *)self.view;
  [[openGLView layer] setNeedsDisplay];
}

@end
