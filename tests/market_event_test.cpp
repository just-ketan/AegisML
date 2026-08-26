#include <gtest/gtest.h>

#include "market_event.hpp"


TEST(MarketEventTest, ValidEventIsAccepted) {

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{100},
        .symbol = {'A', 'A', 'P', 'L'},
        .event_type = EventType::Trade,
        .price = 2254300,
        .quantity = 100
    };

    EXPECT_TRUE(is_valid(event));
}


TEST(MarketEventTest, ZeroEventIdIsRejected) {

    MarketEvent event{
        .event_id = 0,
        .sequence_number = 1,
        .timestamp = Timestamp{100},
        .symbol = {'A', 'A', 'P', 'L'},
        .event_type = EventType::Trade,
        .price = 2254300,
        .quantity = 100
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, EmptySymbolIsRejected) {

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{100},
        .symbol = {},
        .event_type = EventType::Trade,
        .price = 2254300,
        .quantity = 100
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, NonPositivePriceIsRejected) {

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{100},
        .symbol = {'A', 'A', 'P', 'L'},
        .event_type = EventType::Trade,
        .price = 0,
        .quantity = 100
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, NonPositiveQuantityIsRejected) {

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{100},
        .symbol = {'A', 'A', 'P', 'L'},
        .event_type = EventType::Trade,
        .price = 2254300,
        .quantity = 0
    };

    EXPECT_FALSE(is_valid(event));
}