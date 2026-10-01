#include <cmath>
#include <gtest/gtest.h>
#include "radio.h"

// ---------------------------------------------------------------------------
// Mode transitions
// ---------------------------------------------------------------------------

TEST(RadioModeTest, OffToStandbyIsAllowed) {
    Radio radio;

    bool result = radio.setMode(Mode::Standby);

    EXPECT_TRUE(result);
    EXPECT_EQ(radio.getMode(), Mode::Standby);
}

TEST(RadioModeTest, OffToTransmitIsNotAllowed) {
    Radio radio;

    bool result = radio.setMode(Mode::Transmit);

    EXPECT_FALSE(result);
    EXPECT_EQ(radio.getMode(), Mode::Off);
}

// ---------------------------------------------------------------------------
// Frequency
// ---------------------------------------------------------------------------

TEST(RadioFrequencyTest, ValidFrequencyInStandbyIsAccepted) {
    Radio radio;
    ASSERT_TRUE(radio.setMode(Mode::Standby));

    bool result = radio.setFrequency(100.0);

    EXPECT_TRUE(result);
    EXPECT_EQ(radio.getFrequency(), 100.0);
}

TEST(RadioFrequencyTest, BoundaryFrequenciesAreAccepted) {
    Radio radio;
    ASSERT_TRUE(radio.setMode(Mode::Standby));

    EXPECT_TRUE(radio.setFrequency(30.0));
    EXPECT_EQ(radio.getFrequency(), 30.0);

    EXPECT_TRUE(radio.setFrequency(512.0));
    EXPECT_EQ(radio.getFrequency(), 512.0);
}

TEST(RadioFrequencyTest, OutOfRangeFrequencyIsRejected) {
    Radio radio;
    ASSERT_TRUE(radio.setMode(Mode::Standby));

    bool result = radio.setFrequency(600.0);

    EXPECT_FALSE(result);
    EXPECT_EQ(radio.getFrequency(), 30.0);
}

TEST(RadioFrequencyTest, JustOutsideBoundariesIsRejected) {
    Radio radio;
    ASSERT_TRUE(radio.setMode(Mode::Standby));

    EXPECT_FALSE(radio.setFrequency(29.9));
    EXPECT_FALSE(radio.setFrequency(512.1));
    EXPECT_EQ(radio.getFrequency(), 30.0);
}

TEST(RadioFrequencyTest, NaNIsRejected) {
    Radio radio;
    ASSERT_TRUE(radio.setMode(Mode::Standby));

    EXPECT_FALSE(radio.setFrequency(std::nan("")));
    EXPECT_EQ(radio.getFrequency(), 30.0);
}

TEST(RadioFrequencyTest, FrequencyCannotChangeWhileTransmitting) {
    Radio radio;
    ASSERT_TRUE(radio.setMode(Mode::Standby));
    ASSERT_TRUE(radio.setMode(Mode::Transmit));

    bool result = radio.setFrequency(100.0);

    EXPECT_FALSE(result);
    EXPECT_EQ(radio.getFrequency(), 30.0);
}

// ---------------------------------------------------------------------------
// Power
// ---------------------------------------------------------------------------

TEST(RadioPowerTest, ValidPowerIsAccepted) {
    Radio radio;

    EXPECT_TRUE(radio.setPower(1));
    EXPECT_EQ(radio.getPower(), 1);

    EXPECT_TRUE(radio.setPower(5));
    EXPECT_EQ(radio.getPower(), 5);
}

TEST(RadioPowerTest, OutOfRangePowerIsRejected) {
    Radio radio;

    EXPECT_FALSE(radio.setPower(0));
    EXPECT_FALSE(radio.setPower(6));
    EXPECT_EQ(radio.getPower(), 1);
}
