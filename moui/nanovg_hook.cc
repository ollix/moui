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

#include "moui/opengl_hook.h"
#include "nanovg.h"

// Makes sure the nanovg implementation loaded once to prevent "undefined
// symbols" or "duplicate symbols" issues. Only includes the "nanovg_hook.h"
// header file in other modules to use nanovg.
#if defined MOUI_GL2
#  define NANOVG_GL2_IMPLEMENTATION
#elif defined MOUI_GLES2
#  define NANOVG_GLES2_IMPLEMENTATION
#elif defined MOUI_GL3
#  define NANOVG_GL3_IMPLEMENTATION
#elif defined MOUI_GLES3
#  define NANOVG_GLES3_IMPLEMENTATION
#endif
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"
