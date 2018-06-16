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

#if defined(MOUI_METAL)
#  define NANOVG_MTL_IMPLEMENTATION
#elif defined(MOUI_GL2)
#  define NANOVG_GL2_IMPLEMENTATION
#elif defined(MOUI_GLES2)
#  define NANOVG_GLES2_IMPLEMENTATION
#elif defined(MOUI_GL3)
#  define NANOVG_GL3_IMPLEMENTATION
#elif defined(MOUI_GLES3)
#  define NANOVG_GLES3_IMPLEMENTATION
#endif

#include "moui/nanovg_hook.h"

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <string>

#include "nanovg/src/nanovg.h"

#if defined(MOUI_ANDROID)
#  include "aasset.h"
#endif

#if defined(MOUI_GL)
#  include "nanovg/src/nanovg_gl.h"
#  include "nanovg/src/nanovg_gl_utils.h"
#endif

namespace moui {

void nvgClearColor(NVGcontext* context, const int width, const int height,
                   const NVGcolor& clear_color) {
#ifdef MOUI_GL
  const float kAlpha = static_cast<float>(clear_color.a);
  const float kRed = static_cast<float>(clear_color.r) * kAlpha;
  const float kGreen = static_cast<float>(clear_color.g) * kAlpha;
  const float kBlue = static_cast<float>(clear_color.b) * kAlpha;
  glViewport(0, 0, width, height);
  glClearColor(kRed, kGreen, kBlue, kAlpha);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#elif MOUI_METAL
  mnvgClearWithColor(context, clear_color);
#endif
}

bool nvgCompareColor(const NVGcolor& color1, const NVGcolor& color2) {
  return color1.r == color2.r && color1.g == color2.g && \
         color1.b == color2.b && color1.a == color2.a;
}

int nvgContextFlags(const bool antialias, const bool stencil_strokes,
                    const int number_of_buffers) {
  int flags = 0;
  if (antialias)
    flags |= NVG_ANTIALIAS;
  if (stencil_strokes)
    flags |= NVG_STENCIL_STROKES;
#ifdef MOUI_METAL
  if (number_of_buffers >= 3)
    flags |= NVG_TRIPLE_BUFFER;
  else if (number_of_buffers == 2)
    flags |= NVG_DOUBLE_BUFFER;
#endif
  return flags;
}

int nvgCreateFontAtPath(NVGcontext* context, const std::string name,
                        const std::string path) {
#ifdef MOUI_ANDROID
  // Loads the image from Android's assets folder.
  std::FILE* file = aasset_fopen(path.c_str(), "r");
  if (file == NULL) {
    return -1;
  }
  const int kDataSize = aasset_fsize(file);
  unsigned char* data = \
      reinterpret_cast<unsigned char*>(std::malloc(kDataSize));
  int font = -1;
  if (aasset_fread(data, 1, kDataSize, file) == kDataSize) {
    font = nvgCreateFontMem(context, name.c_str(), data, kDataSize,
                             true);  // frees data
  }
  aasset_fclose(file);
  return font;
#endif  // MOUI_ANDROID

  return nvgCreateFont(context, name.c_str(), path.c_str());
}

int nvgCreateImageAtPath(NVGcontext* context, const std::string path,
                         const int image_flags) {
#ifdef MOUI_ANDROID
  // Loads the image from Android's assets folder.
  std::FILE* file = aasset_fopen(path.c_str(), "r");
  if (file == NULL) {
    return -1;
  }
  const int kDataSize = aasset_fsize(file);
  unsigned char data[kDataSize];
  int image = -1;
  if (aasset_fread(data, 1, kDataSize, file) == kDataSize) {
    image = nvgCreateImageMem(context, image_flags, data, kDataSize);
  }
  aasset_fclose(file);
  return image;
#endif  // MOUI_ANDROID

  return nvgCreateImage(context, path.c_str(), image_flags);
}

int nvgCreateImageFromPixels(NVGcontext* context, const int width,
                             const int height, const int image_flags,
                             const bool flips_image_vertically_on_demand,
                             const unsigned char* data) {
  int flags = image_flags | NVG_IMAGE_PREMULTIPLIED;
#ifdef MOUI_GL
  if (flips_image_vertically_on_demand) {
    flags |= NVG_IMAGE_FLIPY;
  }
#endif  // MOUI_GL
  return nvgCreateImageRGBA(context, width, height, flags, data);
}

void nvgDeleteImage(NVGcontext* context, int* image) {
  if (*image < 0)
    return;

  nvgDeleteImage(context, *image);
  *image = -1;
}

void nvgDrawDropShadow(NVGcontext* context, const float x, const float y,
                       const float width, const float height,
                       const float radius, float feather,
                       const NVGcolor inner_color,
                       const NVGcolor outer_color) {
  const NVGpaint kShadowPaint = nvgBoxGradient(context, x, y, width, height,
                                               radius, feather, inner_color,
                                               outer_color);
  nvgBeginPath(context);
  nvgRect(context, x - feather, y - feather, width + feather * 2,
          height + feather * 2);
  nvgFillPaint(context, kShadowPaint);
  nvgFill(context);
}

void nvgReadPixels(NVGcontext* context, int image, int x, int y, int width,
                   int height, void* data) {
#if defined(MOUI_GL)
  glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
#elif defined(MOUI_METAL)
  mnvgReadPixels(context, image, x, y, width, height, data);
#endif
}

void nvgUnpremultiplyImageAlpha(unsigned char* image, const int width,
                                const int height) {
  const int kStride = width * 4;

  // Unpremultiply.
  for (int y = 0; y < height; y++) {
    unsigned char *row = &image[y * kStride];
    for (int x = 0; x < width; x++) {
      const int kRed = row[0], kGreen = row[1], kBlue = row[2], kAlpha = row[3];
      if (kAlpha != 0) {
        row[0] = static_cast<int>(std::min(kRed * 255 / kAlpha, 255));
        row[1] = static_cast<int>(std::min(kGreen * 255 / kAlpha, 255));
        row[2] = static_cast<int>(std::min(kBlue * 255 / kAlpha, 255));
      }
      row += 4;
    }
  }

  // Defringe.
  for (int y = 0; y < height; y++) {
    unsigned char *row = &image[y * kStride];
    for (int x = 0; x < width; x++) {
      const int kAlpha = row[3];
      int red = 0, green = 0, blue = 0, n = 0;
      if (kAlpha == 0) {
        if (x-1 > 0 && row[-1] != 0) {
          red += row[-4];
          green += row[-3];
          blue += row[-2];
          n++;
        }
        if (x + 1 < width && row[7] != 0) {
          red += row[4];
          green += row[5];
          blue += row[6];
          n++;
        }
        if (y - 1 > 0 && row[-kStride + 3] != 0) {
          red += row[-kStride];
          green += row[-kStride + 1];
          blue += row[-kStride + 2];
          n++;
        }
        if (y + 1 < height && row[kStride + 3] != 0) {
          red += row[kStride];
          green += row[kStride + 1];
          blue += row[kStride + 2];
          n++;
        }
        if (n > 0) {
          row[0] = red / n;
          row[1] = green / n;
          row[2] = blue / n;
        }
      }
      row += 4;
    }
  }
}

}  // namespace moui
