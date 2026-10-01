#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include "message_queue.h"

// ---------------------------------------------------------------------------
// Single-threaded behavior
// ---------------------------------------------------------------------------

TEST(MessageQueueTest, PopReturnsPushedMessage) {
    MessageQueue queue;

    ASSERT_TRUE(queue.push({100.0, "hello"}));
    std::optional<Message> message = queue.pop();

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->frequencyMhz, 100.0);
    EXPECT_EQ(message->text, "hello");
}

TEST(MessageQueueTest, MessagesComeOutInOrder) {
    MessageQueue queue;
    ASSERT_TRUE(queue.push({100.0, "first"}));
    ASSERT_TRUE(queue.push({100.0, "second"}));
    ASSERT_TRUE(queue.push({100.0, "third"}));

    EXPECT_EQ(queue.pop()->text, "first");
    EXPECT_EQ(queue.pop()->text, "second");
    EXPECT_EQ(queue.pop()->text, "third");
}

TEST(MessageQueueTest, PushAfterCloseIsRejected) {
    MessageQueue queue;
    queue.close();

    EXPECT_FALSE(queue.push({100.0, "too late"}));
}

TEST(MessageQueueTest, RemainingMessagesCanBePoppedAfterClose) {
    MessageQueue queue;
    ASSERT_TRUE(queue.push({100.0, "last words"}));
    queue.close();

    std::optional<Message> message = queue.pop();
    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->text, "last words");

    EXPECT_FALSE(queue.pop().has_value());
}

// ---------------------------------------------------------------------------
// Multi-threaded behavior
// ---------------------------------------------------------------------------

TEST(MessageQueueTest, PopWaitsUntilMessageArrives) {
    MessageQueue queue;
    std::optional<Message> received;

    std::thread consumer([&] {
        received = queue.pop();  // queue is empty, so this sleeps
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_TRUE(queue.push({100.0, "wake up"}));
    consumer.join();

    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->text, "wake up");
}

TEST(MessageQueueTest, CloseWakesUpWaitingConsumer) {
    MessageQueue queue;
    std::optional<Message> received = Message{};  // non-empty, so the test proves pop() cleared it

    std::thread consumer([&] {
        received = queue.pop();  // sleeps until close()
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.close();
    consumer.join();  // hangs here (and the test times out) if close() doesn't wake the consumer

    EXPECT_FALSE(received.has_value());
}

TEST(MessageQueueTest, ManyProducersOneConsumerDeliversEveryMessage) {
    MessageQueue queue;
    constexpr int kProducers = 4;
    constexpr int kMessagesEach = 1000;

    int receivedCount = 0;  // only the consumer thread touches this until join()
    std::thread consumer([&] {
        while (queue.pop().has_value()) {
            ++receivedCount;
        }
    });

    std::vector<std::thread> producers;
    for (int p = 0; p < kProducers; ++p) {
        producers.emplace_back([&queue, p] {
            for (int i = 0; i < kMessagesEach; ++i) {
                queue.push({100.0, "producer " + std::to_string(p)});
            }
        });
    }
    for (std::thread& producer : producers) {
        producer.join();
    }
    queue.close();  // consumer drains what's left, then pop() returns nullopt
    consumer.join();

    EXPECT_EQ(receivedCount, kProducers * kMessagesEach);
}
