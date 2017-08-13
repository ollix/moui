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
      'target_name': 'libnanovg',
      'type': 'static_library',
      'sources': [
        'deps/nanovg/src/nanovg.c',
        'deps/MetalNanoVG/src/nanovg_mtl.m',
      ],
      'include_dirs': [
        'deps/nanovg/src',
        'deps/MetalNanoVG/src',
        'deps/3rdparty',  # for freetype configuration
      ],
      'defines': [
        'FONS_USE_FREETYPE',
      ],
      'dependencies': [
        'freetype.gyp:libfreetype',
      ],
      "direct_dependent_settings": {
        "include_dirs": [
          'deps/nanovg/src',
          'deps/MetalNanoVG/src',
        ],
      },
      'conditions': [
        ['OS == "mac" or OS == "ios"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Metal.framework',
            ],
          },
        }, {
          'sources!': [
            'deps/MetalNanoVG/src/nanovg_mtl.m',
          ],
        }],
      ],  # conditions
    },
  ],
}
