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

#ifndef MOUI_CORE_LOG_H_
#define MOUI_CORE_LOG_H_

#if defined MOUI_APPLE
#include <cstdio>
#elif defined MOUI_ANDROID
#include <android/log.h>
#endif

#if defined MOUI_APPLE
#define MO_LOG(...) std::printf(__VA_ARGS__);
#elif defined MOUI_ANDROID
#define MO_LOG(...) __android_log_print(ANDROID_LOG_INFO, "moui", __VA_ARGS__)
#endif

#endif  // MOUI_CORE_LOG_H_
