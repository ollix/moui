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

#include <string>

#include "moui/defines.h"

#include "nanovg/src/nanovg.h"

#if defined(MOUI_GL)
#  include "moui/opengl_hook.h"
#  include "nanovg/src/nanovg_gl_utils.h"
#elif defined(MOUI_METAL)
#  include "MetalNanoVG/src/nanovg_mtl.h"
#endif

// Forward declaration.
extern "C" {
#if defined(MOUI_GL2)
  NVGcontext* nvgCreateGL2(int flags);
  void nvgDeleteGL2(NVGcontext* ctx);
#elif defined(MOUI_GLES2)
  NVGcontext* nvgCreateGLES2(int flags);
  void nvgDeleteGLES2(NVGcontext* ctx);
#elif defined(MOUI_GLES3)
  NVGcontext* nvgCreateGLES3(int flags);
  void nvgDeleteGLES3(NVGcontext* ctx);
#endif
}  // extern "C"

#if defined(MOUI_GL2)
#  define NANOVG_GL2 1
#  define nvgCreateContext(flags) nvgCreateGL2(flags)
#  define nvgDeleteContext(context) nvgDeleteGL2(context)
#elif defined(MOUI_GLES2)
#  define NANOVG_GLES2 1
#  define nvgCreateContext(flags) nvgCreateGLES2(flags)
#  define nvgDeleteContext(context) nvgDeleteGLES2(context)
#elif defined(MOUI_GL3)
#  define NANOVG_GL3 1
#  define nvgCreateContext(flags) nvgCreateGL3(flags)
#  define nvgDeleteContext(context) nvgDeleteGL3(context)
#elif defined(MOUI_GLES3)
#  define NANOVG_GLES3 1
#  define nvgCreateContext(flags) nvgCreateGLES3(flags)
#  define nvgDeleteContext(context) nvgDeleteGLES3(context)
#elif defined(MOUI_METAL)
#  define nvgCreateContext(layer, flags) nvgCreateMTL(layer, flags)
#  define nvgDeleteContext(context) nvgDeleteMTL(context)
#  define nvgBindFramebuffer(fb) mnvgBindFramebuffer(fb)
#  define nvgCreateFramebuffer(ctx, w, h, flags) \
          mnvgCreateFramebuffer(ctx, w, h, flags)
#  define nvgDeleteFramebuffer(fb) mnvgDeleteFramebuffer(fb)
#endif

#ifdef MOUI_GL
#  define nvgBindFramebuffer(fb) nvgluBindFramebuffer(fb)
#  define nvgCreateFramebuffer(ctx, w, h, flags) \
          nvgluCreateFramebuffer(ctx, w, h, flags)
#  define nvgDeleteFramebuffer(fb) nvgluDeleteFramebuffer(fb)
#endif

#if defined(MOUI_GL)
  typedef NVGLUframebuffer NVGframebuffer;
#elif defined(MOUI_METAL)
  typedef MNVGframebuffer NVGframebuffer;
#endif

// Additonal APIs for nanovg.
namespace moui {

// Clears the current color buffer with the specified `clear_color`. Note that
// this function should be called between `nvgBindFramebuffer()` and
// `nvgBeginFrame()`.
void nvgClearColor(NVGcontext* context, const int width, const int height,
                   const NVGcolor& clear_color);

// Returns `true` if passed colors are the same.
bool nvgCompareColor(const NVGcolor& color1, const NVGcolor& color2);

// Returns context flags for the specified parameters.
int nvgContextFlags(const bool antialias, const bool stencil_strokes,
                    const int number_of_buffers);

// Creates a font at the specified path and returns the font handle.
// This function is the enhanced version of `nvgCreateFont()` that has
// better cross-platform support.
int nvgCreateFontAtPath(NVGcontext* context, const std::string name,
                        const std::string path);

// Creates an image at the specified path and returns the image handle.
// This function is the enhanced version of `nvgCreateImage()` that has
// better cross-platform support.
int nvgCreateImageAtPath(NVGcontext* context, const std::string path,
                         const int image_flags);

// Creates an image for pixels read from `nvgReadPixels()`.
int nvgCreateImageFromPixels(NVGcontext* context, const int width,
                             const int height, const int image_flags,
                             const unsigned char* data);

// Deletes the passed image and set the image id to -1.
void nvgDeleteImage(NVGcontext* context, int* image);

// Draws drop shadow.
void nvgDrawDropShadow(NVGcontext* context, const float x, const float y,
                       const float width, const float height,
                       const float radius, float feather,
                       const NVGcolor inner_color,
                       const NVGcolor outer_color);

// Copies the pixels from the specified image into the specified `data`.
// Note that for OpenGL, this function reads the pixels from the currently
// binded render buffer directly instead of the specified `image`.
void nvgReadPixels(NVGcontext* context, int image, int x, int y, int width,
                   int height, void* data);

// Updates the specified `image` data to unpremultiply its alpha values.
void nvgUnpremultiplyImageAlpha(unsigned char* image, const int width,
                                const int height);

}  // namespace moui

#endif  // MOUI_NANOVG_HOOK_H_
