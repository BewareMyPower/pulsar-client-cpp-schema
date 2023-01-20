/*
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

import static org.testng.Assert.assertEquals;
import static org.testng.Assert.assertNotNull;
import java.util.concurrent.TimeUnit;
import lombok.AllArgsConstructor;
import lombok.Cleanup;
import lombok.NoArgsConstructor;
import org.apache.pulsar.client.api.Consumer;
import org.apache.pulsar.client.api.Message;
import org.apache.pulsar.client.api.PulsarClient;
import org.apache.pulsar.client.api.Schema;
import org.apache.pulsar.client.api.SubscriptionInitialPosition;
import org.testng.annotations.Test;

public class AvroSchemaTest {

    // Consume the messages from AvroSchemaExample.cc
    @Test
    public void testConsume() throws Exception {
        @Cleanup final PulsarClient client = PulsarClient.builder().serviceUrl("pulsar://localhost:6650").build();
        @Cleanup final Consumer<User> consumer = client.newConsumer(Schema.AVRO(User.class))
                .topic("my-topic-avro")
                .subscriptionName("sub")
                .subscriptionInitialPosition(SubscriptionInitialPosition.Earliest)
                .subscribe();
        final Message<User> message = consumer.receive(3, TimeUnit.SECONDS);
        assertNotNull(message);
        final User user = message.getValue();
        assertEquals(user.age, 18);
        assertEquals(user.name, "xyz");
        consumer.acknowledge(message);
    }

    @NoArgsConstructor
    @AllArgsConstructor
    private static class User {
        String name;
        int age;
    }
}
