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

import android.os.Handler
import android.os.Looper
import kotlinx.coroutines.experimental.delay
import kotlinx.coroutines.experimental.launch

class Clock {

    private val handler: Handler = Handler(Looper.getMainLooper())

    /** Executes a callback in background. */
    fun dispatchAfter(delaySeconds: Float, callbackPtr: Long) {
        launch {
            delay((delaySeconds * 1000).toLong())
            executeCallbackFromJNI(callbackPtr)
        }
    }

    /** Executes a callback on main thread */
    fun executeCallbackOnMainThread(delaySeconds: Float, callbackPtr: Long) {
        if (delaySeconds == 0f && Looper.myLooper() == Looper.getMainLooper()) {
            executeCallbackFromJNI(callbackPtr)
            return
        }

        handler.postDelayed({
            executeCallbackFromJNI(callbackPtr)
        }, (delaySeconds * 1000).toLong())
    }

    /** JNI functions */
    external fun executeCallbackFromJNI(callbackPtr: Long)
}
