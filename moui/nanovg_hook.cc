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

#if defined MOUI_GL2
#  define NANOVG_GL2_IMPLEMENTATION
#elif defined MOUI_GLES2
#  define NANOVG_GLES2_IMPLEMENTATION
#elif defined MOUI_GL3
#  define NANOVG_GL3_IMPLEMENTATION
#elif defined MOUI_GLES3
#  define NANOVG_GLES3_IMPLEMENTATION
#endif

#include "moui/nanovg_hook.h"

#include <algorithm>
#include <cstdlib>

namespace {

// Returns the data of the image snapshot from the current framebuffer.
// The returned image data should be freed manually when no longer needed.
// `NULL` is returned on failure.
unsigned char* CreateImageSnapshot(const int x, const int y, const int width,
                                   const int height) {
  unsigned char* image = \
      reinterpret_cast<unsigned char*>(std::malloc(width * height * 4));
  if (image == NULL)
    return NULL;

  glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
  return image;
}

// Sets the alpha value of every pixel of the passed image.
void SetImageAlpha(const int width, const int height, unsigned char alpha,
                   unsigned char* image) {
  const int kStride = width * 4;
  int x, y;
  for (y = 0; y < height; y++) {
    unsigned char* row = &image[y * kStride];
    for (x = 0; x < width; x++)
      row[x * 4 + 3] = alpha;
  }
}

// Flips the passed image vertically.
void FlipImageVertically(const int width, const int height,
                         unsigned char* image) {
  const int kStride = width * 4;
  int i = 0, j = height - 1, k;
  while (i < j) {
    unsigned char* ri = &image[i * kStride];
    unsigned char* rj = &image[j * kStride];
    for (k = 0; k < kStride; k++) {
      unsigned char t = ri[k];
      ri[k] = rj[k];
      rj[k] = t;
    }
    i++;
    j--;
  }
}

// Unpremultiplies the alpha value of every pixel of the passed image.
void UnpremultiplyImageAlpha(const int width, const int height,
                             unsigned char* image) {
  const int kStride = width * 4;
  int x, y;

  // Unpremultiply
  for (y = 0; y < height; y++) {
    unsigned char* row = &image[y * kStride];
    for (x = 0; x < width; x++) {
      int r = row[0], g = row[1], b = row[2], a = row[3];
      if (a != 0) {
        row[0] = static_cast<int>(std::min(r * 255 / a, 255));
        row[1] = static_cast<int>(std::min(g * 255 / a, 255));
        row[2] = static_cast<int>(std::min(b * 255 / a, 255));
      }
      row += 4;
    }
  }

  // Defringe
  for (y = 0; y < height; y++) {
    unsigned char* row = &image[y * kStride];
    for (x = 0; x < width; x++) {
      int r = 0, g = 0, b = 0, a = row[3], n = 0;
      if (a == 0) {
        if (x-1 > 0 && row[-1] != 0) {
          r += row[-4];
          g += row[-3];
          b += row[-2];
          n++;
        }
        if (x+1 < width && row[7] != 0) {
          r += row[4];
          g += row[5];
          b += row[6];
          n++;
        }
        if (y-1 > 0 && row[-kStride + 3] != 0) {
          r += row[-kStride];
          g += row[-kStride + 1];
          b += row[-kStride + 2];
          n++;
        }
        if (y+1 < height && row[kStride + 3] != 0) {
          r += row[kStride];
          g += row[kStride + 1];
          b += row[kStride + 2];
          n++;
        }
        if (n > 0) {
          row[0] = r / n;
          row[1] = g / n;
          row[2] = b / n;
        }
      }
      row += 4;
    }
  }
}

}  // namespace

namespace moui {

bool nvgCompareColor(const NVGcolor& color1, const NVGcolor& color2) {
  return color1.r == color2.r && color1.g == color2.g && \
         color1.b == color2.b && color1.a == color2.a;
}

int nvgCreateImageSnapshot(NVGcontext* context, const int x, const int y,
                           const int width, const int height,
                           const float scale_factor) {
  const int kScaledWidth = width * scale_factor;
  const int kScaledHeight = height * scale_factor;
  unsigned char* image = CreateImageSnapshot(x * scale_factor, y * scale_factor,
                                             kScaledWidth, kScaledHeight);
  if (image == NULL)
    return -1;

  const int kIdentifier = nvgCreateImageRGBA(
      context, kScaledWidth, kScaledHeight,
      NVG_IMAGE_FLIPY | NVG_IMAGE_PREMULTIPLIED, image);
  std::free(image);
  return kIdentifier;
}

void nvgDeleteFramebuffer(NVGLUframebuffer** framebuffer) {
  if (framebuffer == nullptr || *framebuffer == nullptr) {
    return;
  }
  nvgluDeleteFramebuffer(*framebuffer);
  *framebuffer = nullptr;
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

}  // namespace moui
