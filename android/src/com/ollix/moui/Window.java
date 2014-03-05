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
import android.R;
import android.view.View;
import android.view.ViewGroup;
import java.lang.Object;

public class Window extends Object {

  private Activity mActivity = null;

  public Window(Activity activity) {
    super();
    mActivity = activity;
  }

  // Returns the current window of the current activity.
  public android.view.Window getMainWindow() {
    return mActivity.getWindow();
  }

  // Returns the root view of the specified window that does not include the
  // status bar.
  public View getRootView(android.view.Window window) {
    ViewGroup contentView = (ViewGroup)window.getDecorView().findViewById(
        android.R.id.content);
    return contentView.getChildAt(0);
  }
}
