#include <gtest/gtest.h>

#include "orders/order_book.hpp"

namespace {

Order make_order(
    OrderId id,
    Side side,
    Price price,
    Quantity quantity
) {
    return Order{
        id,
        side,
        price,
        quantity
    };
}

} // namespace

TEST(OrderBookTest, AddsBuyOrder)
{
    OrderBook book;

    const Order order = make_order(
        1,
        Side::Buy,
        10000,
        100
    );

    EXPECT_TRUE(book.add(order));
    EXPECT_EQ(book.size(), 1);
}

TEST(OrderBookTest, AddsSellOrder)
{
    OrderBook book;

    const Order order = make_order(
        1,
        Side::Sell,
        10010,
        100
    );

    EXPECT_TRUE(book.add(order));
    EXPECT_EQ(book.size(), 1);
}

TEST(OrderBookTest, RejectsDuplicateOrderId)
{
    OrderBook book;

    const Order first = make_order(
        1,
        Side::Buy,
        10000,
        100
    );

    const Order duplicate = make_order(
        1,
        Side::Sell,
        10010,
        200
    );

    ASSERT_TRUE(book.add(first));

    EXPECT_FALSE(book.add(duplicate));
    EXPECT_EQ(book.size(), 1);
}

TEST(OrderBookTest, MaintainsSeparateBidAndAskSides)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(2, Side::Sell, 10010, 100)
    ));

    EXPECT_EQ(book.size(), 2);
}

TEST(OrderBookTest, MaintainsMultipleOrdersAtSamePrice)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(2, Side::Buy, 10000, 200)
    ));

    ASSERT_TRUE(book.add(
        make_order(3, Side::Buy, 10000, 300)
    ));

    EXPECT_EQ(book.size(), 3);
}

TEST(OrderBookTest, MaintainsMultiplePriceLevels)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(2, Side::Buy, 9990, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(3, Side::Buy, 9980, 100)
    ));

    EXPECT_EQ(book.size(), 3);
}

TEST(OrderBookTest, RejectsZeroQuantity)
{
    OrderBook book;

    const Order order = make_order(
        1,
        Side::Buy,
        10000,
        0
    );

    EXPECT_FALSE(book.add(order));
    EXPECT_EQ(book.size(), 0);
}

TEST(OrderBookTest, RejectsFilledOrder)
{
    OrderBook book;

    Order order = make_order(
        1,
        Side::Buy,
        10000,
        100
    );

    ASSERT_TRUE(order.apply_fill(100));
    ASSERT_EQ(order.state(), OrderState::Filled);

    EXPECT_FALSE(book.add(order));
    EXPECT_EQ(book.size(), 0);
}

TEST(OrderBookTest, RejectsCancelledOrder)
{
    OrderBook book;

    Order order = make_order(
        1,
        Side::Buy,
        10000,
        100
    );

    ASSERT_TRUE(order.transition(OrderState::CancelPending));
    ASSERT_TRUE(order.transition(OrderState::Cancelled));

    EXPECT_FALSE(book.add(order));
    EXPECT_EQ(book.size(), 0);
}

// test price priorities
TEST(OrderBookTest, ReturnsHighestBid)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(2, Side::Buy, 10050, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(3, Side::Buy, 9990, 100)
    ));

    const auto best_bid = book.best_bid();

    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 2);
}

TEST(OrderBookTest, ReturnsLowestAsk)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Sell, 10020, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(2, Side::Sell, 10010, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(3, Side::Sell, 10030, 100)
    ));

    const auto best_ask = book.best_ask();

    ASSERT_TRUE(best_ask.has_value());
    EXPECT_EQ(*best_ask, 2);
}

TEST(OrderBookTest, BestBidIsEmptyForEmptyBook)
{
    OrderBook book;

    EXPECT_FALSE(book.best_bid().has_value());
}

TEST(OrderBookTest, BestAskIsEmptyForEmptyBook)
{
    OrderBook book;

    EXPECT_FALSE(book.best_ask().has_value());
}

