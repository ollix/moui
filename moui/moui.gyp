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
        'nanovg_hook.cc',
        'ui/base_view.cc',
        'ui/base_window.cc',
        'ui/android/native_view_android.cc',
        'ui/android/view_android.cc',
        'ui/android/window_android.cc',
        'ui/ios/native_view_ios.mm',
        'ui/ios/opengl_view_ios.mm',
        'ui/ios/opengl_view_controller_ios.mm',
        'ui/ios/view_ios.mm',
        'ui/ios/window_ios.mm',
        'ui/mac/opengl_view_mac.mm',
        'ui/mac/native_view_mac.mm',
        'ui/mac/view_mac.mm',
        'ui/mac/window_mac.mm',
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
        'widgets/widget.cc',
        'widgets/widget_view.cc',
      ],
      'include_dirs': [
        '..',
        'deps',
      ],
      'conditions': [
        ['OS == "android"', {
          'defines': [
            'MOUI_GLES2',
            'MOUI_ANDROID',
          ],
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
            'ui/android/native_view_android.cc',
            'ui/android/view_android.cc',
            'ui/android/window_android.cc',
          ],
        }],
        ['OS == "ios"', {
          'defines': [
            'MOUI_APPLE',
            'MOUI_GLES2',
            'MOUI_IOS',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGLES.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
            ],
          },
        }, {  # OS != "ios"
          'sources!': [
            'ui/ios/native_view_ios.mm',
            'ui/ios/opengl_view_ios.mm',
            'ui/ios/opengl_view_controller_ios.mm',
            'ui/ios/view_ios.mm',
            'ui/ios/window_ios.mm',
          ],
        }],
        ['OS == "mac"', {
          'defines': [
            'MOUI_APPLE',
            'MOUI_GL2',
            'MOUI_MAC',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
            ],
          },
        }, {  # OS != "mac"
          'sources!': [
            'ui/mac/opengl_view_mac.mm',
            'ui/mac/native_view_mac.mm',
            'ui/mac/view_mac.mm',
            'ui/mac/window_mac.mm',
          ],
        }],
        ['OS == "win"', {
          'defines': [ 'MOUI_WINDOWS' ],
        }],
        ['OS != "mac" and OS != "ios"', {
          'sources!': [
            'core/apple/clock_apple.mm',
            'core/apple/device_apple.mm',
            'core/apple/path_apple.mm',
          ],
        }],
      ],  # conditions
      'dependencies': [
        'nanovg.gyp:libnanovg',
      ],
      'export_dependent_settings': [
        'nanovg.gyp:libnanovg',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
          'deps',
        ],
        'conditions': [
          ['OS == "android"', {
            'defines': [
              'MOUI_GLES2',
              'MOUI_ANDROID',
            ],
          }],
          ['OS == "ios"', {
            'defines': [
              'MOUI_APPLE',
              'MOUI_GLES2',
              'MOUI_IOS',
            ],
          }],
          ['OS == "mac"', {
            'defines': [
              'MOUI_APPLE',
              'MOUI_GL2',
              'MOUI_MAC',
            ],
          }],
          ['OS == "win"', {
            'defines': [
              'MOUI_WINDOWS',
            ],
          }],
        ],  # conditions
      },  # direct_dependent_settings
    },  # libmoui target
  ],  # targets
}
