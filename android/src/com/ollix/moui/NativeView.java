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

import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import java.lang.Object;

public class NativeView extends Object {

  public NativeView() {
    super();
  }

  // Adds subview to the parentView if the parent is an instance of GroupView.
  public void addSubview(View parentView, View subview) {
    if (parentView instanceof ViewGroup) {
      ViewGroup viewGroup = (ViewGroup)parentView;
      viewGroup.addView(subview);
    }
  }

  // Sets the position and dimension of the view.
  public void setBounds(View view, int x, int y, int width, int height) {
    FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(
        width, height, Gravity.TOP);
    layoutParams.leftMargin = x;
    layoutParams.topMargin = y;
    view.setLayoutParams(layoutParams);
  }
}