TEST(OrderBookTest, MaintainsTimePriorityAtSameBidPrice)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(101, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(102, Side::Buy, 10000, 200)
    ));

    ASSERT_TRUE(book.add(
        make_order(103, Side::Buy, 10000, 300)
    ));

    const auto best_bid = book.best_bid();

    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 101);
}

TEST(OrderBookTest, MaintainsTimePriorityAtSameAskPrice)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(201, Side::Sell, 10010, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(202, Side::Sell, 10010, 200)
    ));

    ASSERT_TRUE(book.add(
        make_order(203, Side::Sell, 10010, 300)
    ));

    const auto best_ask = book.best_ask();

    ASSERT_TRUE(best_ask.has_value());
    EXPECT_EQ(*best_ask, 201);
}

TEST(OrderBookTest, RemovesExistingOrder)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_EQ(book.size(), 1);

    EXPECT_TRUE(book.remove(1));
    EXPECT_EQ(book.size(), 0);
}

TEST(OrderBookTest, RejectsRemovalOfUnknownOrder)
{
    OrderBook book;

    EXPECT_FALSE(book.remove(999));
    EXPECT_EQ(book.size(), 0);
}

TEST(OrderBookTest, RemovesOnlyRequestedOrder)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(2, Side::Buy, 10000, 200)
    ));

    ASSERT_TRUE(book.add(
        make_order(3, Side::Buy, 10000, 300)
    ));

    ASSERT_TRUE(book.remove(2));

    EXPECT_EQ(book.size(), 2);

    const auto best_bid = book.best_bid();

    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 1);
}

TEST(OrderBookTest, RemovingFirstOrderAdvancesTimePriority)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(2, Side::Buy, 10000, 200)
    ));

    ASSERT_TRUE(book.add(
        make_order(3, Side::Buy, 10000, 300)
    ));

    ASSERT_TRUE(book.remove(1));

    const auto best_bid = book.best_bid();

    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 2);
}

TEST(OrderBookTest, RemovingLastOrderAtPriceRemovesPriceLevel)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.add(
        make_order(2, Side::Buy, 9990, 100)
    ));

    ASSERT_TRUE(book.remove(1));

    EXPECT_EQ(book.size(), 1);

    const auto best_bid = book.best_bid();

    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 2);
}

TEST(OrderBookTest, ContainsAddedOrder)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    EXPECT_TRUE(book.contains(1));
}

TEST(OrderBookTest, DoesNotContainUnknownOrder)
{
    OrderBook book;

    EXPECT_FALSE(book.contains(999));
}

TEST(OrderBookTest, DoesNotContainRemovedOrder)
{
    OrderBook book;

    ASSERT_TRUE(book.add(
        make_order(1, Side::Buy, 10000, 100)
    ));

    ASSERT_TRUE(book.remove(1));

    EXPECT_FALSE(book.contains(1));
}

TEST(OrderBookTest, RemovesFilledOrderFromBook)
{
    OrderBook book;

    Order order = make_order(
        1,
        Side::Buy,
        10000,
        100
    );

    ASSERT_TRUE(book.add(order));
    ASSERT_TRUE(book.contains(1));

    ASSERT_TRUE(order.apply_fill(100));
    ASSERT_EQ(order.state(), OrderState::Filled);

    // OrderBook does not observe Order state mutations.
    // The owner must explicitly remove the filled order.
    ASSERT_TRUE(book.remove(1));

    EXPECT_FALSE(book.contains(1));
    EXPECT_EQ(book.size(), 0);
    EXPECT_FALSE(book.best_bid().has_value());
}

TEST(OrderBookTest, KeepsPartiallyFilledOrderInBook)
{
    OrderBook book;

    Order order = make_order(
        1,
        Side::Buy,
        10000,
        100
    );

    ASSERT_TRUE(book.add(order));

    ASSERT_TRUE(order.apply_fill(40));

    ASSERT_EQ(order.state(), OrderState::PartiallyFilled);
    EXPECT_EQ(order.remaining_quantity(), 60);

    EXPECT_TRUE(book.contains(1));
    EXPECT_EQ(book.size(), 1);
    ASSERT_TRUE(book.best_bid().has_value());
    EXPECT_EQ(book.best_bid().value(), 1);
}