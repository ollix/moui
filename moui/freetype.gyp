# Copyright 2016 Ollix
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
      'target_name': 'libfreetype',
      'type': 'static_library',
      'sources': [
        '<!@(ls -1 deps/freetype/src/*/*.c)',
      ],
      # Skips files that may cause compile errors.
      'sources!': [
        'deps/freetype/src/autofit/aflatin2.c',
        'deps/freetype/src/autofit/afshaper.c',
        'deps/freetype/src/gxvalid/gxvfgen.c',
        'deps/freetype/src/truetype/ttinterp.c',
        '<!@(ls -1 deps/freetype/src/gzip/*.c)',
        '<!@(ls -1 deps/freetype/src/tools/*.c)',
      ],
      'include_dirs': [
        'deps/freetype/include',
      ],
      "direct_dependent_settings": {
        "include_dirs": [
          'deps/freetype/include',
        ],
      },
      'defines': [
        'FT2_BUILD_LIBRARY=1',
        'DARWIN_NO_CARBON',
      ],
      'conditions': [
        ['OS != "mac"', {
          'sources!': [
            'deps/freetype/src/base/ftmac.c',
          ],
        }],  # mac
        ['OS == "ios"', {
          'xcode_settings': {
            'OTHER_CFLAGS': [
              '-fpascal-strings',
              '-fvisibility=hidden',
              '-O2',
              '-pipe',
              '-std=c99',
              '-w',
            ],
          },  # xcode_settings
        }],  # ios
      ],  # conditions
    },
  ],
}
