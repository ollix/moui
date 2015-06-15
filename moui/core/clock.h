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
  // The structure for holding the callback function and interval.
  struct Callback {
    std::function<bool(void)> func;
    float interval;
  };

  Clock() {}
  ~Clock() {}

  // Schedules function call at a reqular interval. The callback function must
  // have the return type of bool indicating whether to stop scheduling. The
  // function will be called only once if interval is negative.
  template<class Function, class... Args>
  static void ScheduleInterval(float interval, Function&& f, Args&&... args) {
    auto callback = new Callback;
    callback->func = std::bind(f, args...);
    callback->interval = interval;
    ExecuteCallback(callback);
  }

  // Executes the callback, waits the interval time in platform-specific thread,
  // then executes it again. It stops once the callback returning `false` or
  // the interval is negative. This method should be used internally only. It's
  // defined here for bridging JNI code.
  static void ExecuteCallback(Callback* callback);

  // Executes the specified callback on the main thread.
  static void ExecuteCallbackOnMainThread(std::function<void()> callback);

  // Returns a time point representing the current point in time. The time
  // point is not related to wall clock time and cannot decrease as physical
  // time moves forward. It is best suitable for measuring intervals.
  // In addition, the time point is represented in seconds but is accurate to
  // nanoseconds.
  static double GetTimestamp() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count() / 1000000000.0;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Clock);
};

}  // namespace moui

#endif  // MOUI_CORE_CLOCK_H_
