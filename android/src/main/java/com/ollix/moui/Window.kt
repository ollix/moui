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

import android.app.Activity
import android.view.View
import android.view.ViewGroup
import android.view.Window

class Window() {

    /** Returns the window of the current activity. */
    fun getWindow(activity: Activity): Window {
        return activity.window
    }

    /**
     * Returns the root view of the specified window that does not include the
     * status bar.
     */
    fun getRootView(activity: Activity): View? {
        val viewGroup: ViewGroup = activity.findViewById(android.R.id.content)
        return viewGroup.getChildAt(0)
    }
}
