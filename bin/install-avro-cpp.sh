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

mkdir -p installed && cd installed
PREFIX=$PWD

# Use the same Boost version with pulsar-client-cpp
cp ../pulsar-client-cpp/build-support/dep-version.py .
cp ../pulsar-client-cpp/dependencies.yaml .

# Install Boost libraries, which are depended by avro-cpp
if [[ ! -f .avro-boost-done ]]; then
  BOOST_VERSION=$(./dep-version.py boost)
  BOOST_VERSION_UNDESRSCORE=$(echo ${BOOST_VERSION} | sed 's/\./_/g')
  DIR=boost_${BOOST_VERSION_UNDESRSCORE}
  curl -O -L https://boostorg.jfrog.io/artifactory/main/release/$BOOST_VERSION/source/$DIR.tar.gz
  tar zxf $DIR.tar.gz
  cd $DIR
  ./bootstrap.sh --prefix=$PREFIX --with-libraries=filesystem,iostreams,program_options,regex,system
  ./b2 install -j8
  cd -
  rm -rf $DIR*
  touch .avro-boost-done
fi

# Install avro-cpp
# Don't use 1.11.1, see https://issues.apache.org/jira/browse/AVRO-3601
AVRO_VERSION=1.11.0
if [[ ! -f .avro-done ]]; then
  curl -O -L http://archive.apache.org/dist/avro/avro-$AVRO_VERSION/cpp/avro-cpp-$AVRO_VERSION.tar.gz
  tar zxf avro-cpp-$AVRO_VERSION.tar.gz
  cd avro-cpp-$AVRO_VERSION/
  cmake -B build-avro -DCMAKE_PREFIX_PATH=$PREFIX -DCMAKE_INSTALL_PREFIX=$PREFIX
  cmake --build build-avro -j8 --target install
  cd -
  rm -rf avro-cpp*
  touch .avro-done
fi
