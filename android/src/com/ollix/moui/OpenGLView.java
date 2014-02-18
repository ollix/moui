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

package com.ollix.moui;

import com.ollix.moui.OpenGLRenderer;
import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;

public class OpenGLView extends GLSurfaceView {
  public OpenGLView(Context context, long mouiViewPointer) {
    super(context);

    setEGLContextClientVersion(2);  // adopts OpenGL ES 2.0 context
    setZOrderOnTop(true);
    setEGLConfigChooser(8, 8, 8, 8, 16, 0);
    getHolder().setFormat(PixelFormat.TRANSLUCENT);

    setRenderer(new OpenGLRenderer(mouiViewPointer));
    setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
  }
}
