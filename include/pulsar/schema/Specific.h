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

#include "avro/Decoder.hh"
#include "avro/Encoder.hh"
#include "avro/Specific.hh"

namespace pulsar {
namespace schema {

class Encoder;

template <typename T>
struct codec_traits;

class Encoder {
    template <typename T>
    friend struct codec_traits;

   public:
    Encoder(avro::Encoder& encoder, bool nullableString)
        : encoder_(encoder), nullableString_(nullableString) {}

    void encodeString(const std::string& s) {
        // Assume the schema type is: "type": ["null", "string"]
        // The union schema type is generated default by Java's ReflectDatumWriter
        if (nullableString_) {
            encoder_.encodeUnionIndex(1);
        }
        encoder_.encodeString(s);
    }

    template <typename T>
    void encode(const T& value);

   private:
    avro::Encoder& encoder_;
    const bool nullableString_;
};

class Decoder {
    template <typename T>
    friend struct codec_traits;

   public:
    Decoder(avro::Decoder& decoder, bool nullableString)
        : decoder_(decoder), nullableString_(nullableString) {}

    void decodeString(std::string& s) {
        // Assume the schema type is: "type": ["null", "string"]
        // The union schema type is generated default by Java's ReflectDatumWriter
        if (nullableString_) {
            decoder_.decodeUnionIndex();
        }
        decoder_.decodeString(s);
    }

    template <typename T>
    void decode(T& value);

   private:
    avro::Decoder& decoder_;
    const bool nullableString_;
};

template <typename T>
struct codec_traits {
    static void encode(Encoder& encoder, const T& value) { avro::encode(encoder.encoder_, value); }

    static void decode(Decoder& decoder, T& value) { avro::decode(decoder.decoder_, value); }
};

template <>
struct codec_traits<std::string> {
    static void encode(Encoder& encoder, const std::string& s) { encoder.encodeString(s); }

    static void decode(Decoder& decoder, std::string& s) { decoder.decodeString(s); }
};

template <typename T>
inline void Encoder::encode(const T& value) {
    codec_traits<T>::encode(*this, value);
}

template <typename T>
inline void Decoder::decode(T& value) {
    codec_traits<T>::decode(*this, value);
}

}  // namespace schema
}  // namespace pulsar
