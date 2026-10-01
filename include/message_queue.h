#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>

struct Message
{
    double frequencyMhz = 0.0;  // frequency the message was sent on
    std::string text;
    std::string sender;         // name of the transmitting station
};

// A first-in, first-out queue that several threads can use at once.
// Producers call push(); consumers call pop(), which sleeps until a message
// arrives. close() shuts the queue down so waiting consumers can exit.
class MessageQueue
{
public:
    // Adds a message. Returns false if the queue has been closed.
    bool push(Message message);

    // Waits until a message is available and returns it. Returns
    // std::nullopt once the queue is closed and empty.
    std::optional<Message> pop();

    // Stops accepting messages and wakes up every waiting consumer.
    // Messages already in the queue can still be popped.
    void close();

private:
    std::queue<Message> messages_;
    bool closed_ = false;
    std::mutex mutex_;
    std::condition_variable messageAvailable_;
};

#endif
