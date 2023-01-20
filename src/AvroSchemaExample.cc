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
#include <pulsar/schema/AvroSchema.h>

#include "TestUtil.h"
#include "avro/Decoder.hh"
#include "avro/Encoder.hh"
#include "avro/Specific.hh"

using namespace pulsar;

struct User {
    std::string name;
    int age;
};

namespace avro {
template <>
struct codec_traits<User> {
    static void encode(avro::Encoder& e, const User& user) {
        avro::encode(e, user.age);
        e.encodeUnionIndex(1);
        avro::encode(e, user.name);
    }

    static void decode(avro::Decoder& d, User& user) {
        avro::decode(d, user.age);
        d.decodeUnionIndex();
        avro::decode(d, user.name);
    }
};
}  // namespace avro

int main() {
    Client client("pulsar://localhost:6650");
    // The string field must be nullable to be compatible with Java client
    schema::AvroSchema<User> schema{R"({
    "type": "record",
    "namespace": "org.example",
    "name": "User",
    "fields": [
        {"name": "age", "type": "int"},
        {"name": "name", "type": ["null", "string"]}
    ]
})"};
    std::string topic = "my-topic-avro";

    Producer producer;
    ASSERT_EQ(client.createProducer(topic, ProducerConfiguration{}.setSchema(schema), producer), ResultOk);

    Consumer consumer;
    ASSERT_EQ(client.subscribe(topic, "sub-cpp", ConsumerConfiguration{}.setSchema(schema), consumer),
              ResultOk);

    MessageId msgId;
    ASSERT_EQ(producer.send(schema.newMessage(User{"xyz", 18}).build(), msgId), ResultOk);
    std::cout << "Sent to " << msgId << std::endl;

    TypedMessage<User> msg;
    ASSERT_EQ(consumer.receive(msg, 3000, schema), ResultOk);
    ASSERT_EQ(msg.getValue().name, "xyz");
    ASSERT_EQ(msg.getValue().age, 18);

    consumer.acknowledge(msg);
    client.close();
    return 0;
}
