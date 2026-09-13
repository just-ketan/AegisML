#include <gtest/gtest.h>

#include "orders/matching_engine.hpp"
#include "common/symbol.hpp"

namespace {

MarketEvent make_add_event(
    EventId event_id,
    OrderId order_id,
    Side side,
    Price price,
    Quantity quantity
) {
    return MarketEvent{
        .event_id = event_id,
        .sequence_number = event_id,
        .timestamp = Timestamp{static_cast<Timestamp::rep>(event_id)},
        .symbol = make_symbol("AAPL"),
        .payload = AddOrderEvent{
            .order_id = order_id,
            .side = side,
            .price = price,
            .quantity = quantity
        }
    };
}

}

TEST(MatchingEngineTest, BuyMatchesBestAsk)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 100)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10100, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
ASSERT_EQ(trade.trade_count(), 1);

    EXPECT_EQ(trade.trades[0].incoming_order_id, 200);
    EXPECT_EQ(trade.trades[0].resting_order_id, 100);
    EXPECT_EQ(trade.trades[0].price, 10000);
    EXPECT_EQ(trade.trades[0].quantity, 100);
}

TEST(MatchingEngineTest, SellMatchesBestBid)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Buy, 10000, 100)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Sell, 9900, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
ASSERT_EQ(trade.trade_count(), 1);

    EXPECT_EQ(trade.trades[0].incoming_order_id, 200);
    EXPECT_EQ(trade.trades[0].resting_order_id, 100);
    EXPECT_EQ(trade.trades[0].price, 10000);
    EXPECT_EQ(trade.trades[0].quantity, 100);
}

TEST(MatchingEngineTest, BuyDoesNotMatchAboveBestAsk)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10100, 100)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10000, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    EXPECT_FALSE(trade.has_trades());
}

TEST(MatchingEngineTest, SellDoesNotMatchBelowBestBid)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Buy, 10000, 100)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Sell, 10100, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    EXPECT_FALSE(trade.has_trades());
}

TEST(MatchingEngineTest, TradeUsesRestingOrderPrice)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 100)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10500, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
ASSERT_EQ(trade.trade_count(), 1);

    EXPECT_EQ(trade.trades[0].price, 10000);
}

TEST(MatchingEngineTest, TradeQuantityUsesSmallerRemainingQuantity)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 60)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10100, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
ASSERT_EQ(trade.trade_count(), 1);

    EXPECT_EQ(trade.trades[0].quantity, 60);

    const Order* incoming = manager.find(200);
    ASSERT_NE(incoming, nullptr);

    EXPECT_EQ(incoming->filled_quantity(), 60);
    EXPECT_EQ(incoming->remaining_quantity(), 40);

    const Order* seller = manager.find(100);
    ASSERT_NE(seller, nullptr);

    EXPECT_EQ(seller->filled_quantity(), 60);
    EXPECT_EQ(seller->remaining_quantity(), 0);
    EXPECT_EQ(seller->state(), OrderState::Filled);
}

TEST(MatchingEngineTest, UnknownIncomingOrderDoesNotMatch)
{
    OrderManager manager;
    OrderBook book;

    MatchingEngine engine(manager, book);

    EXPECT_FALSE(engine.match(999).has_trades());
}

TEST(MatchingEngineTest, FilledIncomingOrderDoesNotMatch)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(manager.execute(100, 100));

    MatchingEngine engine(manager, book);

    EXPECT_FALSE(engine.match(100).has_trades());
}

TEST(MatchingEngineTest, RemovesFullyFilledRestingOrderFromBook)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 100)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));
    ASSERT_TRUE(book.contains(100));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10100, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
ASSERT_EQ(trade.trade_count(), 1);

    EXPECT_EQ(manager.find(100)->state(), OrderState::Filled);
    EXPECT_FALSE(book.contains(100));
    EXPECT_EQ(book.size(), 0);
    EXPECT_FALSE(book.best_ask().has_value());
}

TEST(MatchingEngineTest, KeepsPartiallyFilledRestingOrderInBook)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 100)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10100, 40)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
ASSERT_EQ(trade.trade_count(), 1);

    const Order* updated = manager.find(100);

    ASSERT_NE(updated, nullptr);

    EXPECT_EQ(updated->state(), OrderState::PartiallyFilled);
    EXPECT_EQ(updated->remaining_quantity(), 60);

    EXPECT_TRUE(book.contains(100));
    EXPECT_EQ(book.size(), 1);
    EXPECT_EQ(book.best_ask().value(), 100);
}

