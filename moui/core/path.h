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

#ifndef MOUI_CORE_PATH_H_
#define MOUI_CORE_PATH_H_

#include <string>

#include "moui/base.h"

namespace moui {

class Path {
 public:
  enum class Directory {
#ifdef MOUI_APPLE
    kLibrary,
    kResource,
#endif
  };

  Path() {}
  ~Path() {}

  // Returns the direcotry path containing app resources.
  static std::string GetDirectory(const Directory directory);

 private:
  DISALLOW_COPY_AND_ASSIGN(Path);
};

}  // namespace moui

#endif  // MOUI_CORE_PATH_H_
