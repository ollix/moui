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

import android.os.Bundle
import android.view.ViewTreeObserver

import androidx.fragment.app.Fragment

open class MouiFragment : Fragment() {

    override fun onCreateView(inflater: android.view.LayoutInflater,
                              container: android.view.ViewGroup?,
                              savedInstanceState: Bundle?): android.view.View? {
        /** Launches the app once the layout is ready. */
        container?.viewTreeObserver?.addOnGlobalLayoutListener(
            object : ViewTreeObserver.OnGlobalLayoutListener {
                override fun onGlobalLayout() {
                    val viewTreeObserver = container.viewTreeObserver
                    viewTreeObserver.removeOnGlobalLayoutListener(this)
                    launchMouiAppFromJNI()
                }
            }
        )
        return null;
    }

    /** JNI functions. */
    external fun launchMouiAppFromJNI()

    /** Loads native libraries. */
    companion object {
        init {
            System.loadLibrary("moui-jni")
        }
    }
}
