#include <gtest/gtest.h>

#include "events/replay_source.hpp"
#include "common/symbol.hpp"

namespace {

MarketEvent make_trade(
    EventId event_id,
    SequenceNumber sequence,
    Price price)
{
    return MarketEvent{
        .event_id = event_id,
        .sequence_number = sequence,
        .timestamp = Timestamp{
            static_cast<Timestamp::rep>(1000 + sequence)
        },
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = price,
            .quantity = 100
        }
    };
}

}

TEST(ReplaySourceTest, ReplaysEventsInOrder)
{
    EventLog log;

    log.append(make_trade(1, 1, 15000));
    log.append(make_trade(2, 2, 15100));
    log.append(make_trade(3, 3, 15200));

    ReplaySource source(log);

    MarketEvent event{};

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 1);
    EXPECT_EQ(event.sequence_number, 1);

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 2);
    EXPECT_EQ(event.sequence_number, 2);

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 3);
    EXPECT_EQ(event.sequence_number, 3);

    EXPECT_FALSE(source.next_event(event));
    EXPECT_TRUE(source.exhausted());
}

TEST(ReplaySourceTest, TracksPosition)
{
    EventLog log;

    log.append(make_trade(1, 1, 15000));
    log.append(make_trade(2, 2, 15100));

    ReplaySource source(log);

    EXPECT_EQ(source.position(), 0);

    MarketEvent event{};

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(source.position(), 1);

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(source.position(), 2);

    EXPECT_TRUE(source.exhausted());
}

TEST(ReplaySourceTest, ResetRestartsReplay)
{
    EventLog log;

    log.append(make_trade(1, 1, 15000));
    log.append(make_trade(2, 2, 15100));

    ReplaySource source(log);

    MarketEvent event{};

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 1);

    source.reset();

    EXPECT_EQ(source.position(), 0);
    EXPECT_FALSE(source.exhausted());

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 1);
}

TEST(ReplaySourceTest, ReplaysBatchInOrder)
{
    EventLog log;

    log.append(make_trade(1, 1, 15000));
    log.append(make_trade(2, 2, 15100));
    log.append(make_trade(3, 3, 15200));

    ReplaySource source(log);

    std::vector<MarketEvent> batch;

    ASSERT_TRUE(source.next_batch(batch, 2));

    ASSERT_EQ(batch.size(), 2);
    EXPECT_EQ(batch[0].event_id, 1);
    EXPECT_EQ(batch[1].event_id, 2);
    EXPECT_EQ(source.position(), 2);

    ASSERT_TRUE(source.next_batch(batch, 2));

    ASSERT_EQ(batch.size(), 1);
    EXPECT_EQ(batch[0].event_id, 3);
    EXPECT_EQ(source.position(), 3);

    EXPECT_FALSE(source.next_batch(batch, 2));
}

TEST(ReplaySourceTest, RejectsZeroBatchSize)
{
    EventLog log;

    log.append(make_trade(1, 1, 15000));

    ReplaySource source(log);

    std::vector<MarketEvent> batch;

    EXPECT_FALSE(source.next_batch(batch, 0));
    EXPECT_EQ(source.position(), 0);
}