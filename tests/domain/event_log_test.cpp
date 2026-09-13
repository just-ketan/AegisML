#include <gtest/gtest.h>

#include "events/event_log.hpp"
#include "common/symbol.hpp"

TEST(EventLogTest, StartsEmpty)
{
    EventLog log;

    EXPECT_EQ(log.size(), 0);
    EXPECT_TRUE(log.events().empty());
}

TEST(EventLogTest, AppendsEvent)
{
    EventLog log;

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    log.append(event);

    ASSERT_EQ(log.size(), 1);

    const MarketEvent& stored = log.at(0);

    EXPECT_EQ(stored.event_id, event.event_id);
    EXPECT_EQ(stored.sequence_number, event.sequence_number);
    EXPECT_EQ(stored.timestamp, event.timestamp);
    EXPECT_EQ(stored.symbol, event.symbol);

    ASSERT_TRUE(std::holds_alternative<TradeEvent>(stored.payload));

    const auto& stored_trade =
        std::get<TradeEvent>(stored.payload);

    const auto& original_trade =
        std::get<TradeEvent>(event.payload);

    EXPECT_EQ(stored_trade.price, original_trade.price);
    EXPECT_EQ(stored_trade.quantity, original_trade.quantity);
}

TEST(EventLogTest, PreservesEventOrder)
{
    EventLog log;

    MarketEvent first{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    MarketEvent second{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15100,
            .quantity = 50
        }
    };

    log.append(first);
    log.append(second);

    ASSERT_EQ(log.size(), 2);

    EXPECT_EQ(log.at(0).event_id, first.event_id);
    EXPECT_EQ(log.at(1).event_id, second.event_id);

    EXPECT_EQ(log.at(0).sequence_number, first.sequence_number);
    EXPECT_EQ(log.at(1).sequence_number, second.sequence_number);
}

TEST(EventLogTest, AtThrowsForInvalidIndex)
{
    EventLog log;

    EXPECT_THROW(log.at(0), std::out_of_range);
}

TEST(EventLogTest, ClearRemovesAllEvents)
{
    EventLog log;

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    log.append(event);

    ASSERT_EQ(log.size(), 1);

    log.clear();

    EXPECT_EQ(log.size(), 0);
    EXPECT_TRUE(log.events().empty());
}