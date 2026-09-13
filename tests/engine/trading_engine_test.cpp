#include <gtest/gtest.h>
#include <vector>

#include "engine/trading_engine.hpp"
#include "simulator/market_simulator.hpp"
#include "common/symbol.hpp"
#include "events/replay_source.hpp"

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

TEST(TradingEngineTest, IsolatesMatchingBySymbol)
{
    TradingEngine engine;

    // AAPL resting sell.
    MarketEvent aapl_sell{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Sell,
            .price = 10100,
            .quantity = 100
        }
    };

    // GOOGL resting sell at the same price.
    MarketEvent googl_sell{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = Symbol{"GOOGL"},
        .payload = AddOrderEvent{
            .order_id = 2,
            .side = Side::Sell,
            .price = 10100,
            .quantity = 100
        }
    };

    ASSERT_TRUE(engine.process(aapl_sell));
    ASSERT_TRUE(engine.process(googl_sell));

    // AAPL buy crosses only AAPL sell.
    MarketEvent aapl_buy{
        .event_id = 3,
        .sequence_number = 3,
        .timestamp = Timestamp{1002},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 3,
            .side = Side::Buy,
            .price = 10200,
            .quantity = 40
        }
    };

    ASSERT_TRUE(engine.process(aapl_buy));

    // Exactly one execution: AAPL.
    ASSERT_EQ(engine.execution_recorder().size(), 1);

    const Execution& execution =
        engine.execution_recorder().executions().front();

    EXPECT_EQ(execution.symbol, Symbol{"AAPL"});
    EXPECT_EQ(execution.incoming_order_id, 3);
    EXPECT_EQ(execution.resting_order_id, 1);
    EXPECT_EQ(execution.price, 10100);
    EXPECT_EQ(execution.quantity, 40);

    // GOOGL sell must remain completely untouched.
    const Order* googl_order =
        engine.order_manager().find(2);

    ASSERT_NE(googl_order, nullptr);

    EXPECT_EQ(googl_order->remaining_quantity(), 100);
    EXPECT_EQ(googl_order->state(), OrderState::New);

    // AAPL sell was partially filled.
    const Order* aapl_order =
        engine.order_manager().find(1);

    ASSERT_NE(aapl_order, nullptr);

    EXPECT_EQ(aapl_order->remaining_quantity(), 60);
    EXPECT_EQ(
        aapl_order->state(),
        OrderState::PartiallyFilled
    );
}

TEST(TradingEngineTest, CreatesMarketStateLazilyPerSymbol)
{
    TradingEngine engine;

    EXPECT_EQ(
        engine.market_state_manager().size(),
        0
    );

    MarketEvent aapl_event{
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

    ASSERT_TRUE(engine.process(aapl_event));

    EXPECT_EQ(
        engine.market_state_manager().size(),
        1
    );

    ASSERT_NE(
        engine.market_state_manager().find(
            Symbol{"AAPL"}
        ),
        nullptr
    );

    // Another AAPL event must reuse the existing state.
    MarketEvent aapl_event_2{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = Symbol{"AAPL"},
        .payload = AddOrderEvent{
            .order_id = 101,
            .side = Side::Buy,
            .price = 10000,
            .quantity = 25
        }
    };

    ASSERT_TRUE(engine.process(aapl_event_2));

    EXPECT_EQ(
        engine.market_state_manager().size(),
        1
    );

    // First GOOGL event creates a second state.
    MarketEvent googl_event{
        .event_id = 3,
        .sequence_number = 3,
        .timestamp = Timestamp{1002},
        .symbol = Symbol{"GOOGL"},
        .payload = AddOrderEvent{
            .order_id = 200,
            .side = Side::Sell,
            .price = 20000,
            .quantity = 30
        }
    };

    ASSERT_TRUE(engine.process(googl_event));

    EXPECT_EQ(
        engine.market_state_manager().size(),
        2
    );

    ASSERT_NE(
        engine.market_state_manager().find(
            Symbol{"AAPL"}
        ),
        nullptr
    );

    ASSERT_NE(
        engine.market_state_manager().find(
            Symbol{"GOOGL"}
        ),
        nullptr
    );
}

TEST(TradingEngineTest, CancelsRestingOrder)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    const MarketEvent add_event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Sell,
            .price = 10000,
            .quantity = 10
        }
    };

    ASSERT_TRUE(engine.process(add_event));

    const MarketState* state =
        engine.market_state_manager().find(symbol);

    ASSERT_NE(state, nullptr);
    ASSERT_TRUE(state->order_book().best_ask().has_value());

    const MarketEvent cancel_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = symbol,
        .payload = CancelOrderEvent{
            .order_id = 1,
            .quantity = 10
        }
    };

    EXPECT_TRUE(engine.process(cancel_event));

    const Order* order =
        engine.order_manager().find(1);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->state(), OrderState::CancelPending);

    EXPECT_FALSE(state->order_book().best_ask().has_value());
}


