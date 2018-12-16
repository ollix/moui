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

// This is the base class of layout classes. Each layout class is designed to
// arrange child widgets in a particular manner.
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

  // Overrides `Widget` class.
  void AddChild(Widget* child) override;

  // Inherited from `Widget` class.
  void Redraw() override;

  // Resets cells.
  void ResetCells();

  // Accessors and setters.
  bool adjusts_size_to_fit_contents() const {
    return adjusts_size_to_fit_contents_;
  }
  void set_adjusts_size_to_fit_contents(const bool value) {
    adjusts_size_to_fit_contents_ = value;
  }
  std::vector<Widget*>* children();
  float spacing() const { return spacing_; }
  void set_spacing(const float spacing);

 protected:
  // The state of managed widget.
  struct ManagedWidget {
    // The actual widget to be displayed on screen.
    Widget* widget;
    // The required size to display the widget.
    Size occupied_size;
    // The cell that wraps the widget.
    Widget* cell;
  };
  typedef std::vector<ManagedWidget> ManagedWidgetVector;

  // Updates the content view's size. The specified values should be able to
  // display all managed cells. If `adjusts_size_to_fit_contents_` is set to
  // `ture`, the size of the layout itself will be changed as well.
  void UpdateContentSize(const float width, const float height);

  // Inherited from `Widget` class.
  bool WidgetViewWillRender(NVGcontext* context) override;

 private:
  // Arranges cells. This method must be implemented in subclasses.
  virtual void ArrangeCells(const ManagedWidgetVector managed_widgets) = 0;

  // Returns a list of cells that contains the managed widgets.
  std::vector<Widget*> GetCells();

  // Returns `true` if cells should be rearranged.
  bool ShouldRearrangeCells();

  // Indicates whether the layout's size should be adjusted automatically to
  // fit its contents.
  bool adjusts_size_to_fit_contents_;

  // Keeps the strong references to all cells.
  std::vector<Widget*> all_cells_;

  // Keeps a list of weak references to the actual widgets to be layouted.
  std::vector<Widget*> managed_children_;

  // Keeps the states of currently managed widgets.
  std::vector<ManagedWidget> managed_widgets_;

  // Indicates whether cells should be rearranged.
  bool should_rearrange_cells_;

  // The space in ponits between child widgets.
  float spacing_;

  DISALLOW_COPY_AND_ASSIGN(Layout);
};

}  // namespace moui

#endif  // MOUI_WIDGETS_LAYOUT_H_
