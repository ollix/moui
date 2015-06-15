#!/bin/bash
# Copyright (c) 2015 Ollix. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ---
# Author: olliwang@ollix.com (Olli Wang)
#
# This script checks if the moui source files match Google's C++ style guide.
# 'cclint' is required to run this script: https://github.com/ollix/cclint

BASEDIR=$(dirname $0)/../
cd $BASEDIR

cclint --expanddir=recursive \
       --excludedir=moui/deps \
       --excludedir=moui/*/apple \
       --excludedir=moui/*/ios \
       --excludedir=moui/*/mac \
       moui/
