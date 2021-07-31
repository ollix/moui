# Copyright (c) 2018, Ruslan Baratov
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# For the latest version of this file and working example see:

# For the example and for the latest version of this file check:
# * https://github.com/forexample/android-studio-with-hunter

# Hunter need stand-alone toolchain that will not depend
# on any ANDROID_* variables:
# * https://docs.hunter.sh/en/latest/overview/customization/toolchain-id.html

# To achieve this we need to make a 3 steps:
# * Lock CMake variables that coming from Gradle build
# * Include original Android toolchain from NDK
# * Add any custom user variables

# STEP 1: Lock variables from Gradle build

set(ANDROID_ABI "@ANDROID_ABI@")
set(ANDROID_PLATFORM "@ANDROID_PLATFORM@")
set(ANDROID_STL "@ANDROID_STL@")
set(CMAKE_ANDROID_ARCH_ABI "@CMAKE_ANDROID_ARCH_ABI@")
set(CMAKE_SYSTEM_NAME "Android")
set(CMAKE_SYSTEM_VERSION "@CMAKE_SYSTEM_VERSION@")

# STEP 2: Include original toolchain

# Original toolchain file will be saved in variable:
# * _INTERNAL_ANDROID_STUDIO_ORIGINAL_CMAKE_TOOLCHAIN_FILE

if("@_INTERNAL_ANDROID_STUDIO_ORIGINAL_CMAKE_TOOLCHAIN_FILE@" STREQUAL "")
  # Variable not set, initial project configuration.
  # Path to original Android NDK toolchain saved in CMAKE_TOOLCHAIN_FILE.
  if("@CMAKE_TOOLCHAIN_FILE@" STREQUAL "")
    message(FATAL_ERROR "Unexpected: CMAKE_TOOLCHAIN_FILE is empty")
  endif()

  if(NOT EXISTS "@CMAKE_TOOLCHAIN_FILE@")
    message(
        FATAL_ERROR "File not found: '@CMAKE_TOOLCHAIN_FILE@'"
    )
  endif()

  set(_original_toolchain_file "@CMAKE_TOOLCHAIN_FILE@")
else()
  set(_original_toolchain_file "@_INTERNAL_ANDROID_STUDIO_ORIGINAL_CMAKE_TOOLCHAIN_FILE@")
endif()

set(
    _INTERNAL_ANDROID_STUDIO_ORIGINAL_CMAKE_TOOLCHAIN_FILE
    "${_original_toolchain_file}"
    CACHE
    PATH
    "Original Android NDK toolchain"
)

include("${_INTERNAL_ANDROID_STUDIO_ORIGINAL_CMAKE_TOOLCHAIN_FILE}")

# STEP 3: Custom user variables

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
