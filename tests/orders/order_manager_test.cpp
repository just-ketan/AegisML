#include <gtest/gtest.h>

#include "orders/order_manager.hpp"
#include "common/symbol.hpp"

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

TEST(OrderManagerTest, CancelMovesOrderToCancelPending)
{
    OrderManager manager;

    EXPECT_TRUE(
        manager.process(
            make_add_event(1, 100, 1'500'000, 500)
        )
    );

    MarketEvent cancel_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{2},
        .symbol = make_symbol("AAPL"),
        .payload = CancelOrderEvent{
            .order_id = 100,
            .quantity = 500
        }
    };

    EXPECT_TRUE(manager.process(cancel_event));

    const Order* order = manager.find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->state(), OrderState::CancelPending);
}

TEST(OrderManagerTest, ConfirmCancelMovesOrderToCancelled)
{
    OrderManager manager;

    EXPECT_TRUE(
        manager.process(
            make_add_event(1, 100, 1'500'000, 500)
        )
    );

    MarketEvent cancel_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{2},
        .symbol = make_symbol("AAPL"),
        .payload = CancelOrderEvent{
            .order_id = 100,
            .quantity = 500
        }
    };

    EXPECT_TRUE(manager.process(cancel_event));
    EXPECT_TRUE(manager.confirm_cancel(100));

    const Order* order = manager.find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->state(), OrderState::Cancelled);
}

TEST(OrderManagerTest, CannotConfirmCancelBeforeRequest)
{
    OrderManager manager;

    EXPECT_TRUE(
        manager.process(
            make_add_event(1, 100, 1'500'000, 500)
        )
    );

    EXPECT_FALSE(manager.confirm_cancel(100));

    const Order* order = manager.find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->state(), OrderState::New);
}


TEST(OrderManagerTest, CancelledOrderCannotBeExecuted)
{
    OrderManager manager;

    EXPECT_TRUE(
        manager.process(
            make_add_event(1, 100, 1'500'000, 500)
        )
    );

    MarketEvent cancel_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{2},
        .symbol = make_symbol("AAPL"),
        .payload = CancelOrderEvent{
            .order_id = 100,
            .quantity = 500
        }
    };

    EXPECT_TRUE(manager.process(cancel_event));
    EXPECT_TRUE(manager.confirm_cancel(100));

    EXPECT_FALSE(
        manager.process(
            make_execute_event(3, 100, 1'500'000, 100)
        )
    );

    const Order* order = manager.find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->filled_quantity(), 0);
    EXPECT_EQ(order->state(), OrderState::Cancelled);
}