TEST(MatchingEngineTest, UnmatchedIncomingOrderRestsInBook)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10100, 100)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10000, 50)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    EXPECT_FALSE(trade.has_trades());

    EXPECT_TRUE(book.contains(200));
    EXPECT_EQ(book.size(), 2);
    EXPECT_EQ(book.best_bid().value(), 200);
    EXPECT_EQ(book.best_ask().value(), 100);
}

TEST(MatchingEngineTest, PartiallyMatchedIncomingOrderRestsInBook)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 40)
    ));

    const Order* resting = manager.find(100);
    ASSERT_NE(resting, nullptr);

    ASSERT_TRUE(book.add(*resting));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10100, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
    ASSERT_EQ(trade.trade_count(), 1);

    const Order* incoming = manager.find(200);
    ASSERT_NE(incoming, nullptr);

    EXPECT_EQ(incoming->state(), OrderState::PartiallyFilled);
    EXPECT_EQ(incoming->remaining_quantity(), 60);

    EXPECT_TRUE(book.contains(200));
    EXPECT_FALSE(book.contains(100));

    EXPECT_EQ(book.size(), 1);
    EXPECT_EQ(book.best_bid().value(), 200);
}

TEST(MatchingEngineTest, MatchesAcrossMultiplePriceLevels)
{
    OrderManager manager;
    OrderBook book;

    // Resting asks:
    // 40 @ 100
    // 30 @ 101
    // 50 @ 102
    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 40)
    ));
    ASSERT_TRUE(manager.process(
        make_add_event(2, 101, Side::Sell, 10100, 30)
    ));
    ASSERT_TRUE(manager.process(
        make_add_event(3, 102, Side::Sell, 10200, 50)
    ));

    ASSERT_TRUE(book.add(*manager.find(100)));
    ASSERT_TRUE(book.add(*manager.find(101)));
    ASSERT_TRUE(book.add(*manager.find(102)));

    // Incoming BUY needs 70.
    ASSERT_TRUE(manager.process(
        make_add_event(4, 200, Side::Buy, 10200, 70)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
    ASSERT_EQ(trade.trade_count(), 2);

    // First match should consume the best ask.
    EXPECT_EQ(trade.trades[0].incoming_order_id, 200);
    EXPECT_EQ(trade.trades[0].resting_order_id, 100);
    EXPECT_EQ(trade.trades[0].price, 10000);
    EXPECT_EQ(trade.trades[0].quantity, 40);

    EXPECT_EQ(trade.trades[1].incoming_order_id, 200);
    EXPECT_EQ(trade.trades[1].resting_order_id, 101);
    EXPECT_EQ(trade.trades[1].price, 10100);
    EXPECT_EQ(trade.trades[1].quantity, 30);

    const Order* incoming = manager.find(200);
    ASSERT_NE(incoming, nullptr);

    EXPECT_EQ(incoming->state(), OrderState::Filled);
    EXPECT_EQ(incoming->remaining_quantity(), 0);

    EXPECT_EQ(manager.find(100)->state(), OrderState::Filled);
    EXPECT_EQ(manager.find(101)->state(), OrderState::Filled);
    EXPECT_EQ(manager.find(101)->remaining_quantity(), 0); // 30 fully consumed

    EXPECT_FALSE(book.contains(100));
    EXPECT_FALSE(book.contains(101));
    EXPECT_TRUE(book.contains(102));

    EXPECT_EQ(book.size(), 1);
    EXPECT_EQ(book.best_ask().value(), 102);
}

TEST(MatchingEngineTest, RestsRemainingQuantityAfterMultipleMatches)
{
    OrderManager manager;
    OrderBook book;

    // Resting asks:
    // 40 @ 100
    // 30 @ 101
    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 40)
    ));
    ASSERT_TRUE(manager.process(
        make_add_event(2, 101, Side::Sell, 10100, 30)
    ));

    ASSERT_TRUE(book.add(*manager.find(100)));
    ASSERT_TRUE(book.add(*manager.find(101)));

    // Incoming BUY needs 100.
    // Only 70 is available.
    ASSERT_TRUE(manager.process(
        make_add_event(3, 200, Side::Buy, 10200, 100)
    ));

    MatchingEngine engine(manager, book);

    const auto trade = engine.match(200);

    ASSERT_TRUE(trade.has_trades());
    ASSERT_EQ(trade.trade_count(), 2);

    EXPECT_EQ(trade.trades[0].resting_order_id, 100);
    EXPECT_EQ(trade.trades[0].price, 10000);
    EXPECT_EQ(trade.trades[0].quantity, 40);

    EXPECT_EQ(trade.trades[1].resting_order_id, 101);
    EXPECT_EQ(trade.trades[1].price, 10100);
    EXPECT_EQ(trade.trades[1].quantity, 30);

    const Order* incoming = manager.find(200);
    ASSERT_NE(incoming, nullptr);

    EXPECT_EQ(incoming->state(), OrderState::PartiallyFilled);
    EXPECT_EQ(incoming->remaining_quantity(), 30);

    EXPECT_FALSE(book.contains(100));
    EXPECT_FALSE(book.contains(101));

    EXPECT_TRUE(book.contains(200));
    EXPECT_EQ(book.size(), 1);

    EXPECT_EQ(book.best_bid().value(), 200);
}

