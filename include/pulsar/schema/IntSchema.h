/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include <pulsar/Client.h>
#include <pulsar/TypedMessageBuilder.h>

#include <cstdint>
#include <stdexcept>

namespace pulsar {

namespace schema {

class IntSchema {
   public:
    operator SchemaInfo() { return info_; }

    TypedMessageBuilder<int32_t> newMessage(int32_t value) const {
        return TypedMessageBuilder<int32_t>{encode}.setValue(value);
    }

    std::function<int32_t(const char*, std::size_t)> decoder() const {
        return [](const char* data, size_t size) {
            if (size < 4) {
                throw std::invalid_argument("Wrong int32 size " + std::to_string(size));
            }
            int32_t value = 0;
            value += (static_cast<int32_t>(static_cast<unsigned char>(data[0])) << 24);
            value += (static_cast<int32_t>(static_cast<unsigned char>(data[1])) << 16);
            value += (static_cast<int32_t>(static_cast<unsigned char>(data[2])) << 8);
            value += static_cast<int32_t>(static_cast<unsigned char>(data[3]));
            return value;
        };
    }

   private:
    const SchemaInfo info_{SchemaType::INT32, "INT32", ""};

    static std::string encode(int32_t x) {
        std::string data(4, '\0');
        data[0] = static_cast<char>(x >> 24);
        data[1] = static_cast<char>(x >> 16);
        data[2] = static_cast<char>(x >> 8);
        data[3] = static_cast<char>(x);
        return data;
    }
};

}  // namespace schema

}  // namespace pulsar
