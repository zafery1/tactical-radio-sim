#ifndef STATION_H
#define STATION_H

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "channel.h"
#include "message_queue.h"
#include "radio.h"

// A radio operator's station: a Radio connected to a Channel, with its own
// background thread that receives incoming messages.
//
// A message is heard only if, when it arrives, the radio is in Receive mode
// and tuned to the frequency it was sent on. Otherwise it's dropped.
class Station
{
public:
    Station(std::string name, Channel& channel);
    ~Station();

    // A station owns a running thread, so it can't be copied.
    Station(const Station&) = delete;
    Station& operator=(const Station&) = delete;

    const std::string& name() const;
    Radio& radio();

    // Sends text on the current frequency. Returns false (and sends nothing)
    // if the radio isn't in Transmit mode.
    bool transmit(const std::string& text);

    // Every message this station has heard so far, oldest first.
    std::vector<Message> heardMessages() const;

    // Waits until the receive thread has handled at least `count` incoming
    // messages (heard or dropped). Returns false if the timeout runs out.
    bool waitUntilProcessed(std::size_t count, std::chrono::milliseconds timeout);

private:
    void receiveLoop();

    std::string name_;
    Channel& channel_;
    Radio radio_;
    MessageQueue inbox_;

    mutable std::mutex mutex_;  // guards heard_ and processedCount_
    std::condition_variable processed_;
    std::vector<Message> heard_;
    std::size_t processedCount_ = 0;

    // Declared last so it's started after every member above is ready.
    std::thread receiver_;
};

#endif
