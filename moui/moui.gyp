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
        'ui/base_view.cc',
      ],
      'include_dirs': [ '..' ] ,
      'direct_dependent_settings': {
        'include_dirs': [ '..' ],
        'conditions': [
          ['OS=="android"', {
            'defines': [ 'MOUI_ANDROID_PLATFORM' ],
          }],
          ['OS=="ios"', {
            'defines': [ 'MOUI_IOS_PLATFORM' ],
          }],
          ['OS=="mac"', {
            'defines': [ 'MOUI_MAC_PLATFORM' ],
          }],
          ['OS=="win"', {
            'defines': [ 'MOUI_WINDOWS_PLATFORM' ],
          }],
        ],  # conditions
      },
      'conditions': [
        ['OS=="android"', {
          'defines': [ 'MOUI_ANDROID_PLATFORM' ],
        }],
        ['OS=="ios"', {
          'sources': [
            'core/apple/path.mm',
            'ui/ios/MOOpenGLView.mm',
            'ui/ios/MOOpenGLViewController.mm',
            'ui/ios/native_view.mm',
            'ui/ios/view.mm',
          ],
          'defines': [ 'MOUI_IOS_PLATFORM' ],
          'libraries': [
            '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
            '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
          ],
        }],
        ['OS=="mac"', {
          'defines': [ 'MOUI_MAC_PLATFORM' ],
        }],
        ['OS=="win"', {
          'defines': [ 'MOUI_WINDOWS_PLATFORM' ],
        }],
      ],  # conditions
    }  # libmoui
  ],  # targets
}
