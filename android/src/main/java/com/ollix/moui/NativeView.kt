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
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.view.ViewParent
import android.widget.RelativeLayout

class NativeView(context: Context) {

    private val displayDensity: Float

    init {
        displayDensity = context.getResources().getDisplayMetrics().density
    }

    /** Adds subview into a parent view. */
    fun addSubview(parentView: View, subview: View) {
        if (parentView is ViewGroup) {
            parentView.addView(subview)
        }
    }

    /** Returns the view height in points */
    fun getHeight(view: View): Float {
        return (view.height / displayDensity).toFloat()
    }

    /** Returns the int array of view's snapshot colors. */
    fun getSnapshot(view: View): IntArray {
        val bitmap = Bitmap.createBitmap(view.width , view.height,
                                         Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        view.layout(0, 0, view.width, view.height)
        view.draw(canvas)

        var pixels = IntArray(view.width * view.height)
        bitmap.getPixels(pixels,
                         0,  // offset
                         view.width,  // stride
                         0,  // x
                         0,  // y
                         view.width,
                         view.height)
        return pixels
    }

    /** Returns the parent of the specified view. */
    fun getSuperview(view: View): View? {
        val parent: ViewParent = view.parent
        if (parent is View) {
            return parent
        }
        return null
    }

    /** Returns the view width in points */
    fun getWidth(view: View): Float {
        return (view.width / displayDensity).toFloat()
    }

    /** Returns `true` if the view is hidden. */
    fun isHidden(view: View): Boolean {
        return if (view.visibility == View.INVISIBLE) true else false
    }

    /** Removes the specified view from its parent. */
    fun removeFromSuperview(view: View) {
        val parent: ViewParent = view.parent
        if (parent is ViewGroup) {
            parent.removeView(view)
        }
    }

    /** Changes the order of the subview so that it's behind its siblings. */
    fun sendSubviewToBack(parentView: View, subview: View) {
        if (parentView is ViewGroup) {
            val subviewIndex: Int = parentView.indexOfChild(subview);
            if (subviewIndex <= 0) {
                return
            }
            var s = 0
            for (i in 0 until parentView.childCount) {
                val child: View = parentView.getChildAt(s)
                if (child == subview) {
                    s = 1
                } else {
                    child.bringToFront()
                }
            }
        }
    }

    /** Sets the position and dimension of the view. */
    fun setBounds(view: View, x: Float, y: Float, width: Float, height: Float) {
        val layoutParams = RelativeLayout.LayoutParams(
                (width * displayDensity).toInt(),
                (height * displayDensity).toInt())
        layoutParams.leftMargin = (x * displayDensity).toInt()
        layoutParams.topMargin = (y * displayDensity).toInt()
        view.layoutParams = layoutParams
    }

    /** Sets the visibility of the view. */
    fun setHidden(view: View, hidden: Boolean) {
        view.visibility = if (hidden) View.INVISIBLE else View.VISIBLE
    }
}
