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

    ASSERT_TRUE(trade.has_value());

    EXPECT_EQ(trade->incoming_order_id, 200);
    EXPECT_EQ(trade->resting_order_id, 100);
    EXPECT_EQ(trade->price, 10000);
    EXPECT_EQ(trade->quantity, 100);
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

    ASSERT_TRUE(trade.has_value());

    EXPECT_EQ(trade->incoming_order_id, 200);
    EXPECT_EQ(trade->resting_order_id, 100);
    EXPECT_EQ(trade->price, 10000);
    EXPECT_EQ(trade->quantity, 100);
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

    EXPECT_FALSE(trade.has_value());
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

    EXPECT_FALSE(trade.has_value());
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

    ASSERT_TRUE(trade.has_value());

    EXPECT_EQ(trade->price, 10000);
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

    ASSERT_TRUE(trade.has_value());

    EXPECT_EQ(trade->quantity, 60);

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

    EXPECT_FALSE(engine.match(999).has_value());
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

    EXPECT_FALSE(engine.match(100).has_value());
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

    ASSERT_TRUE(trade.has_value());

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

    ASSERT_TRUE(trade.has_value());

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

    EXPECT_FALSE(trade.has_value());

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

    ASSERT_TRUE(trade.has_value());

    const Order* incoming = manager.find(200);
    ASSERT_NE(incoming, nullptr);

    EXPECT_EQ(incoming->state(), OrderState::PartiallyFilled);
    EXPECT_EQ(incoming->remaining_quantity(), 60);

    EXPECT_TRUE(book.contains(200));
    EXPECT_FALSE(book.contains(100));

    EXPECT_EQ(book.size(), 1);
    EXPECT_EQ(book.best_bid().value(), 200);
}