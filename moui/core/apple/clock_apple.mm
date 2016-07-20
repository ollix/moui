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

#include "moui/core/clock.h"

#include <dispatch/dispatch.h>
#import <Foundation/Foundation.h>
#include <functional>

namespace {

dispatch_queue_t queue = dispatch_get_global_queue(
    DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

}  // namespace

namespace moui {

void Clock::DispatchAfter(const float delay, std::function<void()> callback) {
  const dispatch_time_t kWhen = dispatch_time(DISPATCH_TIME_NOW,
                                              delay * NSEC_PER_SEC);
  dispatch_after(kWhen, queue, ^{ callback(); });
}

void Clock::ExecuteCallback(Callback* callback) {
  bool func_result = callback->func();
  if (!func_result || callback->interval < 0) {
    delete callback;
    return;
  }

  // Executes the callback again with interval delay.
  dispatch_time_t delay = dispatch_time(
      DISPATCH_TIME_NOW, callback->interval * NSEC_PER_SEC);
  dispatch_after(delay, queue, ^{ Clock::ExecuteCallback(callback); });
}

void Clock::ExecuteCallbackOnMainThread(const float delay,
                                        std::function<void()> callback) {
  if ([NSThread isMainThread] && delay <= 0) {
    callback();
  } else if (delay > 0) {
    const dispatch_time_t kWhen = dispatch_time(DISPATCH_TIME_NOW,
                                                delay * NSEC_PER_SEC);
    dispatch_after(kWhen, dispatch_get_main_queue(), ^{ callback(); });
  } else {
    dispatch_sync(dispatch_get_main_queue(), ^{ callback(); });
  }
}

void Clock::ExecuteCallbackOnMainThread(std::function<void()> callback) {
  ExecuteCallbackOnMainThread(0, callback);
}

}  // namespace moui
