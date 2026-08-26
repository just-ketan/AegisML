#include <gtest/gtest.h>

#include "market_simulator.hpp"


TEST(MarketSimulatorTest, GeneratesExactEventCount) {

    SimulationConfig config{
        .event_count = 1000,
        .seed = 42,
        .symbols = {
            {'A', 'A', 'P', 'L'},
            {'M', 'S', 'F', 'T'},
            {'G', 'O', 'O', 'G'}
        }
    };

    MarketSimulator simulator(config);

    MarketEvent event;

    std::uint64_t count = 0;

    while (simulator.next_event(event)) {
        ++count;
    }

    EXPECT_EQ(count, 1000);
}


TEST(MarketSimulatorTest, EventIdsAreSequential) {

    SimulationConfig config{
        .event_count = 100,
        .seed = 42,
        .symbols = {
            {'A', 'A', 'P', 'L'}
        }
    };

    MarketSimulator simulator(config);

    MarketEvent event;

    std::uint64_t expected_id = 1;

    while (simulator.next_event(event)) {

        EXPECT_EQ(event.event_id, expected_id);

        ++expected_id;
    }
}


TEST(MarketSimulatorTest, SequenceNumbersAreSequential) {

    SimulationConfig config{
        .event_count = 100,
        .seed = 42,
        .symbols = {
            {'A', 'A', 'P', 'L'}
        }
    };

    MarketSimulator simulator(config);

    MarketEvent event;

    std::uint64_t expected_sequence = 1;

    while (simulator.next_event(event)) {

        EXPECT_EQ(
            event.sequence_number,
            expected_sequence
        );

        ++expected_sequence;
    }
}


TEST(MarketSimulatorTest, TimestampsIncrease) {

    SimulationConfig config{
        .event_count = 100,
        .seed = 42,
        .symbols = {
            {'A', 'A', 'P', 'L'}
        }
    };

    MarketSimulator simulator(config);

    MarketEvent event;

    Timestamp previous_timestamp{0};

    while (simulator.next_event(event)) {

        EXPECT_GT(
            event.timestamp,
            previous_timestamp
        );

        previous_timestamp = event.timestamp;
    }
}


TEST(MarketSimulatorTest, GeneratedEventsAreValid) {

    SimulationConfig config{
        .event_count = 1000,
        .seed = 42,
        .symbols = {
            {'A', 'A', 'P', 'L'},
            {'M', 'S', 'F', 'T'},
            {'G', 'O', 'O', 'G'}
        }
    };

    MarketSimulator simulator(config);

    MarketEvent event;

    while (simulator.next_event(event)) {

        EXPECT_TRUE(is_valid(event));
    }
}


TEST(MarketSimulatorTest, BatchProducesRequestedAmount) {

    SimulationConfig config{
        .event_count = 10,
        .seed = 42,
        .symbols = {
            {'A', 'A', 'P', 'L'}
        }
    };

    MarketSimulator simulator(config);

    std::vector<MarketEvent> batch;

    EXPECT_TRUE(simulator.next_batch(batch, 3));
    EXPECT_EQ(batch.size(), 3);

    EXPECT_TRUE(simulator.next_batch(batch, 3));
    EXPECT_EQ(batch.size(), 3);

    EXPECT_TRUE(simulator.next_batch(batch, 3));
    EXPECT_EQ(batch.size(), 3);

    EXPECT_TRUE(simulator.next_batch(batch, 3));
    EXPECT_EQ(batch.size(), 1);

    EXPECT_FALSE(simulator.next_batch(batch, 3));
    EXPECT_TRUE(batch.empty());
}