TEST(MatchingEngineTest, ReportsTradesInExecutionOrder)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 20)
    ));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 101, Side::Sell, 10100, 30)
    ));

    ASSERT_TRUE(manager.process(
        make_add_event(3, 102, Side::Sell, 10200, 40)
    ));

    ASSERT_TRUE(book.add(*manager.find(100)));
    ASSERT_TRUE(book.add(*manager.find(101)));
    ASSERT_TRUE(book.add(*manager.find(102)));

    ASSERT_TRUE(manager.process(
        make_add_event(4, 200, Side::Buy, 10200, 70)
    ));

    MatchingEngine engine(manager, book);

    const auto result = engine.match(200);

    ASSERT_TRUE(result.has_trades());
    ASSERT_EQ(result.trade_count(), 3);

    EXPECT_EQ(result.trades[0].resting_order_id, 100);
    EXPECT_EQ(result.trades[0].price, 10000);
    EXPECT_EQ(result.trades[0].quantity, 20);

    EXPECT_EQ(result.trades[1].resting_order_id, 101);
    EXPECT_EQ(result.trades[1].price, 10100);
    EXPECT_EQ(result.trades[1].quantity, 30);

    EXPECT_EQ(result.trades[2].resting_order_id, 102);
    EXPECT_EQ(result.trades[2].price, 10200);
    EXPECT_EQ(result.trades[2].quantity, 20);
}

TEST(MatchingEngineTest, MatchPreservesQuantityConservation)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 100)
    ));

    ASSERT_TRUE(book.add(*manager.find(100)));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10100, 60)
    ));

    MatchingEngine engine(manager, book);

    const auto result = engine.match(200);

    ASSERT_TRUE(result.has_trades());
    ASSERT_EQ(result.trade_count(), 1);

    const Trade& trade = result.trades[0];

    const Order* incoming = manager.find(200);
    const Order* resting = manager.find(100);

    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(resting, nullptr);

    EXPECT_EQ(trade.quantity, 60);

    EXPECT_EQ(
        incoming->filled_quantity() + incoming->remaining_quantity(),
        incoming->quantity()
    );

    EXPECT_EQ(
        resting->filled_quantity() + resting->remaining_quantity(),
        resting->quantity()
    );

    EXPECT_EQ(incoming->filled_quantity(), trade.quantity);
    EXPECT_EQ(resting->filled_quantity(), trade.quantity);
}

TEST(MatchingEngineTest, MatchOwnsIncomingOrderResting)
{
    OrderManager manager;
    OrderBook book;

    ASSERT_TRUE(manager.process(
        make_add_event(1, 100, Side::Sell, 10000, 40)
    ));

    ASSERT_TRUE(book.add(*manager.find(100)));

    ASSERT_TRUE(manager.process(
        make_add_event(2, 200, Side::Buy, 10100, 100)
    ));

    // Incoming order exists in OrderManager but has NOT
    // been manually inserted into the OrderBook.
    EXPECT_FALSE(book.contains(200));

    MatchingEngine engine(manager, book);

    const auto result = engine.match(200);

    ASSERT_TRUE(result.has_trades());
    ASSERT_EQ(result.trade_count(), 1);

    const Order* incoming = manager.find(200);

    ASSERT_NE(incoming, nullptr);
    EXPECT_EQ(incoming->state(), OrderState::PartiallyFilled);
    EXPECT_EQ(incoming->remaining_quantity(), 60);

    EXPECT_TRUE(book.contains(200));
    EXPECT_EQ(book.best_bid().value(), 200);
}

