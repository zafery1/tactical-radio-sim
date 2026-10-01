#include <atomic>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include "radio.h"

// These tests use one Radio from several threads at once. On their own they
// can pass by luck even if Radio isn't thread-safe, so also run them under
// ThreadSanitizer (see README), which reports any data race it sees.

TEST(RadioThreadTest, ConcurrentModeAndFrequencyChangesKeepRadioValid) {
    Radio radio;
    ASSERT_TRUE(radio.setMode(Mode::Standby));

    constexpr int kIterations = 10000;
    std::atomic<bool> sawInvalidState{false};

    // Flips between Standby and Receive, always ending in Standby.
    std::thread modeSwitcher([&] {
        for (int i = 0; i < kIterations; ++i) {
            radio.setMode(Mode::Receive);
            radio.setMode(Mode::Standby);
        }
    });

    // Keeps trying to retune; only succeeds while the radio is in Standby.
    std::thread tuner([&] {
        for (int i = 0; i < kIterations; ++i) {
            radio.setFrequency(30.0 + (i % 400));
        }
    });

    // Keeps reading, and checks that it never sees an invalid value.
    std::thread reader([&] {
        for (int i = 0; i < kIterations; ++i) {
            double frequency = radio.getFrequency();
            Mode mode = radio.getMode();
            if (frequency < 30.0 || frequency > 512.0 ||
                (mode != Mode::Standby && mode != Mode::Receive)) {
                sawInvalidState = true;
            }
        }
    });

    modeSwitcher.join();
    tuner.join();
    reader.join();

    EXPECT_FALSE(sawInvalidState);
    EXPECT_EQ(radio.getMode(), Mode::Standby);
}

TEST(RadioThreadTest, ConcurrentPowerChangesKeepPowerInRange) {
    Radio radio;
    constexpr int kThreads = 8;
    constexpr int kIterations = 10000;

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&radio, t] {
            for (int i = 0; i < kIterations; ++i) {
                radio.setPower(1 + (t + i) % 5);
            }
        });
    }
    for (std::thread& thread : threads) {
        thread.join();
    }

    EXPECT_GE(radio.getPower(), 1);
    EXPECT_LE(radio.getPower(), 5);
}
