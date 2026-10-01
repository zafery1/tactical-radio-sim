#include "message_queue.h"

#include <utility>

bool MessageQueue::push(Message message) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) {
            return false;
        }
        messages_.push(std::move(message));
    }
    // Notify after unlocking, so the woken consumer can take the lock at once.
    messageAvailable_.notify_one();
    return true;
}

std::optional<Message> MessageQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    // Sleep until there's a message or the queue is closed. wait() releases
    // the lock while sleeping and re-checks the condition on every wake-up.
    messageAvailable_.wait(lock, [this] {
        return !messages_.empty() || closed_;
    });

    if (messages_.empty()) {
        return std::nullopt;  // closed and nothing left
    }
    Message message = std::move(messages_.front());
    messages_.pop();
    return message;
}

void MessageQueue::close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
        messageAvailable_.notify_all();
    }
}
