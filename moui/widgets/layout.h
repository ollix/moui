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

  // Accessors and setters.
  float bottom_padding() const { return bottom_padding_; }
  void set_bottom_padding(const float padding);
  float left_padding() const { return left_padding_; }
  void set_left_padding(const float padding);
  float right_padding() const { return right_padding_; }
  void set_right_padding(const float padding);
  float spacing() const { return spacing_; }
  void set_spacing(const float spacing);
  float top_padding() const { return top_padding_; }
  void set_top_padding(const float padding);

 protected:
  // The state of managed widget.
  struct ManagedWidget {
    Point origin;
    Size size;
    Widget* widget;
  };

  // Keeps the states of currently managed widgets.
  std::vector<ManagedWidget> managed_widgets_;

  // Inherited from Widget class.
  virtual bool WidgetViewWillRender(NVGcontext* context) override;

 private:
  // Arranges child widgets. This method must be implmented in subclasses.
  virtual void ArrangeChildren() {}

  // Returns true if child widgets should be arranged.
  bool ShouldArrangeChildren();

  // The padding in points to the bottom side of the layout.
  float bottom_padding_;

  // The padding in points to the left side of the layout.
  float left_padding_;

  // The padding in points to the right side of the layout.
  float right_padding_;

  // Indicates whether child widgets should be arranged.
  bool should_arrange_children_;

  // The space in ponits between child widgets.
  float spacing_;

  // The padding in points to the top side of the layout.
  float top_padding_;

  DISALLOW_COPY_AND_ASSIGN(Layout);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_LAYOUT_H_
