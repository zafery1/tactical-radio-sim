#include <chrono>
#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "channel.h"
#include "station.h"

using namespace std::chrono_literals;

namespace {

// Moves a station to Standby, tunes it, then switches to the given mode.
// Returns false if any step is rejected.
bool tune(Station& station, double frequencyMhz, Mode mode) {
    Radio& radio = station.radio();
    if (radio.getMode() != Mode::Standby && !radio.setMode(Mode::Standby)) {
        return false;
    }
    if (!radio.setFrequency(frequencyMhz)) {
        return false;
    }
    return mode == Mode::Standby || radio.setMode(mode);
}

// Sends a message from `sender`, waits for every other station to handle
// it, and prints who heard it.
void sendAndReport(Station& sender, const std::string& text,
                   const std::vector<Station*>& stations,
                   std::map<Station*, std::size_t>& received) {
    std::cout << "\n" << sender.name() << " (" << sender.radio().getFrequency()
              << " MHz): \"" << text << "\"\n";

    std::map<Station*, std::size_t> heardBefore;
    for (Station* station : stations) {
        heardBefore[station] = station->heardMessages().size();
    }

    if (!sender.transmit(text)) {
        std::cout << "  not sent: " << sender.name() << " is not in Transmit mode\n";
        return;
    }

    for (Station* station : stations) {
        if (station == &sender) {
            continue;
        }
        ++received[station];  // the channel delivers one copy to each station
        if (!station->waitUntilProcessed(received[station], 1s)) {
            std::cout << "  " << station->name() << ": timed out\n";
            continue;
        }
        bool heard = station->heardMessages().size() > heardBefore[station];
        std::cout << "  " << station->name() << (heard ? " heard it" : " heard nothing") << "\n";
    }
}

}  // namespace

int main() {
    std::cout << "Radio simulator starting\n";

    Channel channel;
    Station alpha("Alpha", channel);
    Station bravo("Bravo", channel);
    Station charlie("Charlie", channel);
    std::vector<Station*> stations = {&alpha, &bravo, &charlie};
    std::map<Station*, std::size_t> received;

    // Alpha and Bravo share 100 MHz; Charlie is on 250 MHz.
    if (!tune(alpha, 100.0, Mode::Transmit) ||
        !tune(bravo, 100.0, Mode::Receive) ||
        !tune(charlie, 250.0, Mode::Receive)) {
        std::cerr << "Setup failed\n";
        return 1;
    }
    sendAndReport(alpha, "Bravo, this is Alpha. Radio check, over.", stations, received);

    // Swap roles: Transmit <-> Receive is allowed directly.
    if (!alpha.radio().setMode(Mode::Receive) || !bravo.radio().setMode(Mode::Transmit)) {
        std::cerr << "Mode change failed\n";
        return 1;
    }
    sendAndReport(bravo, "Alpha, this is Bravo. Loud and clear, over.", stations, received);

    // Charlie retunes to 100 MHz and joins the net.
    if (!tune(charlie, 100.0, Mode::Receive) ||
        !alpha.radio().setMode(Mode::Transmit) || !bravo.radio().setMode(Mode::Receive)) {
        std::cerr << "Retune failed\n";
        return 1;
    }
    sendAndReport(alpha, "All stations, this is Alpha. Net check, out.", stations, received);

    std::cout << "\nRadio simulator stopping\n";
    return 0;
}
