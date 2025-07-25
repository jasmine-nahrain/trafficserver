#######################
#
#  Licensed to the Apache Software Foundation (ASF) under one or more contributor license
#  agreements.  See the NOTICE file distributed with this work for additional information regarding
#  copyright ownership.  The ASF licenses this file to you under the Apache License, Version 2.0
#  (the "License"); you may not use this file except in compliance with the License.  You may obtain
#  a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software distributed under the License
#  is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
#  or implied. See the License for the specific language governing permissions and limitations under
#  the License.
#
#######################

add_library(
  hdrs STATIC
  HTTP.cc
  HdrHeap.cc
  HdrTSOnly.cc
  HdrToken.cc
  HdrUtils.cc
  HttpCompat.cc
  MIME.cc
  URL.cc
  VersionConverter.cc
  HuffmanCodec.cc
  XPACK.cc
  HeaderValidator.cc
)
add_library(ts::hdrs ALIAS hdrs)

target_link_libraries(
  hdrs
  PUBLIC libswoc::libswoc ts::tscore
  PRIVATE ts::inkevent ts::tsutil lshpack
)

# For ls-hpack includes.
target_include_directories(hdrs PRIVATE ${CMAKE_SOURCE_DIR}/lib)

if(BUILD_TESTING)
  add_executable(
    test_proxy_hdrs
    unit_tests/test_HdrHeap.cc
    unit_tests/test_Hdrs.cc
    unit_tests/test_HdrUtils.cc
    unit_tests/test_HdrHeap.cc
    unit_tests/test_HeaderValidator.cc
    unit_tests/test_Huffmancode.cc
    unit_tests/test_mime.cc
    unit_tests/test_URL.cc
    unit_tests/unit_test_main.cc
  )
  target_link_libraries(
    test_proxy_hdrs PRIVATE ts::hdrs ts::tscore ts::inkevent libswoc::libswoc catch2::catch2 lshpack
  )
  add_test(NAME test_proxy_hdrs COMMAND test_proxy_hdrs)

  add_executable(test_proxy_hdrs_xpack unit_tests/test_XPACK.cc)
  target_link_libraries(
    test_proxy_hdrs_xpack PRIVATE ts::hdrs ts::tscore ts::tsutil libswoc::libswoc catch2::catch2 lshpack
  )
  add_test(NAME test_proxy_hdrs_xpack COMMAND test_proxy_hdrs_xpack)
endif()

clang_tidy_check(hdrs)