TEST(TradingEngineTest, CancelledOrderDoesNotMatch)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    const MarketEvent add_sell{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Sell,
            .price = 10000,
            .quantity = 10
        }
    };

    ASSERT_TRUE(engine.process(add_sell));

    const MarketEvent cancel_sell{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = symbol,
        .payload = CancelOrderEvent{
            .order_id = 1,
            .quantity = 10
        }
    };

    ASSERT_TRUE(engine.process(cancel_sell));

    const MarketEvent add_buy{
        .event_id = 3,
        .sequence_number = 3,
        .timestamp = Timestamp{1002},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 2,
            .side = Side::Buy,
            .price = 10100,
            .quantity = 10
        }
    };

    ASSERT_TRUE(engine.process(add_buy));

    EXPECT_EQ(engine.execution_recorder().size(), 0);
}


TEST(TradingEngineTest, CancelUnknownOrderFails)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    const MarketEvent cancel_event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = CancelOrderEvent{
            .order_id = 999,
            .quantity = 10
        }
    };

    EXPECT_FALSE(engine.process(cancel_event));
}


TEST(TradingEngineTest, CancelWrongSymbolDoesNotCorruptBook)
{
    TradingEngine engine;

    const Symbol aapl = make_symbol("AAPL");
    const Symbol googl = make_symbol("GOOGL");

    const MarketEvent add_event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = aapl,
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Sell,
            .price = 10000,
            .quantity = 10
        }
    };

    ASSERT_TRUE(engine.process(add_event));

    const MarketEvent cancel_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = googl,
        .payload = CancelOrderEvent{
            .order_id = 1,
            .quantity = 10
        }
    };

    EXPECT_FALSE(engine.process(cancel_event));

    const MarketState* state =
        engine.market_state_manager().find(aapl);

    ASSERT_NE(state, nullptr);

    ASSERT_TRUE(state->order_book().best_ask().has_value());
    EXPECT_EQ(*state->order_book().best_ask(), 1);

    const Order* order =
        engine.order_manager().find(1);

    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->state(), OrderState::New);
}

TEST(TradingEngineTest, CancellationRemovesOrderFromBookAndPreservesQuantity)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    const MarketEvent add_event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Sell,
            .price = 10000,
            .quantity = 100
        }
    };

    ASSERT_TRUE(engine.process(add_event));

    const MarketState* state =
        engine.market_state_manager().find(symbol);

    ASSERT_NE(state, nullptr);
    ASSERT_TRUE(state->order_book().contains(100));

    const MarketEvent cancel_event{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = symbol,
        .payload = CancelOrderEvent{
            .order_id = 100,
            .quantity = 1
        }
    };

    ASSERT_TRUE(engine.process(cancel_event));

    const Order* order =
        engine.order_manager().find(100);

    ASSERT_NE(order, nullptr);

    EXPECT_EQ(order->state(), OrderState::CancelPending);

    // Cancellation does not alter filled/remaining quantities.
    EXPECT_EQ(order->filled_quantity(), 0);
    EXPECT_EQ(order->remaining_quantity(), 100);

    // A cancel request removes the order from the active book.
    EXPECT_FALSE(state->order_book().contains(100));
    EXPECT_EQ(state->order_book().size(), 0);
}

TEST(TradingEngineTest, AcceptsSequentialEvents)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    const MarketEvent event1{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    const MarketEvent event2{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15100,
            .quantity = 50
        }
    };

    EXPECT_TRUE(engine.process(event1));
    EXPECT_TRUE(engine.process(event2));
}


TEST(TradingEngineTest, RejectsOutOfOrderEvent)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    const MarketEvent event1{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    const MarketEvent event3{
        .event_id = 3,
        .sequence_number = 3,
        .timestamp = Timestamp{1002},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15200,
            .quantity = 50
        }
    };

    ASSERT_TRUE(engine.process(event1));
    EXPECT_FALSE(engine.process(event3));
}


