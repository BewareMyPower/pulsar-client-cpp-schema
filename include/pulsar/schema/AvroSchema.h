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

#include "avro/ValidSchema.hh"  // for avro::ValidSchema

namespace pulsar {

namespace schema {

/**
 * To use this schema, you have to specialize the template `pulsar::schema::codec_traits<T>` if `T` is not
 * generated from the `avrogencpp` binary.
 *
 * For example, given the following C struct:
 *
 * ```c++
 * struct User {
 *   std::string name;
 *   int age;
 *   char opaque[100]; // Assuming you don't want to encode this field
 * };
 * ```
 *
 * You have to define the encode and decode order in the template specialization.
 *
 * ```c++
 * namespace pulsar {
 * namespace schema {
 *
 * template <>
 * struct codec_traits<User> {
 *   static void encode(Encoder& e, const User& user) {
 *     e.encode(user.age);
 *     e.encode(user.name);
 *   };
 *
 *   static void decode(Decoder& d, User& user) {
 *     d.decode(user.age);
 *     d.decode(user.name);
 *   };
 * };
 *
 * }  // namespace schema
 * }  // namespace pulsar
 * ```
 *
 * The corresponding schema definition of the specialization above is:
 *
 * ```json
 * "fields": [
 *     {"name": "age", "type": "int"},
 *     {"name": "name", "type": ["null", "string"]}
 * ]
 * ```
 */
template <typename T>
class AvroSchema {
   public:
    AvroSchema(const std::string& schema, bool nullableString = true);

    operator SchemaInfo() { return schemaInfo_; }

    TypedMessageBuilder<T> newMessage(const T& value) const {
        return TypedMessageBuilder<T>{[this](const T& value) { return encode(value); }}.setValue(value);
    }

    T operator()(const char* data, std::size_t size) const;

   private:
    const SchemaInfo schemaInfo_;
    const avro::ValidSchema validSchema_;
    const bool nullableString_;

    std::string encode(const T& value) const;
};

}  // namespace schema
}  // namespace pulsar

#include "Specific.h"
#include "avro/Compiler.hh"  // for avro::compileJsonSchemaFromString
#include "avro/Decoder.hh"   // for avro::Decoder
#include "avro/Encoder.hh"   // for avro::Encoder
#include "avro/Stream.hh"    // for avro::OutputStream and avro::InputStream

template <typename T>
inline pulsar::schema::AvroSchema<T>::AvroSchema(const std::string& schema, bool nullableString)
    : schemaInfo_(SchemaType::AVRO, "", schema),
      validSchema_(avro::compileJsonSchemaFromString(schema)),
      nullableString_(nullableString) {}

template <typename T>
inline std::string pulsar::schema::AvroSchema<T>::encode(const T& value) const {
    std::unique_ptr<avro::OutputStream> out = avro::memoryOutputStream();
    std::shared_ptr<avro::Encoder> encoder = avro::binaryEncoder();
    encoder->init(*out);

    pulsar::schema::Encoder proxy{*encoder, nullableString_};
    proxy.encode(value);

    encoder->flush();

    std::unique_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
    size_t pos = 0;
    size_t readable = out->byteCount();
    std::string data(readable, '\0');

    const uint8_t* ptr;
    size_t len;
    while (readable > 0 && in->next(&ptr, &len)) {
        if (len > readable) {
            readable = len;
        }
        std::copy(ptr, ptr + len, &data[pos]);
        pos += len;
        readable -= len;
    }

    return data;
}

template <typename T>
inline T pulsar::schema::AvroSchema<T>::operator()(const char* data, std::size_t size) const {
    std::unique_ptr<avro::InputStream> in =
        avro::memoryInputStream(reinterpret_cast<const uint8_t*>(data), size);
    std::shared_ptr<avro::Decoder> decoder = avro::validatingDecoder(validSchema_, avro::binaryDecoder());
    decoder->init(*in);

    pulsar::schema::Decoder proxy{*decoder, nullableString_};
    T value;
    proxy.decode(value);
    return value;
}
