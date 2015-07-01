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

#ifndef MOUI_WIDGETS_LINEAR_LAYOUT_H_
#define MOUI_WIDGETS_LINEAR_LAYOUT_H_

#include "moui/base.h"
#include "moui/widgets/layout.h"

namespace moui {

class LinearLayout : public Layout {
 public:
  explicit LinearLayout(const Orientation orientation);
  ~LinearLayout();

 private:
  // Inherited from `Layout` class.
  void ArrangeCells(const ManagedWidgetVector managed_widgets) final;

  // The direction to arrange the child widgets.
  Orientation orientation_;

  DISALLOW_COPY_AND_ASSIGN(LinearLayout);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_LINEAR_LAYOUT_H_
