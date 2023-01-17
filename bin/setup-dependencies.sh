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

mkdir -p dependencies && cd dependencies
PREFIX=$PWD
cp ../pulsar-client-cpp/build-support/dep-version.py .
cp ../pulsar-client-cpp/dependencies.yaml .

if [[ ! -f .boost-done ]]; then
  BOOST_VERSION=$(./dep-version.py boost)
  BOOST_VERSION_UNDESRSCORE=$(echo ${BOOST_VERSION} | sed 's/\./_/g')
  DIR=boost_${BOOST_VERSION_UNDESRSCORE}
  curl -O -L https://boostorg.jfrog.io/artifactory/main/release/$BOOST_VERSION/source/$DIR.tar.gz
  tar zxf $DIR.tar.gz
  mkdir -p include
  cp -rf $DIR/boost include/
  rm -rf $DIR*
  touch .boost-done
fi

if [[ ! -f .protobuf-done ]]; then
  PROTOBUF_VERSION=$(./dep-version.py protobuf)
  curl -O -L https://github.com/google/protobuf/releases/download/v${PROTOBUF_VERSION}/protobuf-cpp-${PROTOBUF_VERSION}.tar.gz
  tar xfz protobuf-cpp-${PROTOBUF_VERSION}.tar.gz
  cd protobuf-${PROTOBUF_VERSION}
  CXXFLAGS=-fPIC ./configure --prefix=$PREFIX
  make -j8 && make install
  cd -
  rm -rf protobuf-${PROTOBUF_VERSION} protobuf-cpp-${PROTOBUF_VERSION}.tar.gz
  touch .protobuf-done
fi

if [[ ! -f .openssl-done ]]; then
  OPENSSL_VERSION=$(./dep-version.py openssl)
  OPENSSL_VERSION_UNDERSCORE=$(echo $OPENSSL_VERSION | sed 's/\./_/g')
  curl -O -L https://github.com/openssl/openssl/archive/OpenSSL_${OPENSSL_VERSION_UNDERSCORE}.tar.gz
  tar xfz OpenSSL_${OPENSSL_VERSION_UNDERSCORE}.tar.gz
  cd openssl-OpenSSL_${OPENSSL_VERSION_UNDERSCORE}
  ./config -fPIC --prefix=$PREFIX
  make -j8 && make install
  cd -
  rm -rf openssl-OpenSSL_${OPENSSL_VERSION_UNDERSCORE} OpenSSL_${OPENSSL_VERSION_UNDERSCORE}.tar.gz
  touch .openssl-done
fi
export OPENSSL_ROOT_DIR=$PREFIX

if [[ ! -f .curl-done ]]; then
  CURL_VERSION=$(./dep-version.py curl)
  CURL_VERSION_UNDERSCORE=$(echo $CURL_VERSION | sed 's/\./_/g')
  curl -O -L  https://github.com/curl/curl/releases/download/curl-${CURL_VERSION_UNDERSCORE}/curl-${CURL_VERSION}.tar.gz
  tar xfz curl-${CURL_VERSION}.tar.gz
  cd curl-${CURL_VERSION}
  CFLAGS=-fPIC ./configure \
      --without-zstd \
      --without-brotli \
      --without-nghttp2 \
      --without-libidn2 \
      --disable-ldap \
      --without-brotli \
      --without-secure-transport \
      --with-ssl=$PREFIX \
      --disable-ipv6 \
      --prefix=$PREFIX
  make -j8 && make install
  cd -
  rm -rf curl-${CURL_VERSION}*
  touch .curl-done
fi

if [[ ! -f .zlib-done ]]; then
  ZLIB_VERSION=$(./dep-version.py zlib)
  curl -O -L https://github.com/madler/zlib/archive/v${ZLIB_VERSION}.tar.gz
  tar xfz v${ZLIB_VERSION}.tar.gz
  cd zlib-${ZLIB_VERSION}
  CFLAGS=-fPIC ./configure --prefix=$PREFIX
  make -j8 && make install
  cd -
  rm -rf v${ZLIB_VERSION}.tar.gz zlib-${ZLIB_VERSION}
  touch .zlib-done
fi
