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
  # Project-level configurations.
  'configurations': {
    'Debug': {
      'xcode_settings': {
        'GCC_OPTIMIZATION_LEVEL': '0',
        'ONLY_ACTIVE_ARCH': 'YES',
      },
    },
    'Release': {
      'xcode_settings': {
        'GCC_OPTIMIZATION_LEVEL': 's',
      },
    },
  },
  # Target-level configurations.
  'target_defaults': {
    'default_configuration': 'Debug',
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG' ],
      },
      'Release': {
        'defines': [ 'NDEBUG' ],
      },
    },
    'conditions': [
      ['OS == "ios"', {
        'xcode_settings': {
          'IPHONEOS_DEPLOYMENT_TARGET': '9.0',
          'SDKROOT': 'iphoneos',  # -isysroot
        },
      }],  # OS == "ios"
      ['OS == "mac"', {
        'xcode_settings': {
          'COMBINE_HIDPI_IMAGES': 'YES',
          'SDKROOT': 'macosx',  # -isysroot
        },
      }],  # OS == "mac"
    ],  # conditions
    'xcode_settings': {
      'OTHER_CPLUSPLUSFLAGS': [
        '-std=c++11',  # supports C++11
      ],
      'SKIP_INSTALL': 'YES',
    },  # xcode_settings
  },  # target_defaults
}
