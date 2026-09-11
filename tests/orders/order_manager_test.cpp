#include <gtest/gtest.h>

#include "order_manager.hpp"
#include "symbol.hpp"

namespace {

MarketEvent make_add_event(
    EventId event_id,
    OrderId order_id,
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
            .side = Side::Buy,
            .price = price,
            .quantity = quantity
        }
    };
}

MarketEvent make_execute_event(
    EventId event_id,
    OrderId order_id,
    Price price,
    Quantity quantity
) {
    return MarketEvent{
        .event_id = event_id,
        .sequence_number = event_id,
        .timestamp = Timestamp{static_cast<Timestamp::rep>(event_id)},
        .symbol = make_symbol("AAPL"),
        .payload = ExecuteOrderEvent{
            .order_id = order_id,
            .price = price,
            .quantity = quantity
        }
    };
}

}

TEST(OrderManagerTest, AddCreatesOrder)
{
    OrderManager manager;

    const auto event = make_add_event(
        1,
        100,
        1'500'000,
        500
    );

    EXPECT_TRUE(manager.process(event));
    EXPECT_EQ(manager.size(), 1);

    const Order* order = manager.find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->id(), 100);
    EXPECT_EQ(order->side(), Side::Buy);
    EXPECT_EQ(order->price(), 1'500'000);
    EXPECT_EQ(order->quantity(), 500);
    EXPECT_EQ(order->filled_quantity(), 0);
    EXPECT_EQ(order->remaining_quantity(), 500);
    EXPECT_EQ(order->state(), OrderState::New);
}

TEST(OrderManagerTest, ExecutePartiallyFillsOrder)
{
    OrderManager manager;

    EXPECT_TRUE(
        manager.process(
            make_add_event(1, 100, 1'500'000, 500)
        )
    );

    EXPECT_TRUE(
        manager.process(
            make_execute_event(2, 100, 1'500'000, 200)
        )
    );

    const Order* order = manager.find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->filled_quantity(), 200);
    EXPECT_EQ(order->remaining_quantity(), 300);
    EXPECT_EQ(order->state(), OrderState::PartiallyFilled);
}

TEST(OrderManagerTest, ExecuteFullyFillsOrder)
{
    OrderManager manager;

    EXPECT_TRUE(
        manager.process(
            make_add_event(1, 100, 1'500'000, 500)
        )
    );

    EXPECT_TRUE(
        manager.process(
            make_execute_event(2, 100, 1'500'000, 500)
        )
    );

    const Order* order = manager.find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->filled_quantity(), 500);
    EXPECT_EQ(order->remaining_quantity(), 0);
    EXPECT_EQ(order->state(), OrderState::Filled);
}

TEST(OrderManagerTest, ExecuteUnknownOrderFails)
{
    OrderManager manager;

    const auto event = make_execute_event(
        1,
        999,
        1'500'000,
        100
    );

    EXPECT_FALSE(manager.process(event));
    EXPECT_EQ(manager.size(), 0);
}