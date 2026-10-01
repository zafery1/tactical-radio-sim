#include "radio.h"

bool Radio::setMode(Mode newMode) {
    std::lock_guard<std::mutex> lock(mutex_);
    bool allowed = false;

    // Which modes can we move to from the current one?
    switch (mode_) {
        case Mode::Off:
            allowed = (newMode == Mode::Standby);
            break;
        case Mode::Standby:
            allowed = (newMode == Mode::Off || newMode == Mode::Receive || newMode == Mode::Transmit);
            break;
        case Mode::Receive:
            allowed = (newMode == Mode::Standby || newMode == Mode::Transmit);
            break;
        case Mode::Transmit:
            allowed = (newMode == Mode::Standby || newMode == Mode::Receive);
            break;
    }

    if (!allowed) {
        return false;
    }
    mode_ = newMode;
    return true;
}

Mode Radio::getMode() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return mode_;
}
bool Radio::setPower(int newPower) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (newPower < 1 || newPower > 5){
        return false;
    }
    power_ = newPower;
    return true;
}
int Radio::getPower() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return power_;
}
bool Radio::setFrequency(double newFrequency) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!(newFrequency >= 30 && newFrequency <= 512)) {
        return false;
    }
    if (mode_ != Mode::Standby) {
        return false;
    }
    frequency_ = newFrequency;
    return true;
    
}
double Radio::getFrequency() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frequency_;
}