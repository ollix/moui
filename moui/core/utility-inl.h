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

#ifndef MOUI_CORE_UTILITY_INL_H_
#define MOUI_CORE_UTILITY_INL_H_

#include <chrono>

namespace moui {

// Prototypes.

// Returns the timestamp in milliseconds for measuring intervals. The timestamp
// is not related to wall clock time. The specified offset will be added to the
// current timestamp.
inline double GetTimestamp(const double offset);

// Implementation.

inline double GetTimestamp(const double offset) {
  auto now = std::chrono::steady_clock::now();
  double timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()).count() / 1000.0;
  return timestamp + offset;
}

}  // namespace moui

#endif  // MOUI_CORE_PATH_H_