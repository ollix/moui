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

#include "moui/core/path.h"

#include <string>

#import <Foundation/Foundation.h>

namespace moui {

std::string Path::GetDirectory(const Directory directory) {
  // Temporary directory.
  if (directory == Directory::kTemporary) {
    return [NSTemporaryDirectory() UTF8String];
  }

  // Resource directory.
  if (directory == Directory::kResource) {
    NSBundle* bundle = [NSBundle mainBundle];
    if (bundle == nil)
      return "";
    return std::string([[bundle resourcePath] UTF8String]);
  }

  // Other directories.
  NSSearchPathDirectory search_path_directory;
  switch(directory) {
    case Directory::kDocument:
      search_path_directory = NSDocumentDirectory;
      break;
    case Directory::kLibrary:
      search_path_directory = NSLibraryDirectory;
      break;
    default:
      return "";
  }
  NSArray* results = [[NSFileManager defaultManager]
      URLsForDirectory:search_path_directory inDomains:NSUserDomainMask];
  return [[[results lastObject] path] UTF8String];
}

void Path::Reset() {
}

}  // namespace moui
