#!/usr/bin/env bash
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

set -ex
ROOT=$(cd $(dirname $0)/.. && pwd)

git submodule update --init

$ROOT/bin/setup-dependencies.sh

cmake -S pulsar-client-cpp -B build \
    -DCMAKE_PREFIX_PATH=$ROOT/dependencies \
    -DPROTOC_PATH=$ROOT/dependencies/bin/protoc \
    -DLINK_STATIC=ON \
    -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Debug -DBUILD_STATIC_LIB=OFF \
    -DCMAKE_INSTALL_PREFIX=$ROOT/installed
cmake --build build -j8 --target install
