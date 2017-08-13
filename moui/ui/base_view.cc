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

#include "moui/ui/base_view.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "moui/native/native_view.h"

namespace moui {

BaseView::BaseView() : NativeView(nullptr), animation_count_(0) {
}

BaseView::~BaseView() {
}

bool BaseView::IsAnimating() const {
  return animation_count_ > 0;
}

void BaseView::StartAnimation() {
  ++animation_count_;
  StartUpdatingNativeView();
}

void BaseView::StopAnimation() {
  --animation_count_;
  if (animation_count_ == 0)
    StopUpdatingNativeView();
  animation_count_ = std::max(0, animation_count_);
}

}  // namespace moui
