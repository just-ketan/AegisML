#include <gtest/gtest.h>

#include "engine/trading_engine.hpp"

TEST(TradingEngineTest, StartsEmpty)
{
    TradingEngine engine;

    EXPECT_EQ(engine.order_manager().size(), 0);
    EXPECT_EQ(engine.execution_recorder().size(), 0);
}

TEST(TradingEngineTest, ProcessesAddOrder)
{
    TradingEngine engine;

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 10100,
            .quantity = 50
        }
    };

    EXPECT_TRUE(engine.process(event));

    EXPECT_EQ(engine.order_manager().size(), 1);

    const Order* order =
        engine.order_manager().find(100);

    ASSERT_NE(order, nullptr);

    EXPECT_EQ(order->id(), 100);
    EXPECT_EQ(order->side(), Side::Buy);
    EXPECT_EQ(order->price(), 10100);
    EXPECT_EQ(order->quantity(), 50);
    EXPECT_EQ(order->remaining_quantity(), 50);
    EXPECT_EQ(order->state(), OrderState::New);
}

TEST(TradingEngineTest, MatchesOrdersAndRecordsExecution)
{
    TradingEngine engine;

    MarketEvent sell_event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 50,
            .side = Side::Sell,
            .price = 10100,
            .quantity = 100
        }
    };

    MarketEvent buy_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 10200,
            .quantity = 40
        }
    };

    EXPECT_TRUE(engine.process(sell_event));
    EXPECT_TRUE(engine.process(buy_event));

    EXPECT_EQ(engine.execution_recorder().size(), 1);

    const Execution& execution =
        engine.execution_recorder().executions().front();

    EXPECT_EQ(execution.execution_id, 1);
    EXPECT_EQ(execution.event_id, 2);
    EXPECT_EQ(execution.timestamp, Timestamp{1001});
    EXPECT_EQ(execution.symbol, Symbol{"AAPL"});

    EXPECT_EQ(execution.incoming_order_id, 100);
    EXPECT_EQ(execution.resting_order_id, 50);
    EXPECT_EQ(execution.price, 10100);
    EXPECT_EQ(execution.quantity, 40);
}


TEST(TradingEngineTest, MatchesAcrossMultiplePriceLevels)
{
    TradingEngine engine;

    MarketEvent sell_1{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 50,
            .side = Side::Sell,
            .price = 10000,
            .quantity = 30
        }
    };

    MarketEvent sell_2{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 51,
            .side = Side::Sell,
            .price = 10100,
            .quantity = 40
        }
    };

    MarketEvent buy{
        .event_id = 3,
        .sequence_number = 3,
        .timestamp = Timestamp{1002},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 10100,
            .quantity = 50
        }
    };

    EXPECT_TRUE(engine.process(sell_1));
    EXPECT_TRUE(engine.process(sell_2));
    EXPECT_TRUE(engine.process(buy));

    ASSERT_EQ(engine.execution_recorder().size(), 2);

    const auto& executions =
        engine.execution_recorder().executions();

    EXPECT_EQ(executions[0].execution_id, 1);
    EXPECT_EQ(executions[0].incoming_order_id, 100);
    EXPECT_EQ(executions[0].resting_order_id, 50);
    EXPECT_EQ(executions[0].price, 10000);
    EXPECT_EQ(executions[0].quantity, 30);

    EXPECT_EQ(executions[1].execution_id, 2);
    EXPECT_EQ(executions[1].incoming_order_id, 100);
    EXPECT_EQ(executions[1].resting_order_id, 51);
    EXPECT_EQ(executions[1].price, 10100);
    EXPECT_EQ(executions[1].quantity, 20);

    const Order* incoming =
        engine.order_manager().find(100);

    const Order* first_resting =
        engine.order_manager().find(50);

    const Order* second_resting =
        engine.order_manager().find(51);

    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(first_resting, nullptr);
    ASSERT_NE(second_resting, nullptr);

    EXPECT_EQ(incoming->remaining_quantity(), 0);
    EXPECT_EQ(incoming->state(), OrderState::Filled);

    EXPECT_EQ(first_resting->remaining_quantity(), 0);
    EXPECT_EQ(first_resting->state(), OrderState::Filled);

    EXPECT_EQ(second_resting->remaining_quantity(), 20);
    EXPECT_EQ(
        second_resting->state(),
        OrderState::PartiallyFilled
    );
}

TEST(TradingEngineTest, RoutesCancelOrder)
{
    TradingEngine engine;

    MarketEvent add_event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 10100,
            .quantity = 50
        }
    };

    ASSERT_TRUE(engine.process(add_event));

    const Order* order =
        engine.order_manager().find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->state(), OrderState::New);

    MarketEvent cancel_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = Symbol{"AAPL"},
        .payload = CancelOrderEvent{
            .order_id = 100,
            .quantity = 50
        }
    };

    EXPECT_TRUE(engine.process(cancel_event));

    order = engine.order_manager().find(100);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->state(), OrderState::CancelPending);
}

TEST(TradingEngineTest, ProcessesCompleteOrderLifecycle)
{
    TradingEngine engine;

    // 1. Resting sell order.
    MarketEvent sell_event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 50,
            .side = Side::Sell,
            .price = 10100,
            .quantity = 100
        }
    };

    ASSERT_TRUE(engine.process(sell_event));

    // 2. Incoming buy partially fills the sell.
    MarketEvent buy_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 10200,
            .quantity = 40
        }
    };

    ASSERT_TRUE(engine.process(buy_event));

    const Order* sell =
        engine.order_manager().find(50);

    const Order* buy =
        engine.order_manager().find(100);

    ASSERT_NE(sell, nullptr);
    ASSERT_NE(buy, nullptr);

    EXPECT_EQ(sell->remaining_quantity(), 60);
    EXPECT_EQ(sell->state(), OrderState::PartiallyFilled);

    EXPECT_EQ(buy->remaining_quantity(), 0);
    EXPECT_EQ(buy->state(), OrderState::Filled);

    // 3. Execution was recorded.
    ASSERT_EQ(engine.execution_recorder().size(), 1);

    const Execution& execution =
        engine.execution_recorder().executions().front();

    EXPECT_EQ(execution.incoming_order_id, 100);
    EXPECT_EQ(execution.resting_order_id, 50);
    EXPECT_EQ(execution.price, 10100);
    EXPECT_EQ(execution.quantity, 40);
    EXPECT_EQ(execution.event_id, 2);
    EXPECT_EQ(execution.timestamp, Timestamp{1001});
    EXPECT_EQ(execution.symbol, Symbol{"AAPL"});

    // 4. Cancel the remaining resting quantity.
    MarketEvent cancel_event{
        .event_id = 3,
        .sequence_number = 3,
        .timestamp = Timestamp{1002},
        .symbol = Symbol{"AAPL"},
        .payload = CancelOrderEvent{
            .order_id = 50,
            .quantity = 60
        }
    };

    ASSERT_TRUE(engine.process(cancel_event));

    sell = engine.order_manager().find(50);

    ASSERT_NE(sell, nullptr);
    EXPECT_EQ(sell->state(), OrderState::CancelPending);
}