#include <chrono>
#include <gtest/gtest.h>
#include "channel.h"
#include "station.h"

using namespace std::chrono_literals;

namespace {

// Powers a station on, tunes it, and puts it in the given mode.
// Returns false if any step is rejected.
bool prepare(Station& station, double frequencyMhz, Mode mode) {
    Radio& radio = station.radio();
    if (!radio.setMode(Mode::Standby) || !radio.setFrequency(frequencyMhz)) {
        return false;
    }
    return mode == Mode::Standby || radio.setMode(mode);
}

}  // namespace

TEST(StationTest, ListenerOnSameFrequencyHearsMessage) {
    Channel channel;
    Station alpha("Alpha", channel);
    Station bravo("Bravo", channel);
    ASSERT_TRUE(prepare(alpha, 100.0, Mode::Transmit));
    ASSERT_TRUE(prepare(bravo, 100.0, Mode::Receive));

    ASSERT_TRUE(alpha.transmit("radio check"));
    ASSERT_TRUE(bravo.waitUntilProcessed(1, 1s));

    std::vector<Message> heard = bravo.heardMessages();
    ASSERT_EQ(heard.size(), 1u);
    EXPECT_EQ(heard[0].text, "radio check");
    EXPECT_EQ(heard[0].sender, "Alpha");
    EXPECT_EQ(heard[0].frequencyMhz, 100.0);
}

TEST(StationTest, ListenerOnDifferentFrequencyHearsNothing) {
    Channel channel;
    Station alpha("Alpha", channel);
    Station charlie("Charlie", channel);
    ASSERT_TRUE(prepare(alpha, 100.0, Mode::Transmit));
    ASSERT_TRUE(prepare(charlie, 250.0, Mode::Receive));

    ASSERT_TRUE(alpha.transmit("radio check"));
    ASSERT_TRUE(charlie.waitUntilProcessed(1, 1s));  // arrived, then dropped

    EXPECT_TRUE(charlie.heardMessages().empty());
}

TEST(StationTest, ListenerInStandbyHearsNothing) {
    Channel channel;
    Station alpha("Alpha", channel);
    Station bravo("Bravo", channel);
    ASSERT_TRUE(prepare(alpha, 100.0, Mode::Transmit));
    ASSERT_TRUE(prepare(bravo, 100.0, Mode::Standby));

    ASSERT_TRUE(alpha.transmit("radio check"));
    ASSERT_TRUE(bravo.waitUntilProcessed(1, 1s));

    EXPECT_TRUE(bravo.heardMessages().empty());
}

TEST(StationTest, TransmitFailsWhenNotInTransmitMode) {
    Channel channel;
    Station alpha("Alpha", channel);
    Station bravo("Bravo", channel);
    ASSERT_TRUE(prepare(alpha, 100.0, Mode::Receive));
    ASSERT_TRUE(prepare(bravo, 100.0, Mode::Receive));

    EXPECT_FALSE(alpha.transmit("should not send"));

    // Nothing was sent, so Bravo's receive thread never gets a message.
    EXPECT_FALSE(bravo.waitUntilProcessed(1, 100ms));
}

TEST(StationTest, SenderDoesNotReceiveItsOwnMessage) {
    Channel channel;
    Station alpha("Alpha", channel);
    Station bravo("Bravo", channel);
    ASSERT_TRUE(prepare(alpha, 100.0, Mode::Transmit));
    ASSERT_TRUE(prepare(bravo, 100.0, Mode::Receive));

    ASSERT_TRUE(alpha.transmit("radio check"));
    ASSERT_TRUE(bravo.waitUntilProcessed(1, 1s));

    EXPECT_FALSE(alpha.waitUntilProcessed(1, 100ms));
}

TEST(StationTest, EveryListenerGetsItsOwnCopy) {
    Channel channel;
    Station alpha("Alpha", channel);
    Station bravo("Bravo", channel);
    Station delta("Delta", channel);
    ASSERT_TRUE(prepare(alpha, 100.0, Mode::Transmit));
    ASSERT_TRUE(prepare(bravo, 100.0, Mode::Receive));
    ASSERT_TRUE(prepare(delta, 100.0, Mode::Receive));

    ASSERT_TRUE(alpha.transmit("all stations"));
    ASSERT_TRUE(bravo.waitUntilProcessed(1, 1s));
    ASSERT_TRUE(delta.waitUntilProcessed(1, 1s));

    EXPECT_EQ(bravo.heardMessages().size(), 1u);
    EXPECT_EQ(delta.heardMessages().size(), 1u);
}
