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
{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'libmoui',
      'type': 'static_library',
      'sources': [
        'core/base_application.cc',
        'ui/base_view.cc',
        'ui/base_window.cc',
        'widgets/widget.cc',
        'widgets/widget_view.cc',
      ],
      'include_dirs': [ '..' ],
      'conditions': [
        ['OS=="android"', {
          'defines': [
            'MOUI_GLES2',
            'MOUI_ANDROID' ,
          ],
          'sources': [
            'core/android/application.cc',
            'core/android/clock.cc',
            'ui/android/native_view.cc',
            'ui/android/view.cc',
            'ui/android/window.cc',
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
        }],
        ['OS=="ios"', {
          'defines': [
            'MOUI_GLES2',
            'MOUI_IOS'
          ],
          'sources': [
            'core/apple/clock.mm',
            'core/apple/path.mm',
            'ui/ios/MOOpenGLView.mm',
            'ui/ios/MOOpenGLViewController.mm',
            'ui/ios/native_view.mm',
            'ui/ios/view.mm',
            'ui/ios/window.mm',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGLES.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
            ],
          },
        }],
        ['OS=="mac"', {
          'defines': [
            'MOUI_GL2',
            'MOUI_MAC'
          ],
          'sources': [
            'core/apple/clock.mm',
            'core/apple/path.mm',
            'ui/mac/MOOpenGLView.mm',
            'ui/mac/native_view.mm',
            'ui/mac/view.mm',
            'ui/mac/window.mm',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
            ],
          },
        }],
        ['OS=="win"', {
          'defines': [ 'MOUI_WINDOWS' ],
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
          '..'
        ],
        'conditions': [
          ['OS=="android"', {
            'defines': [ 'MOUI_ANDROID' ],
          }],
          ['OS=="ios"', {
            'defines': [ 'MOUI_IOS' ],
          }],
          ['OS=="mac"', {
            'defines': [ 'MOUI_MAC' ],
          }],
          ['OS=="win"', {
            'defines': [ 'MOUI_WINDOWS' ],
          }],
        ],  # conditions
      },  # direct_dependent_settings
    },  # libmoui target
  ],  # targets
}
