#ifndef RADIO_H
#define RADIO_H

#include <mutex>

enum class Mode
{
    Off,
    Standby,
    Receive, 
    Transmit
};

// A snapshot of all radio settings, read at one instant.
struct RadioStatus
{
    Mode mode;
    double frequencyMhz;
    int power;
};

class Radio
{
public:
    bool setMode(Mode newMode);
    Mode getMode() const;
    bool setFrequency(double newFrequency);
    double getFrequency() const;
    bool setPower(int newPower);
    int getPower() const;

    // Reads every setting under one lock, so the values are consistent with
    // each other even if another thread is changing the radio.
    RadioStatus status() const;
private:
    Mode mode_ = Mode::Off;
    double frequency_ = 30.0;
    int power_ = 1;

    // Guards all members above, so one Radio can be shared between threads.
    // mutable: the const getters also need to lock it.
    mutable std::mutex mutex_;
};
#endif