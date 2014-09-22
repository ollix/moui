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
import android.view.MotionEvent;
import android.util.Log;

public class OpenGLView extends GLSurfaceView {

  private boolean mHandlingEvent;
  private long mMouiViewPointer;
  private native void handleEvent(long mouiViewPointer, int action, float x,
                                  float y);
  private native boolean shouldHandleEvent(long mouiViewPointer, float x,
                                           float y);

  public OpenGLView(Context context, long mouiViewPointer) {
    super(context);
    mHandlingEvent = false;
    mMouiViewPointer = mouiViewPointer;

    setEGLContextClientVersion(2);  // adopts OpenGL ES 2.0 context
    setZOrderOnTop(true);
    setEGLConfigChooser(8,  // red size
                        8,  // green size
                        8,  // blue size
                        8,  // alpha size
                        24,  // enables depth
                        8);  // enables stencil
    getHolder().setFormat(PixelFormat.TRANSLUCENT);

    setRenderer(new OpenGLRenderer(mouiViewPointer));
    setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
  }

  // Returns the value returned by native view's HandleEvent() method.
  // If false, the event will be passed to the next responser.
  @Override
  public boolean onTouchEvent(MotionEvent event) {
    // Do nothing if the event is not handling and is not the initial action.
    if (!mHandlingEvent && event.getAction() != MotionEvent.ACTION_DOWN) {
      return false;
    }
    // Determines the action location.
    int[] coords = new int[2];
    getLocationInWindow(coords);
    final float x = event.getRawX() - coords[0];
    final float y = event.getRawY() - coords[1];
    // If it's an initial action, checks if it should handle consequent events.
    if (!mHandlingEvent && !shouldHandleEvent(mMouiViewPointer, x, y)) {
      return false;
    }
    mHandlingEvent = true;
    // Handles the event in corresponded moui view.
    handleEvent(mMouiViewPointer, event.getAction(), x, y);
    // Resets mHandlingEvent if the action is complete.
    if (event.getAction() == MotionEvent.ACTION_UP ||
        event.getAction() == MotionEvent.ACTION_CANCEL) {
      mHandlingEvent = false;
    }
    return true;  // Returns true means this event has consumed.
  }
}