TEST(TradingEngineTest, RejectingInvalidEventDoesNotConsumeSequence)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    const MarketEvent invalid_event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{-1},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    const MarketEvent valid_event{
        .event_id = 2,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    EXPECT_FALSE(engine.process(invalid_event));
    EXPECT_TRUE(engine.process(valid_event));
}

class FailingMarketDataSource : public IMarketDataSource
{
public:
    bool next_event(MarketEvent& event) override
    {
        if(index_ == 0){
            event = MarketEvent{
                .event_id = 1,
                .sequence_number = 1,
                .timestamp = Timestamp{1000},
                .symbol = make_symbol("AAPL"),
                .payload = TradeEvent{
                    .price = 15000,
                    .quantity = 100
                }
            };

            ++index_;
            return true;
        }

        if(index_ == 1){
            event = MarketEvent{
                .event_id = 2,
                .sequence_number = 3,
                .timestamp = Timestamp{1002},
                .symbol = make_symbol("AAPL"),
                .payload = TradeEvent{
                    .price = 15000,
                    .quantity = 100
                }
            };

            ++index_;
            return true;
        }

        return false;
    }

    bool next_batch(
        std::vector<MarketEvent>&,
        std::size_t
    ) override
    {
        return false;
    }

private:
    std::size_t index_ = 0;
};

TEST(TradingEngineTest, StopsProcessingWhenAnEventFails)
{
    FailingMarketDataSource source;
    TradingEngine engine;

    EXPECT_EQ(engine.process(source), 1);
}

class SequentialMarketDataSource : public IMarketDataSource
{
public:
    explicit SequentialMarketDataSource(
        SequenceNumber event_count)
        : event_count_(event_count)
    {
    }

    bool next_event(MarketEvent& event) override
    {
        if(next_sequence_ > event_count_){
            return false;
        }

        event = MarketEvent{
            .event_id = next_sequence_,
            .sequence_number = next_sequence_,
            .timestamp = Timestamp{
                static_cast<Timestamp::rep>(next_sequence_ * 1000)
            },
            .symbol = make_symbol("AAPL"),
            .payload = TradeEvent{
                .price = 15000,
                .quantity = 100
            }
        };

        ++next_sequence_;
        return true;
    }

    bool next_batch(
        std::vector<MarketEvent>& batch,
        std::size_t max_events) override
    {
        batch.clear();

        while(batch.size() < max_events){
            MarketEvent event;

            if(!next_event(event)){
                break;
            }

            batch.push_back(event);
        }

        return !batch.empty();
    }

private:
    SequenceNumber event_count_;
    SequenceNumber next_sequence_ = 1;
};

TEST(TradingEngineTest, ProcessesRequestedBatchSize)
{
    SequentialMarketDataSource source{10};
    TradingEngine engine;

    EXPECT_EQ(engine.process_batch(source, 4), 4);
}


TEST(TradingEngineTest, ProcessesRemainingEventsAfterBatch)
{
    SequentialMarketDataSource source{5};
    TradingEngine engine;

    EXPECT_EQ(engine.process_batch(source, 3), 3);
    EXPECT_EQ(engine.process_batch(source, 3), 2);
}


TEST(TradingEngineTest, ZeroBatchSizeProcessesNothing)
{
    SequentialMarketDataSource source{5};
    TradingEngine engine;

    EXPECT_EQ(engine.process_batch(source, 0), 0);
}


TEST(TradingEngineTest, EmptySourceProcessesNothing)
{
    SequentialMarketDataSource source{0};
    TradingEngine engine;

    EXPECT_EQ(engine.process_batch(source, 10), 0);
}

TEST(TradingEngineTest, ProcessesEntireMarketDataSource)
{
    SequentialMarketDataSource source{3};
    TradingEngine engine;

    EXPECT_EQ(engine.process(source), 3);
}

class BatchWithGapSource : public IMarketDataSource
{
public:
    bool next_event(MarketEvent& event) override
    {
        if(index_ >= 3){
            return false;
        }

        const SequenceNumber sequence =
            index_ == 1 ? 3 : index_ + 1;

        event = MarketEvent{
            .event_id = index_ + 1,
            .sequence_number = sequence,
            .timestamp = Timestamp{
                static_cast<Timestamp::rep>((index_ + 1) * 1000)
            },
            .symbol = make_symbol("AAPL"),
            .payload = TradeEvent{
                .price = 15000,
                .quantity = 100
            }
        };

        ++index_;
        return true;
    }

    bool next_batch(
        std::vector<MarketEvent>& batch,
        std::size_t max_events) override
    {
        batch.clear();

        while(batch.size() < max_events){
            MarketEvent event;

            if(!next_event(event)){
                break;
            }

            batch.push_back(event);
        }

        return !batch.empty();
    }

private:
    std::size_t index_ = 0;
};

