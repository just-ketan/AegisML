#include <gtest/gtest.h>

#include <vector>

#include "simulator/market_simulator.hpp"
#include "common/symbol.hpp"
#include "time/simulation_clock.hpp"

TEST(MarketSimulatorTest, GeneratesExactNumberOfEvents)
{
    SimulationConfig config{
        .event_count = 100,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT"),
            make_symbol("GOOGL")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    MarketEvent event;
    std::size_t count = 0;

    while (simulator.next_event(event)) {
        ++count;
    }

    EXPECT_EQ(count, 100);
}


TEST(MarketSimulatorTest, StopsAfterConfiguredEventCount)
{
    SimulationConfig config{
        .event_count = 5,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    MarketEvent event;

    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(simulator.next_event(event));
    }

    EXPECT_FALSE(simulator.next_event(event));
}


TEST(MarketSimulatorTest, EventIdsAreSequential)
{
    SimulationConfig config{
        .event_count = 100,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    MarketEvent event;
    EventId expected_id = 1;

    while (simulator.next_event(event)) {
        EXPECT_EQ(event.event_id, expected_id);
        ++expected_id;
    }

    EXPECT_EQ(expected_id, 101);
}


TEST(MarketSimulatorTest, SequenceNumbersAreSequential)
{
    SimulationConfig config{
        .event_count = 100,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    MarketEvent event;
    SequenceNumber expected_sequence = 1;

    while (simulator.next_event(event)) {
        EXPECT_EQ(event.sequence_number, expected_sequence);
        ++expected_sequence;
    }

    EXPECT_EQ(expected_sequence, 101);
}


TEST(MarketSimulatorTest, TimestampsFollowClock)
{
    SimulationConfig config{
        .event_count = 3,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT")
        }
    };

    SimulationClock clock{Timestamp{1000}};
    MarketSimulator simulator(config, clock);

    MarketEvent first;
    MarketEvent second;
    MarketEvent third;

    ASSERT_TRUE(simulator.next_event(first));
    EXPECT_EQ(first.timestamp, Timestamp{1000});

    clock.advance(Timestamp{250});

    ASSERT_TRUE(simulator.next_event(second));
    EXPECT_EQ(second.timestamp, Timestamp{1250});

    clock.advance(Timestamp{500});

    ASSERT_TRUE(simulator.next_event(third));
    EXPECT_EQ(third.timestamp, Timestamp{1750});
}


TEST(MarketSimulatorTest, GeneratedEventsAreValid)
{
    SimulationConfig config{
        .event_count = 1000,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT"),
            make_symbol("GOOGL"),
            make_symbol("AMZN"),
            make_symbol("TSLA")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    MarketEvent event;

    while (simulator.next_event(event)) {
        EXPECT_TRUE(is_valid(event));
    }
}


TEST(MarketSimulatorTest, DeterministicWithSameSeed)
{
    SimulationConfig config{
        .event_count = 100,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT"),
            make_symbol("GOOGL")
        }
    };

    SimulationClock clock_a;
    SimulationClock clock_b;

    MarketSimulator simulator_a(config, clock_a);
    MarketSimulator simulator_b(config, clock_b);

    MarketEvent event_a;
    MarketEvent event_b;

    while (simulator_a.next_event(event_a) &&
           simulator_b.next_event(event_b)) {

        EXPECT_EQ(event_a.event_id, event_b.event_id);
        EXPECT_EQ(event_a.sequence_number, event_b.sequence_number);
        EXPECT_EQ(event_a.timestamp, event_b.timestamp);
        EXPECT_EQ(event_a.symbol, event_b.symbol);
        EXPECT_EQ(event_a.payload, event_b.payload);
    }
}


TEST(MarketSimulatorTest, DifferentSeedsProduceDifferentStreams)
{
    SimulationConfig config_a{
        .event_count = 100,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT"),
            make_symbol("GOOGL")
        }
    };

    SimulationConfig config_b{
        .event_count = 100,
        .seed = 12345,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT"),
            make_symbol("GOOGL")
        }
    };

    SimulationClock clock_a;
    SimulationClock clock_b;

    MarketSimulator simulator_a(config_a, clock_a);
    MarketSimulator simulator_b(config_b, clock_b);

    MarketEvent event_a;
    MarketEvent event_b;

    bool found_difference = false;

    while (simulator_a.next_event(event_a) &&
           simulator_b.next_event(event_b)) {

        if (event_a.symbol != event_b.symbol ||
            event_a.payload != event_b.payload) {

            found_difference = true;
            break;
        }
    }

    EXPECT_TRUE(found_difference);
}


