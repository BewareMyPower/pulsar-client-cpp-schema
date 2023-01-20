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
 * To use this schema, you have to implement `avro::codec_traits<T>` if `T` is not generated from the
 * `avrogencpp` binary.
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
 * You should implement the following template specialization like:
 *
 * ```c++
 * namespace avro {
 *
 * template <>
 * struct codec_traits<User> {
 *   static void encode(avro::Encoder& e, const User& user) {
 *     avro::encode(e, user.name);
 *     e.encodeUnionIndex(1);
 *     avro::encode(e, user.age);
 *   };
 *
 *   static void decode(avro::Decoder& d, User& user) {
 *     avro::decode(d, user.name);
 *     d.decodeUnionIndex();
 *     avro::decode(d, user.age);
 *   };
 * };
 *
 * }  // namespace avro
 * ```
 */
template <typename T>
class AvroSchema {
   public:
    AvroSchema(const std::string& schema);

    operator SchemaInfo() { return schemaInfo_; }

    TypedMessageBuilder<T> newMessage(const T& value) const {
        return TypedMessageBuilder<T>{encode}.setValue(value);
    }

    T operator()(const char* data, std::size_t size) const;

   private:
    const SchemaInfo schemaInfo_;
    const avro::ValidSchema validSchema_;

    static std::string encode(const T& value);
};

}  // namespace schema
}  // namespace pulsar

#include "avro/Compiler.hh"  // for avro::compileJsonSchemaFromString
#include "avro/Decoder.hh"   // for avro::Decoder
#include "avro/Encoder.hh"   // for avro::Encoder
#include "avro/Specific.hh"  // for avro::encode and avro::decode
#include "avro/Stream.hh"    // for avro::OutputStream and avro::InputStream

template <typename T>
inline pulsar::schema::AvroSchema<T>::AvroSchema(const std::string& schema)
    : schemaInfo_(SchemaType::AVRO, "", schema), validSchema_(avro::compileJsonSchemaFromString(schema)) {}

template <typename T>
inline std::string pulsar::schema::AvroSchema<T>::encode(const T& value) {
    std::unique_ptr<avro::OutputStream> out = avro::memoryOutputStream();
    std::shared_ptr<avro::Encoder> encoder = avro::binaryEncoder();
    encoder->init(*out);

    avro::encode(*encoder, value);

    encoder->flush();

    auto bytesPtr = avro::snapshot(*out);
    // TODO: avoid the copy by implementing our own OutputStream
    return std::string(bytesPtr->cbegin(), bytesPtr->cend());
}

template <typename T>
inline T pulsar::schema::AvroSchema<T>::operator()(const char* data, std::size_t size) const {
    std::unique_ptr<avro::InputStream> in =
        avro::memoryInputStream(reinterpret_cast<const uint8_t*>(data), size);
    std::shared_ptr<avro::Decoder> decoder = avro::validatingDecoder(validSchema_, avro::binaryDecoder());
    decoder->init(*in);

    T value;
    avro::decode(*decoder, value);
    return value;
}
