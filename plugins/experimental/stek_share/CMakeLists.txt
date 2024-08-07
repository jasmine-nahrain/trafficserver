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

if(nuraft_FOUND)
  add_atsplugin(stek_share common.cc log_store.cc stek_share.cc stek_utils.cc)

  target_link_libraries(stek_share PRIVATE OpenSSL::SSL nuraft::nuraft yaml-cpp::yaml-cpp)
else()
  message(STATUS "skipping stek_share plugin (missing nuraft)")

  verify_global_plugin(stek_share)
endif()

# This was added by add_atsplugin, but one of the modules above must be overriding it.
if(APPLE)
  set(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup")
endif()