TEST(MarketSimulatorTest, GeneratesDifferentEventTypes)
{
    SimulationConfig config{
        .event_count = 1000,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT"),
            make_symbol("GOOGL")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    bool has_trade = false;
    bool has_quote = false;
    bool has_add = false;
    bool has_cancel = false;
    bool has_execute = false;

    MarketEvent event;

    while (simulator.next_event(event)) {
        switch (event.payload.index()) {
            case 0:
                has_trade = true;
                break;

            case 1:
                has_quote = true;
                break;

            case 2:
                has_add = true;
                break;

            case 3:
                has_cancel = true;
                break;

            case 4:
                has_execute = true;
                break;
        }
    }

    EXPECT_TRUE(has_trade);
    EXPECT_TRUE(has_quote);
    EXPECT_TRUE(has_add);
    EXPECT_TRUE(has_cancel);
    EXPECT_TRUE(has_execute);
}


TEST(MarketSimulatorTest, BatchReturnsRequestedNumberOfEvents)
{
    SimulationConfig config{
        .event_count = 10,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

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


TEST(MarketSimulatorTest, BatchEventsHaveSequentialIds)
{
    SimulationConfig config{
        .event_count = 10,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    std::vector<MarketEvent> batch;

    EventId expected_id = 1;

    while (simulator.next_batch(batch, 3)) {
        for (const auto& event : batch) {
            EXPECT_EQ(event.event_id, expected_id);
            ++expected_id;
        }
    }

    EXPECT_EQ(expected_id, 11);
}


TEST(MarketSimulatorTest, BatchDoesNotExceedRequestedSize)
{
    SimulationConfig config{
        .event_count = 20,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    std::vector<MarketEvent> batch;

    while (simulator.next_batch(batch, 4)) {
        EXPECT_LE(batch.size(), 4);
        EXPECT_FALSE(batch.empty());
    }
}


TEST(MarketSimulatorTest, EmptyBatchSizeIsRejected)
{
    SimulationConfig config{
        .event_count = 10,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL")
        }
    };

    SimulationClock clock;
MarketSimulator simulator(config, clock);

    std::vector<MarketEvent> batch;

    EXPECT_FALSE(simulator.next_batch(batch, 0));
    EXPECT_TRUE(batch.empty());
}

TEST(MarketSimulatorTest, UsesClockTimestamp)
{
    SimulationConfig config{
        .event_count = 1,
        .seed = 42,
        .symbols = {make_symbol("AAPL")}
    };

    SimulationClock clock{Timestamp{5000}};
    MarketSimulator simulator{config, clock};

    MarketEvent event;

    ASSERT_TRUE(simulator.next_event(event));
    EXPECT_EQ(event.timestamp, Timestamp{5000});
}

TEST(MarketSimulatorTest, ClockControlsEventTimestamp)
{
    SimulationConfig config{
        .event_count = 2,
        .seed = 42,
        .symbols = {make_symbol("AAPL")}
    };

    SimulationClock clock{Timestamp{1000}};
    MarketSimulator simulator{config, clock};

    MarketEvent first;
    MarketEvent second;

    ASSERT_TRUE(simulator.next_event(first));

    clock.advance(Timestamp{250});

    ASSERT_TRUE(simulator.next_event(second));

    EXPECT_EQ(first.timestamp, Timestamp{1000});
    EXPECT_EQ(second.timestamp, Timestamp{1250});
}