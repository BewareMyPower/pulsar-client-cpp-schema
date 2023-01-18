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

set -e

cd `dirname $0`

docker run --rm -itd -p 6650:6650 -p 8080:8080 \
    --mount source=pulsardata,target=/pulsar/data \
    --mount source=pulsarconf,target=/pulsar/conf \
    apachepulsar/pulsar:2.11.0 \
    bin/pulsar standalone -nss -nfw > .container-id.txt

echo "-- Wait for Pulsar service to be ready"
until curl http://localhost:8080/metrics > /dev/null 2>&1 ; do sleep 1; done
while true; do
  echo "# Query namespaces under tenant \"public\"..."
  OUTPUT=$(curl -L http://localhost:8080/admin/v2/namespaces/public 2>/dev/null)
  if grep "\"public\/default\"" <<< $OUTPUT; then
    break
  else
    echo "Sleep for 1 second because public/default namespace does not exist"
    sleep 1
  fi
done

docker container ls | grep pulsar
