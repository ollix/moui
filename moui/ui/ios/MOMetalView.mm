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

#ifdef MOUI_METAL

#import "moui/ui/ios/MOMetalView.h"

#import <QuartzCore/QuartzCore.h>

@implementation MOMetalView {
 @private
  BOOL _registeredNotification;
}

+ (Class)layerClass {
  return [CAMetalLayer class];
}

- (void)createDrawableWithSize:(CGSize)size {
  CAMetalLayer* layer = (CAMetalLayer*)self.layer;
  layer.drawableSize = size;
  layer.presentsWithTransaction = YES;

  if (!_registeredNotification) {
    NSNotificationCenter *notificationCenter = \
        [NSNotificationCenter defaultCenter];
    UIApplication *application = [UIApplication sharedApplication];
    [notificationCenter addObserver:self
          selector:@selector(enableTransaction)
          name:UIApplicationDidBecomeActiveNotification
          object:application];
  //   [notificationCenter addObserver:self
  //       selector:@selector(disableTransaction)
  //       name:UIApplicationWillResignActiveNotification
  //       object:application];
    _registeredNotification = YES;
  }
}

- (void)enableTransaction {
  CAMetalLayer* layer = (CAMetalLayer*)self.layer;
  layer.presentsWithTransaction = YES;
}

- (void)disableTransaction {
  CAMetalLayer* layer = (CAMetalLayer*)self.layer;
  layer.presentsWithTransaction = NO;
}

- (void)presentMetalLayerWithTransaction:(bool)value {
  CAMetalLayer* layer = (CAMetalLayer*)self.layer;
  layer.presentsWithTransaction = value;
}

@end

#endif  // MOUI_METAL
