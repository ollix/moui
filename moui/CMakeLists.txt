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

cmake_minimum_required(VERSION 3.4.1)

# Freetype Library

file(GLOB FREETYPE_SOURCE_FILES
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "deps/freetype/src/*/*.c")
list(REMOVE_ITEM FREETYPE_SOURCE_FILES
     "deps/freetype/src/autofit/aflatin2.c"
     "deps/freetype/src/autofit/afshaper.c"
     "deps/freetype/src/base/ftmac.c"
     "deps/freetype/src/gxvalid/gxvfgen.c"
     "deps/freetype/src/truetype/ttinterp.c")
list(FILTER FREETYPE_SOURCE_FILES EXCLUDE REGEX "/gzip/")
list(FILTER FREETYPE_SOURCE_FILES EXCLUDE REGEX "/tools/")

add_library(freetype STATIC ${FREETYPE_SOURCE_FILES})

target_include_directories(freetype PUBLIC "deps/freetype/include")

target_compile_definitions(freetype
    PRIVATE
    "FT2_BUILD_LIBRARY=1"
    "DARWIN_NO_CARBON")

target_compile_options(freetype
    PRIVATE
    "-fpascal-strings"
    "-fvisibility=hidden"
    "-O2"
    "-pipe"
    "-std=c99"
    "-w")

# NanoVG Library

add_library(nanovg STATIC "deps/nanovg/src/nanovg.c")

target_compile_definitions(nanovg PUBLIC "FONS_USE_FREETYPE")

target_include_directories(nanovg
    PUBLIC "deps/nanovg/src"
    PRIVATE "deps/3rdparty")  # for freetype

target_link_libraries(nanovg PUBLIC freetype)

if(CMAKE_OSX_SYSROOT STREQUAL "iphoneos" OR CMAKE_OSX_SYSROOT STREQUAL "macosx")
    target_sources(nanovg PRIVATE "deps/MetalNanoVG/src/nanovg_mtl.m")
    target_link_libraries(nanovg PRIVATE "-framework Metal")
endif()

# Moui Library

add_library(moui
    STATIC
    "core/base_application.cc"
    "core/event.cc"
    "nanovg_hook.cc"
    "ui/base_view.cc"
    "ui/base_window.cc"
    "widgets/activity_indicator_view.cc"
    "widgets/button.cc"
    "widgets/control.cc"
    "widgets/grid_layout.cc"
    "widgets/label.cc"
    "widgets/layout.cc"
    "widgets/linear_layout.cc"
    "widgets/page_control.cc"
    "widgets/progress_view.cc"
    "widgets/scroll_view.cc"
    "widgets/scroller.cc"
    "widgets/switch.cc"
    "widgets/table_view.cc"
    "widgets/table_view_cell.cc"
    "widgets/widget.cc"
    "widgets/widget_view.cc")

set_target_properties(moui
    PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")

target_include_directories(moui PUBLIC ".." "deps")

target_link_libraries(moui PUBLIC nanovg)

# Android
if(DEFINED ANDROID_PLATFORM)
    target_compile_definitions(moui
        PUBLIC
        "MOUI_ANDROID"
        "MOUI_GLES2")

    add_subdirectory(deps/aasset)
    target_link_libraries(moui
        PUBLIC
        log
        PRIVATE
        aasset
        android
        GLESv2)

    target_sources(moui
        PRIVATE
        "core/android/application_android.cc"
        "core/android/clock_android.cc"
        "core/android/device_android.cc"
        "core/android/path_android.cc"
        "native/android/native_object_android.cc"
        "native/android/native_view_android.cc"
        "native/android/native_window_android.cc"
        "ui/android/view_android.cc"
        "ui/android/window_android.cc")
# iOS
elseif(CMAKE_OSX_SYSROOT STREQUAL "iphoneos")
    target_compile_definitions(moui
        PUBLIC
        "MOUI_IOS"
        "MOUI_METAL")

    target_link_libraries(moui
        PRIVATE
        "-framework CoreGraphics"
        "-framework Foundation"
        "-framework Metal"
        "-framework QuartzCore"
        "-framework UIKit")

    target_sources(moui
        PRIVATE
        "core/apple/clock_apple.mm"
        "core/apple/device_apple.mm"
        "core/apple/path_apple.mm"
        "native/ios/native_application_ios.mm"
        "native/ios/native_object_ios.mm"
        "native/ios/native_view_ios.mm"
        "native/ios/native_view_controller_ios.mm"
        "native/ios/native_window_ios.mm"
        "ui/ios/MOMetalView.mm"
        "ui/ios/MOOpenGLView.mm"
        "ui/ios/MOView.mm"
        "ui/ios/view_ios.mm"
        "ui/ios/window_ios.mm")
# Mac
elseif(CMAKE_OSX_SYSROOT STREQUAL "macosx")
    target_compile_definitions(moui
        PUBLIC
        "MOUI_MAC"
        "MOUI_METAL")

    target_link_libraries(moui
        PRIVATE
        "-framework Cocoa"
        "-framework CoreVideo"
        "-framework Foundation"
        "-framework Metal"
        "-framework QuartzCore")

    target_sources(moui
        PRIVATE
        "core/apple/clock_apple.mm"
        "core/apple/device_apple.mm"
        "core/apple/path_apple.mm"
        "native/mac/native_object_mac.mm"
        "native/mac/native_view_mac.mm"
        "native/mac/native_window_mac.mm"
        "ui/mac/MOMetalView.mm"
        "ui/mac/MOOpenGLView.mm"
        "ui/mac/MOView.mm"
        "ui/mac/view_mac.mm"
        "ui/mac/window_mac.mm")
endif()
