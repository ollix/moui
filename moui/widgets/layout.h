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

#ifndef MOUI_WIDGETS_LAYOUT_H_
#define MOUI_WIDGETS_LAYOUT_H_

#include <vector>

#include "moui/base.h"
#include "moui/widgets/scroll_view.h"
#include "moui/widgets/widget.h"

namespace moui {

// This is the base class for implementing every particular layout. Each layout
// is used to arrange child widgets in a particular manner.
class Layout : public ScrollView {
 public:
  // The orientation represents whether the layout's children should be
  // arranged vertically or horizontally.
  enum class Orientation {
    kHorizontal,
    kVertical,
  };

  Layout();
  ~Layout();

 private:
  // The state of managed widget.
  struct ManagedWidget {
    Point origin;
    Size size;
    Widget* widget;
  };

  // Arranges child widgets. This method must be implmented in subclasses.
  virtual void ArrangeChildren() {}

  // Returns true if child widgets should be arranged.
  bool ShouldArrangeChildren();

  // Inherited from Widget class.
  virtual void WidgetViewDidRender(NVGcontext* context) override final;

  // Inherited from Widget class.
  virtual void WidgetViewWillRender(NVGcontext* context) override final;

  // Keeps the states of currently managed widgets.
  std::vector<ManagedWidget> managed_widgets_;

  // Indicates whether child widgets should be arranged.
  bool should_arrange_children_;

  DISALLOW_COPY_AND_ASSIGN(Layout);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_LAYOUT_H_
