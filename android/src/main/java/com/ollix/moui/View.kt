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
import android.view.Choreographer
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.SurfaceView

abstract class View(context: Context, mouiViewPtr: Long)
        : SurfaceView(context), SurfaceHolder.Callback {
    private var animationIsPaused = false
    private val displayDensity: Float
    private var drawableIsValid = false
    private val frameCallback: Choreographer.FrameCallback
    private var handlingEvent = false
    private var isAnimating = false
    private val mouiViewPtr: Long
    private var pendingFrameUpdate = false

    init {
        displayDensity = context.getResources().getDisplayMetrics().density
        this.mouiViewPtr = mouiViewPtr
        holder.addCallback(this)
        setZOrderOnTop(true)

        frameCallback = object : Choreographer.FrameCallback {
            override fun doFrame(frameTimeNanos: Long) {
                pendingFrameUpdate = false
                renderFrame()
            }
        }
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
        /** Determines the action location. */
        var coords = IntArray(2)
        getLocationInWindow(coords)
        val X: Float = (event.getRawX() - coords[0]) / displayDensity
        val Y: Float = (event.getRawY() - coords[1]) / displayDensity
        /**
         * If it's an initial action, checks if it should handle consequent
         * events.
         */
        if (!handlingEvent && !shouldHandleEventFromJNI(mouiViewPtr, X, Y)) {
            return false
        }
        handlingEvent = true
        /** Handles the event in corresponded moui view. */
        handleEventFromJNI(mouiViewPtr, event.getAction(), X, Y)
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
        }
    }

    fun redrawView() {
        if (pendingFrameUpdate) {
            return
        }

        if (drawableIsValid) {
            pendingFrameUpdate = true
            Choreographer.getInstance().postFrameCallback(frameCallback)
        }
    }

    fun startUpdatingView() {
        if (drawableIsValid) {
            isAnimating = true
            redrawView()
        }
    }

    fun stopUpdatingView() {
        if (drawableIsValid) {
            isAnimating = false
            Choreographer.getInstance().removeFrameCallback(frameCallback)
        }
    }

    /** Implmenets SurfaceHolder.Callback methods */

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int,
                                height: Int) {
        redrawView()
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        if (drawableIsValid) {
            destroyDrawable()
        }
        if (createDrawable()) {
            animationIsPaused = false
            drawableIsValid = true
        } else {
            destroyDrawable()
        }
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        if (!drawableIsValid) {
            return
        }
        if (isAnimating) {
            stopUpdatingView()
            isAnimating = true
            animationIsPaused = true
        }
        destroyDrawable()
        drawableIsValid = false
        onSurfaceDestroyedFromJNI(mouiViewPtr)
    }

    /** Abstract functions */
    abstract fun createDrawable(): Boolean
    abstract fun destroyDrawable()
    abstract fun prepareDrawable()
    abstract fun presentDrawable()

    /** JNI functions */
    external fun drawFrameFromJNI(mouiViewPtr: Long)

    external fun handleEventFromJNI(mouiViewPtr: Long,
                                    action: Int,
                                    x: Float,
                                    y: Float)

    external fun shouldHandleEventFromJNI(mouiViewPtr: Long,
                                          x: Float,
                                          y: Float): Boolean

    external fun onSurfaceDestroyedFromJNI(mouiViewPtr: Long)
}
