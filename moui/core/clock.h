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

#ifndef MOUI_CORE_CLOCK_H_
#define MOUI_CORE_CLOCK_H_

#include <chrono>  // NOLINT
#include <functional>
#include <utility>

#include "moui/base.h"

namespace moui {

// The `Clock` class is used to schedule function calls. It is designed as a
// place for static class methods so don't try instantiating this class.
class Clock {
 public:
  struct Callback {
    std::function<void()> func;
  };

  Clock() {}
  ~Clock() {}

  // Executes the `callback` function at the specified `delay` time in seconds.
  static void DispatchAfter(const float delay, std::function<void()> callback);

  // Executes the specified callback on the main thread with a delay time
  // in seconds.
  static void ExecuteCallbackOnMainThread(const float delay,
                                          std::function<void()> callback);

  // Executes the specified callback on the main thread.
  static void ExecuteCallbackOnMainThread(std::function<void()> callback);

  // Returns a time point representing the current point in time. The time
  // point is not related to wall clock time and cannot decrease as physical
  // time moves forward. It is best suitable for measuring intervals.
  // In addition, the time point is represented in seconds but is accurate to
  // nanoseconds.
  static double GetTimestamp() {
    static auto start = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start).count();
  }

  // Resets the clock.
  static void Reset();

 private:
  DISALLOW_COPY_AND_ASSIGN(Clock);
};

}  // namespace moui

#endif  // MOUI_CORE_CLOCK_H_
