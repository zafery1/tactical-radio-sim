#include "channel.h"

#include <algorithm>

void Channel::join(MessageQueue& inbox) {
    std::lock_guard<std::mutex> lock(mutex_);
    inboxes_.push_back(&inbox);
}

void Channel::leave(MessageQueue& inbox) {
    std::lock_guard<std::mutex> lock(mutex_);
    inboxes_.erase(std::remove(inboxes_.begin(), inboxes_.end(), &inbox), inboxes_.end());
}

void Channel::broadcast(const Message& message, const MessageQueue& senderInbox) {
    // Holding the lock for the whole loop means no inbox can leave (and be
    // destroyed) while we're delivering to it.
    std::lock_guard<std::mutex> lock(mutex_);
    for (MessageQueue* inbox : inboxes_) {
        if (inbox != &senderInbox) {
            inbox->push(message);  // a copy for each listener
        }
    }
}
