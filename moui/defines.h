// Copyright (c) 2016 Ollix. All rights reserved.
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

#ifndef MOUI_DEFINES_H_
#define MOUI_DEFINES_H_

#if defined(MOUI_IOS) || defined(MOUI_MAC)
#  define MOUI_APPLE
#endif

#ifdef MOUI_METAL
#  define MOUI_BGFX
#endif

#if defined(MOUI_BGFX)
#elif defined MOUI_GL2
#  define NANOVG_GL2_IMPLEMENTATION
#elif defined MOUI_GLES2
#  define NANOVG_GLES2_IMPLEMENTATION
#elif defined MOUI_GL3
#  define NANOVG_GL3_IMPLEMENTATION
#elif defined MOUI_GLES3
#  define NANOVG_GLES3_IMPLEMENTATION
#endif

#endif  // MOUI_DEFINES_H_