TEST(TradingEngineTest, BatchProcessingStopsAtFirstRejectedEvent)
{
    BatchWithGapSource source;
    TradingEngine engine;

    EXPECT_EQ(engine.process_batch(source, 3), 1);
}

TEST(TradingEngineTest, SemanticFailureConsumesSequence)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    MarketEvent first{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 100
        }
    };

    MarketEvent duplicate{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 100
        }
    };

    MarketEvent third{
        .event_id = 3,
        .sequence_number = 3,
        .timestamp = Timestamp{1002},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    ASSERT_TRUE(engine.process(first));

    EXPECT_FALSE(engine.process(duplicate));

    EXPECT_TRUE(engine.process(third));
}

TEST(TradingEngineTest, LogsAcceptedEvent)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    ASSERT_TRUE(engine.process(event));

    ASSERT_EQ(engine.event_log().size(), 1);
    EXPECT_EQ(engine.event_log().at(0).event_id, 1);
    EXPECT_EQ(engine.event_log().at(0).sequence_number, 1);
}

TEST(TradingEngineTest, LogsSemanticFailure)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    MarketEvent first{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 100
        }
    };

    MarketEvent duplicate{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 100,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 100
        }
    };

    ASSERT_TRUE(engine.process(first));
    EXPECT_FALSE(engine.process(duplicate));

    ASSERT_EQ(engine.event_log().size(), 2);
    EXPECT_EQ(engine.event_log().at(0).sequence_number, 1);
    EXPECT_EQ(engine.event_log().at(1).sequence_number, 2);
}

TEST(TradingEngineTest, DoesNotLogStructurallyInvalidEvent)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    MarketEvent invalid{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 0,
            .quantity = 100
        }
    };

    EXPECT_FALSE(engine.process(invalid));
    EXPECT_EQ(engine.event_log().size(), 0);
}

TEST(TradingEngineTest, DoesNotLogSequenceRejectedEvent)
{
    TradingEngine engine;

    const Symbol symbol = make_symbol("AAPL");

    MarketEvent first{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    MarketEvent gap{
        .event_id = 2,
        .sequence_number = 3,
        .timestamp = Timestamp{1001},
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 15100,
            .quantity = 100
        }
    };

    ASSERT_TRUE(engine.process(first));
    EXPECT_FALSE(engine.process(gap));

    ASSERT_EQ(engine.event_log().size(), 1);
    EXPECT_EQ(engine.event_log().at(0).sequence_number, 1);
}

