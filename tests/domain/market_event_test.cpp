#include <gtest/gtest.h>

#include "events/market_event.hpp"
#include "common/symbol.hpp"
#include "events/event_sequence.hpp"

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

TEST(MarketEventTest, AcceptsZeroTimestamp)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{0},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_TRUE(is_valid(event));
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

// ------------------------------------------------------------
// Event sequence validation
// ------------------------------------------------------------

TEST(EventSequenceValidatorTest, AcceptsInitialSequence)
{
    EventSequenceValidator validator;

    EXPECT_TRUE(validator.accept(1));
    EXPECT_EQ(validator.next_expected(), 2);
}


TEST(EventSequenceValidatorTest, AcceptsStrictlyIncreasingSequence)
{
    EventSequenceValidator validator;

    EXPECT_TRUE(validator.accept(1));
    EXPECT_TRUE(validator.accept(2));
    EXPECT_TRUE(validator.accept(3));

    EXPECT_EQ(validator.next_expected(), 4);
}


TEST(EventSequenceValidatorTest, RejectsDuplicateSequence)
{
    EventSequenceValidator validator;

    EXPECT_TRUE(validator.accept(1));
    EXPECT_FALSE(validator.accept(1));

    EXPECT_EQ(validator.next_expected(), 2);
}


TEST(EventSequenceValidatorTest, RejectsSequenceGap)
{
    EventSequenceValidator validator;

    EXPECT_TRUE(validator.accept(1));
    EXPECT_FALSE(validator.accept(3));

    EXPECT_EQ(validator.next_expected(), 2);
}


TEST(EventSequenceValidatorTest, RejectsOutOfOrderSequence)
{
    EventSequenceValidator validator;

    EXPECT_TRUE(validator.accept(1));
    EXPECT_TRUE(validator.accept(2));

    EXPECT_FALSE(validator.accept(1));

    EXPECT_EQ(validator.next_expected(), 3);
}


TEST(EventSequenceValidatorTest, RejectsZeroSequence)
{
    EventSequenceValidator validator;

    EXPECT_FALSE(validator.accept(0));

    EXPECT_EQ(validator.next_expected(), 1);
}


TEST(EventSequenceValidatorTest, ResetRestoresInitialState)
{
    EventSequenceValidator validator;

    EXPECT_TRUE(validator.accept(1));
    EXPECT_TRUE(validator.accept(2));

    validator.reset();

    EXPECT_EQ(validator.next_expected(), 1);
    EXPECT_TRUE(validator.accept(1));
}

TEST(MarketEventTest, ClassifiesTradeEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{0},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_EQ(event_type(event), EventType::Trade);
}

TEST(MarketEventTest, ClassifiesQuoteEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{0},
        .symbol = make_symbol("AAPL"),
        .payload = QuoteEvent{
            .bid_price = 15000,
            .bid_quantity = 100,
            .asking_price = 15100,
            .asking_quantity = 100
        }
    };

    EXPECT_EQ(event_type(event), EventType::Quote);
}

TEST(MarketEventTest, ClassifiesAddOrderEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{0},
        .symbol = make_symbol("AAPL"),
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_EQ(event_type(event), EventType::Add);
}

TEST(MarketEventTest, ClassifiesCancelOrderEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{0},
        .symbol = make_symbol("AAPL"),
        .payload = CancelOrderEvent{
            .order_id = 1,
            .quantity = 100
        }
    };

    EXPECT_EQ(event_type(event), EventType::Cancel);
}

TEST(MarketEventTest, ClassifiesExecuteOrderEvent)
{
    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{0},
        .symbol = make_symbol("AAPL"),
        .payload = ExecuteOrderEvent{
            .order_id = 1,
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_EQ(event_type(event), EventType::Execute);
}