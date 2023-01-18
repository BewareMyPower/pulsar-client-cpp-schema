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
#include <pulsar/schema/IntSchema.h>

#include "TestUtil.h"

using namespace pulsar;

int main() {
    Client client("pulsar://127.0.0.1:6650");
    schema::IntSchema schema;
    std::string topic = "my-topic";

    Producer producer;
    auto result = client.createProducer(topic, ProducerConfiguration{}.setSchema(schema), producer);
    if (result != ResultOk) {
        std::cerr << "Failed to create producer: " << result;
        return 1;
    }

    Consumer consumer;
    result = client.subscribe(topic, "sub", ConsumerConfiguration{}.setSchema(schema), consumer);
    if (result != ResultOk) {
        std::cerr << "Failed to subscribe: " << result;
        return 2;
    }

    auto test = [&](int32_t value) {
        MessageId msgId;
        producer.send(schema.newMessage(value).build(), msgId);
        std::cout << "Sent " << value << " to " << msgId << std::endl;
        TypedMessage<int32_t> msg;
        ASSERT_EQ(consumer.receive(msg, 100, schema.decoder()), ResultOk);
        std::cout << "Received " << msg.getValue() << " from " << msg.getMessageId() << std::endl;
        ASSERT_EQ(msg.getValue(), value);
        ASSERT_EQ(msg.getMessageId(), msgId);
        ASSERT_EQ(consumer.acknowledge(msg), ResultOk);
    };

    test(0x7F);
    test(0xFF);
    test(0x7FFF);
    test(0xFFFF);
    test(0x7FFFFF);
    test(0xFFFFFF);
    test(0x7FFFFFFF);
    test(0xFFFFFFFF);

    client.close();
    return 0;
}
