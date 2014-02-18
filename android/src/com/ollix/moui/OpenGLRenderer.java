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

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OpenGLRenderer implements GLSurfaceView.Renderer {

  private long mMouiViewPointer;
  private native void drawFrame(long mouiViewPointer);

  public OpenGLRenderer(long mouiViewPointer) {
    mMouiViewPointer = mouiViewPointer;
  }

  @Override
  public void onDrawFrame(GL10 unused) {
    drawFrame(mMouiViewPointer);
  }

  @Override
  public void onSurfaceCreated(GL10 unused, EGLConfig config) {}

  @Override
  public void onSurfaceChanged(GL10 unused, int width, int height) {
    GLES20.glViewport(0, 0, width, height);
  }
}
