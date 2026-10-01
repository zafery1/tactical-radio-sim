#include "station.h"

#include <optional>
#include <utility>

Station::Station(std::string name, Channel& channel)
    : name_(std::move(name)), channel_(channel) {
    channel_.join(inbox_);
    receiver_ = std::thread(&Station::receiveLoop, this);
}

Station::~Station() {
    // Order matters: stop new deliveries, wake the receive thread so it can
    // exit, then wait for it before our members are destroyed.
    channel_.leave(inbox_);
    inbox_.close();
    receiver_.join();
}

const std::string& Station::name() const {
    return name_;
}

Radio& Station::radio() {
    return radio_;
}

bool Station::transmit(const std::string& text) {
    // One snapshot, so the mode we check and the frequency we send on match.
    RadioStatus status = radio_.status();
    if (status.mode != Mode::Transmit) {
        return false;
    }
    channel_.broadcast({status.frequencyMhz, text, name_}, inbox_);
    return true;
}

std::vector<Message> Station::heardMessages() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return heard_;
}

bool Station::waitUntilProcessed(std::size_t count, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return processed_.wait_for(lock, timeout, [&] {
        return processedCount_ >= count;
    });
}

void Station::receiveLoop() {
    // pop() sleeps until a message arrives; it returns nullopt once the
    // inbox is closed, which ends the loop and the thread.
    while (std::optional<Message> message = inbox_.pop()) {
        RadioStatus status = radio_.status();
        bool canHear = status.mode == Mode::Receive &&
                       status.frequencyMhz == message->frequencyMhz;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (canHear) {
                heard_.push_back(std::move(*message));
            }
            ++processedCount_;
        }
        processed_.notify_all();
    }
}
