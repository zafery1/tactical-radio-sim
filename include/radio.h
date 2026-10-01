#ifndef RADIO_H
#define RADIO_H

enum class Mode
{
    Off,
    Standby,
    Receive, 
    Transmit
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
private:
    Mode mode_ = Mode::Off;
    double frequency_ = 30.0;
    int power_ = 1;
};
#endif