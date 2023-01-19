#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

set(CMAKE_CXX_STANDARD 11)
find_path(PULSAR_INCLUDE_DIRS NAMES "pulsar/Client.h")
if (NOT PULSAR_INCLUDE_DIRS)
    message(FATAL_ERROR "Failed to find PULSAR_INCLUDE_DIRS")
endif ()
find_library(PULSAR_LIBRARIES NAMES "pulsar")
if (NOT PULSAR_LIBRARIES)
    message(FATAL_ERROR "Failed to find PULSAR_LIBRARIES")
endif ()
message(STATUS "PULSAR_INCLUDE_DIRS: ${PULSAR_INCLUDE_DIRS}")
message(STATUS "PULSAR_LIBRARIES: ${PULSAR_LIBRARIES}")

add_executable(IntSchemaExample IntSchemaExample.cc)
target_include_directories(IntSchemaExample PUBLIC ../include ${PULSAR_INCLUDE_DIRS})
target_link_libraries(IntSchemaExample ${PULSAR_LIBRARIES})

# Required by avro-cpp
find_package(Boost REQUIRED COMPONENTS filesystem iostreams program_options regex system)

find_path(AVRO_CPP_INCLUDE_DIRS NAMES "avro/Encoder.hh")
if (NOT AVRO_CPP_INCLUDE_DIRS)
    message(FATAL_ERROR "Failed to find AVRO_CPP_INCLUDE_DIRS")
endif ()
find_library(AVRO_CPP_LIBRARIES NAMES "avrocpp")
if (NOT AVRO_CPP_LIBRARIES)
    message(FATAL_ERROR "Failed to find AVRO_CPP_LIBRARIES")
endif ()
message(STATUS "AVRO_CPP_INCLUDE_DIRS: ${AVRO_CPP_INCLUDE_DIRS}")
message(STATUS "AVRO_CPP_LIBRARIES: ${AVRO_CPP_LIBRARIES}")

add_executable(AvroSchemaExample AvroSchemaExample.cc)
target_include_directories(AvroSchemaExample PUBLIC
    ../include
    ${Boost_INCLUDE_DIRS}
    ${AVRO_CPP_INCLUDE_DIRS}
    ${PULSAR_INCLUDE_DIRS})
target_link_libraries(AvroSchemaExample
    ${AVRO_CPP_LIBRARIES}
    ${PULSAR_LIBRARIES}
    Boost::filesystem
    Boost::iostreams
    Boost::program_options
    Boost::regex
    Boost::system)
