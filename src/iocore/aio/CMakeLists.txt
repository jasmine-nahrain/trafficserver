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

add_library(aio STATIC)
add_library(ts::aio ALIAS aio)

target_sources(aio PRIVATE AIO.cc AIO_fault_injection.cc)

target_link_libraries(aio PUBLIC ts::inkevent ts::tscore)
if(TS_USE_LINUX_IO_URING)
  target_link_libraries(aio PUBLIC ts::inkuring)
endif()

if(BUILD_TESTING)
  add_executable(test_AIO test_AIO.cc)
  target_link_libraries(test_AIO ts::aio)
  add_test(
    NAME test_AIO
    COMMAND $<TARGET_FILE:test_AIO>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/iocore/aio
  )
endif()

clang_tidy_check(aio)
