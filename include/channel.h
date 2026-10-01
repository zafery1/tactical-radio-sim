#ifndef CHANNEL_H
#define CHANNEL_H

#include <mutex>
#include <vector>

#include "message_queue.h"

// The shared "air" between stations. Each station registers its inbox, and
// broadcast() delivers a copy of a message to every inbox except the sender's.
// A Channel must outlive every station that joined it.
class Channel
{
public:
    void join(MessageQueue& inbox);
    void leave(MessageQueue& inbox);
    void broadcast(const Message& message, const MessageQueue& senderInbox);

private:
    std::mutex mutex_;
    std::vector<MessageQueue*> inboxes_;
};

#endif
