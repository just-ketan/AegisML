#include <gtest/gtest.h>

#include "market_event.hpp"
#include "symbol.hpp"


TEST(MarketEventTest, ValidTradeEvent)
{
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

    EXPECT_TRUE(is_valid(event));
}


TEST(MarketEventTest, ValidQuoteEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = QuoteEvent{
            .bid_price = 14900,
            .bid_quantity = 100,
            .asking_price = 15000,
            .asking_quantity = 200
        }
    };

    EXPECT_TRUE(is_valid(event));
}


TEST(MarketEventTest, ValidAddOrderEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_TRUE(is_valid(event));
}


TEST(MarketEventTest, ValidCancelOrderEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = CancelOrderEvent{
            .order_id = 1,
            .quantity = 50
        }
    };

    EXPECT_TRUE(is_valid(event));
}


TEST(MarketEventTest, ValidExecuteOrderEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = ExecuteOrderEvent{
            .order_id = 1,
            .price = 15000,
            .quantity = 50
        }
    };

    EXPECT_TRUE(is_valid(event));
}


// ------------------------------------------------------------
// Envelope validation
// ------------------------------------------------------------

TEST(MarketEventTest, RejectsZeroEventId)
{
    MarketEvent event{
        .event_id = 0,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsZeroSequenceNumber)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 0,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsNegativeTimestamp)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{-1},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsEmptySymbol)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = Symbol{},
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_FALSE(is_valid(event));
}


// ------------------------------------------------------------
// Trade validation
// ------------------------------------------------------------

TEST(MarketEventTest, RejectsTradeWithZeroPrice)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 0,
            .quantity = 100
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsTradeWithZeroQuantity)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 0
        }
    };

    EXPECT_FALSE(is_valid(event));
}


// ------------------------------------------------------------
// Quote validation
// ------------------------------------------------------------

TEST(MarketEventTest, RejectsCrossedQuote)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = QuoteEvent{
            .bid_price = 15100,
            .bid_quantity = 100,
            .asking_price = 15000,
            .asking_quantity = 200
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsQuoteWithZeroBidQuantity)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = QuoteEvent{
            .bid_price = 14900,
            .bid_quantity = 0,
            .asking_price = 15000,
            .asking_quantity = 200
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsQuoteWithZeroAskingQuantity)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = QuoteEvent{
            .bid_price = 14900,
            .bid_quantity = 100,
            .asking_price = 15000,
            .asking_quantity = 0
        }
    };

    EXPECT_FALSE(is_valid(event));
}


// ------------------------------------------------------------
// Order validation
// ------------------------------------------------------------

TEST(MarketEventTest, RejectsAddOrderWithZeroOrderId)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = AddOrderEvent{
            .order_id = 0,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsAddOrderWithZeroPrice)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Buy,
            .price = 0,
            .quantity = 100
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsAddOrderWithZeroQuantity)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 0
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsCancelWithZeroOrderId)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = CancelOrderEvent{
            .order_id = 0,
            .quantity = 50
        }
    };

    EXPECT_FALSE(is_valid(event));
}


TEST(MarketEventTest, RejectsExecuteWithZeroOrderId)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = ExecuteOrderEvent{
            .order_id = 0,
            .price = 15000,
            .quantity = 50
        }
    };

    EXPECT_FALSE(is_valid(event));
}