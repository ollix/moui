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
#include <cstdio>

#include "nanovg/src/nanovg.h"

#ifdef MOUI_BGFX
#  include "bgfx/bgfx.h"
#endif

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

#ifndef MOUI_BGFX
  glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
#endif
  return image;
}

#ifdef MOUI_BGFX
bool* GetViewIdAvailabilityTable() {
  static bool* table = nullptr;
  if (table == nullptr) {
    const int kMaxViews = static_cast<int>(bgfx::getCaps()->limits.maxViews);
    table = reinterpret_cast<bool*>(std::malloc(sizeof(bool) * kMaxViews));
    for (int i = 0; i < kMaxViews; ++i)
      table[i] = true;
  }
  return table;
}
#endif  // MOUI_BGFX

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

}  // namespace

namespace moui {

bool nvgCompareColor(const NVGcolor& color1, const NVGcolor& color2) {
  return color1.r == color2.r && color1.g == color2.g && \
         color1.b == color2.b && color1.a == color2.a;
}

NVGLUframebuffer* nvgCreateFramebuffer(NVGcontext* context, const int width,
                                       const int height,
                                       const int image_flags) {
  NVGLUframebuffer* framebuffer = nvgluCreateFramebuffer(context, width,
                                                         height, image_flags);
#ifdef MOUI_BGFX
  if (framebuffer != nullptr) {
    bool* view_id_availability_table = GetViewIdAvailabilityTable();
    int view_id = 0;
    for (int i = 1; i < bgfx::getCaps()->limits.maxViews; ++i) {
      if (view_id_availability_table[i]) {
        view_id_availability_table[i] = false;
        view_id = i;
        break;
      }
    }
    framebuffer->viewId = 0;
    if (view_id == 0) {
      nvgDeleteFramebuffer(&framebuffer);
    } else {
      bgfx::setViewMode(view_id, bgfx::ViewMode::Sequential);
      nvgluSetViewFramebuffer(view_id, framebuffer);
    }
  }
#endif  // MOUI_BGFX
  if (framebuffer == nullptr)
    fprintf(stderr, "Failed to Create NanoVG Framebuffer\n");
  return framebuffer;
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
#ifdef MOUI_BGFX
  const int kViewId = static_cast<int>((*framebuffer)->viewId);
  bool* view_id_availability_table = GetViewIdAvailabilityTable();
  if (kViewId > 0)
    view_id_availability_table[kViewId] = true;
#endif  // MOUI_BGFX
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