TEST(TradingEngineTest, DeterministicReplayReconstructsMarketState)
{
    const Symbol symbol = make_symbol("AAPL");

    TradingEngine original;

    const MarketEvent buy_100{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 1,
            .side = Side::Buy,
            .price = 15000,
            .quantity = 100
        }
    };

    const MarketEvent buy_50{
        .event_id = 2,
        .sequence_number = 2,
        .timestamp = Timestamp{1001},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 2,
            .side = Side::Buy,
            .price = 14900,
            .quantity = 50
        }
    };

    const MarketEvent sell_120{
        .event_id = 3,
        .sequence_number = 3,
        .timestamp = Timestamp{1002},
        .symbol = symbol,
        .payload = AddOrderEvent{
            .order_id = 3,
            .side = Side::Sell,
            .price = 15000,
            .quantity = 120
        }
    };

    ASSERT_TRUE(original.process(buy_100));
    ASSERT_TRUE(original.process(buy_50));
    ASSERT_TRUE(original.process(sell_120));

    ASSERT_EQ(original.event_log().size(), 3);
    ASSERT_EQ(original.execution_recorder().size(), 1);

    const Order* original_order_1 =
        original.order_manager().find(1);

    const Order* original_order_2 =
        original.order_manager().find(2);

    const Order* original_order_3 =
        original.order_manager().find(3);

    ASSERT_NE(original_order_1, nullptr);
    ASSERT_NE(original_order_2, nullptr);
    ASSERT_NE(original_order_3, nullptr);

    EXPECT_EQ(original_order_1->state(), OrderState::Filled);
    EXPECT_EQ(original_order_1->filled_quantity(), 100);
    EXPECT_EQ(original_order_1->remaining_quantity(), 0);

    EXPECT_EQ(original_order_2->state(), OrderState::New);
    EXPECT_EQ(original_order_2->filled_quantity(), 0);
    EXPECT_EQ(original_order_2->remaining_quantity(), 50);

    EXPECT_EQ(original_order_3->state(), OrderState::PartiallyFilled);
    EXPECT_EQ(original_order_3->filled_quantity(), 100);
    EXPECT_EQ(original_order_3->remaining_quantity(), 20);

    const MarketState* original_state =
        original.market_state_manager().find(symbol);

    ASSERT_NE(original_state, nullptr);

    EXPECT_EQ(original_state->order_book().size(), 2);
    EXPECT_TRUE(original_state->order_book().contains(2));
    EXPECT_TRUE(original_state->order_book().contains(3));

    const auto original_best_bid =
        original_state->order_book().best_bid();

    const auto original_best_ask =
        original_state->order_book().best_ask();

    ASSERT_TRUE(original_best_bid.has_value());
    ASSERT_TRUE(original_best_ask.has_value());

    EXPECT_EQ(*original_best_bid, 2);
    EXPECT_EQ(*original_best_ask, 3);

    TradingEngine replayed;

    ReplaySource source(original.event_log());

    EXPECT_EQ(replayed.process(source), 3);

    EXPECT_EQ(replayed.event_log().size(), 3);
    EXPECT_EQ(replayed.execution_recorder().size(), 1);

    const Order* replay_order_1 =
        replayed.order_manager().find(1);

    const Order* replay_order_2 =
        replayed.order_manager().find(2);

    const Order* replay_order_3 =
        replayed.order_manager().find(3);

    ASSERT_NE(replay_order_1, nullptr);
    ASSERT_NE(replay_order_2, nullptr);
    ASSERT_NE(replay_order_3, nullptr);

    EXPECT_EQ(replay_order_1->state(), original_order_1->state());
    EXPECT_EQ(replay_order_1->filled_quantity(),
              original_order_1->filled_quantity());
    EXPECT_EQ(replay_order_1->remaining_quantity(),
              original_order_1->remaining_quantity());

    EXPECT_EQ(replay_order_2->state(), original_order_2->state());
    EXPECT_EQ(replay_order_2->filled_quantity(),
              original_order_2->filled_quantity());
    EXPECT_EQ(replay_order_2->remaining_quantity(),
              original_order_2->remaining_quantity());

    EXPECT_EQ(replay_order_3->state(), original_order_3->state());
    EXPECT_EQ(replay_order_3->filled_quantity(),
              original_order_3->filled_quantity());
    EXPECT_EQ(replay_order_3->remaining_quantity(),
              original_order_3->remaining_quantity());

    const MarketState* replayed_state =
        replayed.market_state_manager().find(symbol);

    ASSERT_NE(replayed_state, nullptr);

    EXPECT_EQ(
        replayed_state->order_book().size(),
        original_state->order_book().size()
    );

    EXPECT_TRUE(replayed_state->order_book().contains(2));
    EXPECT_TRUE(replayed_state->order_book().contains(3));

    const auto replay_best_bid =
        replayed_state->order_book().best_bid();

    const auto replay_best_ask =
        replayed_state->order_book().best_ask();

    ASSERT_TRUE(replay_best_bid.has_value());
    ASSERT_TRUE(replay_best_ask.has_value());

    EXPECT_EQ(*replay_best_bid, *original_best_bid);
    EXPECT_EQ(*replay_best_ask, *original_best_ask);

    const auto& original_executions =
        original.execution_recorder().executions();

    const auto& replayed_executions =
        replayed.execution_recorder().executions();

    ASSERT_EQ(replayed_executions.size(), original_executions.size());

    for (std::size_t i = 0; i < original_executions.size(); ++i) {
        EXPECT_EQ(
            replayed_executions[i].execution_id,
            original_executions[i].execution_id
        );

        EXPECT_EQ(
            replayed_executions[i].event_id,
            original_executions[i].event_id
        );

        EXPECT_EQ(
            replayed_executions[i].timestamp,
            original_executions[i].timestamp
        );

        EXPECT_EQ(
            replayed_executions[i].symbol,
            original_executions[i].symbol
        );

        EXPECT_EQ(
            replayed_executions[i].incoming_order_id,
            original_executions[i].incoming_order_id
        );

        EXPECT_EQ(
            replayed_executions[i].resting_order_id,
            original_executions[i].resting_order_id
        );

        EXPECT_EQ(
            replayed_executions[i].price,
            original_executions[i].price
        );

        EXPECT_EQ(
            replayed_executions[i].quantity,
            original_executions[i].quantity
        );
    }
}

