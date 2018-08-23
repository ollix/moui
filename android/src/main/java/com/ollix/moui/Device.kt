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
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.BatteryManager
import android.util.DisplayMetrics
import java.lang.Math

class Device(context: Context) {

    private val batteryStatusFilter =
            IntentFilter(Intent.ACTION_BATTERY_CHANGED);
    private val context: Context

    init {
        this.context = context
    }

    /** Returns the current battery state. */
    fun getBatteryState(): Int {
        val batteryStatus = context.registerReceiver(null, batteryStatusFilter)
        if (batteryStatus == null) {
            return 0
        }
        return batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1)
    }

    /** Returns the logical density of the screen. */
    fun getScreenScaleFactor(): Float {
        val metrics = context.getResources().getDisplayMetrics()
        return metrics.density
    }

    /**
     * Returns the smallest screen size an application will see in normal
     * operation, corresponding to smallest screen width resource qualifier.
     * This is the smallest value of both screen width dp and screen height dp
     * in both portrait and landscape.
     */
    fun getSmallestScreenWidthDp(): Float {
        val metrics = context.getResources().getDisplayMetrics()
        val smallestWidthPixels = Math.min(metrics.widthPixels,
                                           metrics.heightPixels)
        return smallestWidthPixels / metrics.density
    }
}
