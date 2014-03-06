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

import android.app.Activity;
import android.util.DisplayMetrics;
import java.lang.Math;
import java.lang.Object;

public class Device extends Object {

  private Activity mActivity = null;

  public Device(Activity activity) {
    super();
    mActivity = activity;
  }

  // Returns the logical density of the screen.
  public float getScreenScaleFactor() {
    // DisplayMetrics metrics = new DisplayMetrics();
    // mActivity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
    DisplayMetrics metrics = mActivity.getResources().getDisplayMetrics();
    return metrics.density;
  }

  // Returns the smallest screen size an application will see in normal
  // operation, corresponding to smallest screen width resource qualifier.
  // This is the smallest value of both screen width dp and screen height dp
  // in both portrait and landscape.
  public float getSmallestScreenWidthDp() {
    DisplayMetrics metrics = mActivity.getResources().getDisplayMetrics();
    int smallestWidthPixels = Math.min(metrics.widthPixels,
                                       metrics.heightPixels);
    return smallestWidthPixels / metrics.density;
  }
}
