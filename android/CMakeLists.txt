# Copyright 2017 Ollix
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.10.2)

add_library(moui-jni
            SHARED
            "src/main/cpp/clock.cc"
            "src/main/cpp/fragment.cc"
            "src/main/cpp/view.cc")

add_subdirectory("../moui" "native-moui")
target_link_libraries(moui-jni LINK_PUBLIC moui)
