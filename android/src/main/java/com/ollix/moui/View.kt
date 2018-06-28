/*
 * Copyright (c) 2017 Ollix. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ---
 * Author: olliwang@ollix.com (Olli Wang)
 */

package com.ollix.moui

import android.content.Context
import android.graphics.SurfaceTexture
import android.os.SystemClock
import android.view.Choreographer
import android.view.MotionEvent
import android.view.TextureView

abstract class View(context: Context, mouiViewPtr: Long)
        : TextureView(context), TextureView.SurfaceTextureListener {

    private var animationIsPaused = false
    private val displayDensity: Float
    private var drawableIsValid = false
    private val frameCallback: Choreographer.FrameCallback
    private var handlingEvent = false
    private var isAnimating = false
    private var lastRedrawTime: Long = 0
    private val mouiViewPtr: Long
    private var pendingFrameUpdate = false
    private var textureSizeChanged = false

    init {
        displayDensity = context.getResources().getDisplayMetrics().density
        this.mouiViewPtr = mouiViewPtr
        setOpaque(false)

        frameCallback = object : Choreographer.FrameCallback {
            override fun doFrame(frameTimeNanos: Long) {
                pendingFrameUpdate = false
                renderFrame()
            }
        }
        setSurfaceTextureListener(this)
    }

    fun backgroundIsOpaque(): Boolean {
        return isOpaque()
    }

    /**
     * Returns the value returned by native view's HandleEvent() method.
     * If false, the event will be passed to the next responser.
     */
    override fun onTouchEvent(event: MotionEvent): Boolean {
        /**
         * Does nothing if the event is not handling and is not the initial
         * action.
         */
        if (!handlingEvent && event.getAction() != MotionEvent.ACTION_DOWN) {
            return false
        }
        /** Populates the action locations. */
        var coords = IntArray(2)
        getLocationInWindow(coords)
        val locations = FloatArray(event.pointerCount * 2)
        var index = 0
        for (i in 0 until event.pointerCount) {
            locations[index++] = (event.getX(i) - coords[0]) / displayDensity
            locations[index++] = (event.getY(i) - coords[1]) / displayDensity
        }

        /**
         * If it's an initial action, checks if it should handle consequent
         * events.
         */
        if (!handlingEvent && !shouldHandleEventFromJNI(mouiViewPtr,
                                                        locations[0],
                                                        locations[1])) {
            return false
        }

        handlingEvent = true
        /** Handles the event in corresponded moui view. */
        handleEventFromJNI(mouiViewPtr, event.getAction(), locations)
        /** Resets `handlingEvent` if the action is complete. */
        if (event.getAction() == MotionEvent.ACTION_UP ||
                event.getAction() == MotionEvent.ACTION_CANCEL) {
            handlingEvent = false
        }
        return true    // Returns true means this event has consumed.
    }

    fun renderFrame() {
        if (!drawableIsValid) {
            return
        }

        prepareDrawable()
        drawFrameFromJNI(mouiViewPtr)
        presentDrawable()

        if (isAnimating && !animationIsPaused) {
            redrawView()
        } else if (textureSizeChanged) {
            textureSizeChanged = false
            redrawView()
        }
    }

    fun redrawView() {
        if (!drawableIsValid) {
            return
        }
        val redrawTime = SystemClock.elapsedRealtime()
        if ((redrawTime - lastRedrawTime) > 500) {
            pendingFrameUpdate = false
        }
        lastRedrawTime = redrawTime

        if (pendingFrameUpdate) {
            return
        }

        if (drawableIsValid) {
            pendingFrameUpdate = true
            Choreographer.getInstance().postFrameCallback(frameCallback)
        }
    }

    fun setBackgroundOpaque(isOpaque: Boolean) {
        setOpaque(isOpaque)
    }

    fun startUpdatingView() {
        if (drawableIsValid && !isAnimating) {
            isAnimating = true
            redrawView()
        }
    }

    fun stopUpdatingView() {
        if (drawableIsValid) {
            isAnimating = false
        }
    }

    /** Implements TextureView.SurfaceTextureListener methods */

    override fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture,
                                           width: Int,
                                           height: Int) {
        if (drawableIsValid) {
            destroyDrawable()
        }
        if (createDrawable(surfaceTexture as Object)) {
            drawableIsValid = true
            if (animationIsPaused) {
                animationIsPaused = false
                startUpdatingView()
            } else {
                redrawView()
            }
        } else {
            destroyDrawable()
        }
    }

    override fun onSurfaceTextureSizeChanged(st: SurfaceTexture,
                                             width: Int,
                                             height: Int) {
        textureSizeChanged = true
        redrawView()
    }

    override fun onSurfaceTextureDestroyed(st: SurfaceTexture): Boolean {
        if (!drawableIsValid) {
            return true
        }
        if (isAnimating) {
            stopUpdatingView()
            isAnimating = true
            animationIsPaused = true
        }
        destroyDrawable()
        drawableIsValid = false
        onSurfaceDestroyedFromJNI(mouiViewPtr)
        return true
    }

    override fun onSurfaceTextureUpdated(st: SurfaceTexture) {
    }

    /** Abstract functions */
    abstract fun createDrawable(surface: Object): Boolean
    abstract fun destroyDrawable()
    abstract fun prepareDrawable()
    abstract fun presentDrawable()

    /** JNI functions */
    external fun drawFrameFromJNI(mouiViewPtr: Long)

    external fun handleEventFromJNI(mouiViewPtr: Long,
                                    action: Int,
                                    locations: FloatArray)

    external fun shouldHandleEventFromJNI(mouiViewPtr: Long,
                                          x: Float,
                                          y: Float): Boolean

    external fun onSurfaceDestroyedFromJNI(mouiViewPtr: Long)
}
