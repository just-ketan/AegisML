#include <gtest/gtest.h>

#include "events/event_serializer.hpp"
#include "common/symbol.hpp"

namespace {

MarketEvent make_trade_event()
{
    return MarketEvent{
        .event_id = 42,
        .sequence_number = 7,
        .timestamp = Timestamp{123456},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 250
        }
    };
}

}

TEST(EventSerializerTest, RoundTripsTradeEvent)
{
    const MarketEvent original = make_trade_event();

    const auto bytes =
        EventSerializer::serialize(original);

    const MarketEvent restored =
        EventSerializer::deserialize(bytes);

    EXPECT_EQ(restored.event_id, original.event_id);
    EXPECT_EQ(
        restored.sequence_number,
        original.sequence_number
    );
    EXPECT_EQ(restored.timestamp, original.timestamp);
    EXPECT_EQ(restored.symbol, original.symbol);

    ASSERT_TRUE(
        std::holds_alternative<TradeEvent>(
            restored.payload
        )
    );

    const auto& original_trade =
        std::get<TradeEvent>(original.payload);

    const auto& restored_trade =
        std::get<TradeEvent>(restored.payload);

    EXPECT_EQ(
        restored_trade.price,
        original_trade.price
    );

    EXPECT_EQ(
        restored_trade.quantity,
        original_trade.quantity
    );
}

TEST(EventSerializerTest, RoundTripsQuoteEvent)
{
    const MarketEvent original{
        .event_id = 10,
        .sequence_number = 20,
        .timestamp = Timestamp{5000},
        .symbol = make_symbol("MSFT"),
        .payload = QuoteEvent{
            .bid_price = 14900,
            .bid_quantity = 100,
            .asking_price = 15000,
            .asking_quantity = 200
        }
    };

    const auto bytes =
        EventSerializer::serialize(original);

    const MarketEvent restored =
        EventSerializer::deserialize(bytes);

    EXPECT_EQ(restored.event_id, original.event_id);
    EXPECT_EQ(restored.sequence_number, original.sequence_number);
    EXPECT_EQ(restored.timestamp, original.timestamp);
    EXPECT_EQ(restored.symbol, original.symbol);

    ASSERT_TRUE(
        std::holds_alternative<QuoteEvent>(
            restored.payload
        )
    );

    const auto& expected =
        std::get<QuoteEvent>(original.payload);

    const auto& actual =
        std::get<QuoteEvent>(restored.payload);

    EXPECT_EQ(actual.bid_price, expected.bid_price);
    EXPECT_EQ(actual.bid_quantity, expected.bid_quantity);
    EXPECT_EQ(actual.asking_price, expected.asking_price);
    EXPECT_EQ(actual.asking_quantity, expected.asking_quantity);
}

TEST(EventSerializerTest, RoundTripsAddOrderEvent)
{
    const MarketEvent original{
        .event_id = 11,
        .sequence_number = 21,
        .timestamp = Timestamp{6000},
        .symbol = make_symbol("GOOG"),
        .payload = AddOrderEvent{
            .order_id = 900,
            .side = Side::Buy,
            .price = 17500,
            .quantity = 300
        }
    };

    const auto bytes =
        EventSerializer::serialize(original);

    const MarketEvent restored =
        EventSerializer::deserialize(bytes);

    ASSERT_TRUE(
        std::holds_alternative<AddOrderEvent>(
            restored.payload
        )
    );

    const auto& expected =
        std::get<AddOrderEvent>(original.payload);

    const auto& actual =
        std::get<AddOrderEvent>(restored.payload);

    EXPECT_EQ(restored.event_id, original.event_id);
    EXPECT_EQ(restored.sequence_number, original.sequence_number);
    EXPECT_EQ(restored.timestamp, original.timestamp);
    EXPECT_EQ(restored.symbol, original.symbol);

    EXPECT_EQ(actual.order_id, expected.order_id);
    EXPECT_EQ(actual.side, expected.side);
    EXPECT_EQ(actual.price, expected.price);
    EXPECT_EQ(actual.quantity, expected.quantity);
}


TEST(EventSerializerTest, RoundTripsCancelOrderEvent)
{
    const MarketEvent original{
        .event_id = 12,
        .sequence_number = 22,
        .timestamp = Timestamp{7000},
        .symbol = make_symbol("TSLA"),
        .payload = CancelOrderEvent{
            .order_id = 901,
            .quantity = 125
        }
    };

    const auto bytes =
        EventSerializer::serialize(original);

    const MarketEvent restored =
        EventSerializer::deserialize(bytes);

    ASSERT_TRUE(
        std::holds_alternative<CancelOrderEvent>(
            restored.payload
        )
    );

    const auto& expected =
        std::get<CancelOrderEvent>(original.payload);

    const auto& actual =
        std::get<CancelOrderEvent>(restored.payload);

    EXPECT_EQ(actual.order_id, expected.order_id);
    EXPECT_EQ(actual.quantity, expected.quantity);
}

TEST(EventSerializerTest, RoundTripsExecuteOrderEvent)
{
    const MarketEvent original{
        .event_id = 13,
        .sequence_number = 23,
        .timestamp = Timestamp{8000},
        .symbol = make_symbol("NVDA"),
        .payload = ExecuteOrderEvent{
            .order_id = 902,
            .price = 18000,
            .quantity = 75
        }
    };

    const auto bytes =
        EventSerializer::serialize(original);

    const MarketEvent restored =
        EventSerializer::deserialize(bytes);

    ASSERT_TRUE(
        std::holds_alternative<ExecuteOrderEvent>(
            restored.payload
        )
    );

    const auto& expected =
        std::get<ExecuteOrderEvent>(original.payload);

    const auto& actual =
        std::get<ExecuteOrderEvent>(restored.payload);

    EXPECT_EQ(actual.order_id, expected.order_id);
    EXPECT_EQ(actual.price, expected.price);
    EXPECT_EQ(actual.quantity, expected.quantity);
}

