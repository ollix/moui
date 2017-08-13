# Copyright 2014 Ollix
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
#
# ---
# Author: olliwang@ollix.com (Olli Wang)

{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'libmoui',
      'type': 'static_library',
      'sources': [
        'core/android/application_android.cc',
        'core/android/clock_android.cc',
        'core/android/device_android.cc',
        'core/apple/clock_apple.mm',
        'core/apple/device_apple.mm',
        'core/apple/path_apple.mm',
        'core/base_application.cc',
        'core/event.cc',
        'native/android/native_object_android.cc',
        'native/android/native_view_android.cc',
        'native/android/native_window_android.cc',
        'native/ios/native_application_ios.mm',
        'native/ios/native_object_ios.mm',
        'native/ios/native_view_ios.mm',
        'native/ios/native_view_controller_ios.mm',
        'native/ios/native_window_ios.mm',
        'native/mac/native_object_mac.mm',
        'native/mac/native_view_mac.mm',
        'native/mac/native_window_mac.mm',
        'nanovg_hook.cc',
        'ui/base_view.cc',
        'ui/base_window.cc',
        'ui/android/view_android.cc',
        'ui/android/window_android.cc',
        'ui/ios/MOMetalView.mm',
        'ui/ios/MOOpenGLView.mm',
        'ui/ios/MOView.mm',
        'ui/ios/view_ios.mm',
        'ui/ios/window_ios.mm',
        'ui/mac/MOMetalView.mm',
        'ui/mac/MOOpenGLView.mm',
        'ui/mac/MOView.mm',
        'ui/mac/view_mac.mm',
        'ui/mac/window_mac.mm',
        'widgets/activity_indicator_view.cc',
        'widgets/button.cc',
        'widgets/control.cc',
        'widgets/grid_layout.cc',
        'widgets/label.cc',
        'widgets/layout.cc',
        'widgets/linear_layout.cc',
        'widgets/page_control.cc',
        'widgets/progress_view.cc',
        'widgets/scroll_view.cc',
        'widgets/scroller.cc',
        'widgets/switch.cc',
        'widgets/table_view.cc',
        'widgets/table_view_cell.cc',
        'widgets/widget.cc',
        'widgets/widget_view.cc',
      ],
      'include_dirs': [
        '..',
        'deps',
      ],
      'conditions': [
        ['OS == "android"', {
          'variables': {
            'defines': [
            'MOUI_GLES2',
            'MOUI_ANDROID',
            ],
          },
          'ldflags': [
            '-lGLESv2',
            '-llog',
          ],
          'direct_dependent_settings': {
            'ldflags': [
              '-lGLESv2',
              '-llog',
            ],
          },
        }, {  # OS != "android"
          'sources!': [
            'core/android/application_android.cc',
            'core/android/clock_android.cc',
            'core/android/device_android.cc',
            'native/android/native_object_android.cc',
            'native/android/native_view_android.cc',
            'native/android/native_window_android.cc',
            'ui/android/view_android.cc',
            'ui/android/window_android.cc',
          ],
        }],
        ['OS == "ios"', {
          'variables': {
            'defines': [
              'MOUI_METAL',
              # 'MOUI_GLES2',
              'MOUI_IOS',
            ],
          },
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/Metal.framework',
              # '$(SDKROOT)/System/Library/Frameworks/OpenGLES.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
            ],
          },
        }, {  # OS != "ios"
          'sources!': [
            'native/ios/native_application_ios.mm',
            'native/ios/native_object_ios.mm',
            'native/ios/native_view_ios.mm',
            'native/ios/native_view_controller_ios.mm',
            'native/ios/native_window_ios.mm',
            'ui/ios/MOMetalView.mm',
            'ui/ios/MOOpenGLView.mm',
            'ui/ios/MOView.mm',
            'ui/ios/view_ios.mm',
            'ui/ios/window_ios.mm',
          ],
        }],
        ['OS == "mac"', {
          'variables': {
            'defines': [
              'MOUI_METAL',
              # 'MOUI_GL2',
              'MOUI_MAC',
            ],
          },
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreVideo.framework',
              '$(SDKROOT)/System/Library/Frameworks/Metal.framework',
              # '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
            ],
          },
        }, {  # OS != "mac"
          'sources!': [
            'native/mac/native_object_mac.mm',
            'native/mac/native_view_mac.mm',
            'native/mac/native_window_mac.mm',
            'ui/mac/MOMetalView.mm',
            'ui/mac/MOOpenGLView.mm',
            'ui/mac/MOView.mm',
            'ui/mac/view_mac.mm',
            'ui/mac/window_mac.mm',
          ],
        }],
        ['OS == "win"', {
          'variables': {
            'defines': [
              'MOUI_WINDOWS',
            ],
          },
        }],
        ['OS == "mac" or OS == "ios"', {
          'xcode_settings': {
            'CLANG_ENABLE_OBJC_ARC': 'YES',
          },
        }, {
          'sources!': [
            'core/apple/clock_apple.mm',
            'core/apple/device_apple.mm',
            'core/apple/path_apple.mm',
          ],
        }],
      ],  # conditions
      'defines': [
        '<@(defines)',
      ],
      'dependencies': [
        'nanovg.gyp:libnanovg',
      ],
      'export_dependent_settings': [
        'nanovg.gyp:libnanovg',
      ],
      'direct_dependent_settings': {
        'defines': [
          '<@(defines)',
        ],
        'include_dirs': [
          '..',
          'deps',
        ],
      },  # direct_dependent_settings
    },  # libmoui target
  ],  # targets
}
