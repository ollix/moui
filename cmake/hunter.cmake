# Copyright (c) 2019 Ollix. All rights reserved.
#
# ---
# Author: olliwang@ollix.com (Olli Wang)

if (ANDROID)
  set(gen_toolchain "${CMAKE_CURRENT_BINARY_DIR}/generated/toolchain.cmake")
  configure_file("${CMAKE_CURRENT_LIST_DIR}/android_toolchain.cmake"
                 "${gen_toolchain}" @ONLY)
  set(CMAKE_TOOLCHAIN_FILE "${gen_toolchain}" CACHE PATH "" FORCE)
endif(ANDROID)

include("${CMAKE_CURRENT_LIST_DIR}/HunterGate.cmake")
HunterGate(URL "https://github.com/cpp-pm/hunter/archive/v0.23.312.tar.gz"
           SHA1 "42311324f6b859983d833cffc2151d0b86b8bea4")
