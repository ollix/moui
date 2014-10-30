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

#ifndef MOUI_NANOVG_HOOK_H_
#define MOUI_NANOVG_HOOK_H_

#if defined MOUI_GL2
#  define NANOVG_GL2
#  define nvgCreateGL(flags) nvgCreateGL2(flags)
#  define nvgDeleteGL(context) nvgDeleteGL2(context)
#elif defined MOUI_GLES2
#  define NANOVG_GLES2
#  define nvgCreateGL(flags) nvgCreateGLES2(flags)
#  define nvgDeleteGL(context) nvgDeleteGLES2(context)
#elif defined MOUI_GL3
#  define NANOVG_GL3
#  define nvgCreateGL(flags) nvgCreateGL3(flags)
#  define nvgDeleteGL(context) nvgDeleteGL3(context)
#elif defined MOUI_GLES3
#  define NANOVG_GLES3
#  define nvgCreateGL(flags) nvgCreateGLES3(flags)
#  define nvgDeleteGL(context) nvgDeleteGLES3(context)
#endif

#include "moui/opengl_hook.h"
#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

// Workaround to fix the missing prototype in nanovg.
void nvgluBindFramebuffer(NVGLUframebuffer* fb);

// Additonal APIs for nanovg.
namespace moui {

// Returns true if passed colors are the same.
bool nvgCompareColor(const NVGcolor& color1, const NVGcolor& color2);

// Returns the image identifier of the current snapshot of the passed context
// or 0 on failure. The returned image needs to be freed manually through
// nvgDeleteImage().
int nvgCreateImageSnapshot(NVGcontext* context, const int x, const int y,
                           const int width, const int height,
                           const float scale_factor);

}  // namespace moui

#endif  // MOUI_NANOVG_HOOK_H_
