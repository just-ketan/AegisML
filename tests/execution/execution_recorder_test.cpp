#include <gtest/gtest.h>

#include "execution/execution_recorder.hpp"

TEST(ExecutionRecorderTest, StartsEmpty)
{
    ExecutionRecorder recorder;

    EXPECT_EQ(recorder.size(), 0);
    EXPECT_TRUE(recorder.executions().empty());
}

TEST(ExecutionRecorderTest, RecordsTrade)
{
    ExecutionRecorder recorder;

    Trade trade{
        .incoming_order_id = 100,
        .resting_order_id = 50,
        .price = 10100,
        .quantity = 25
    };
    
    ExecutionContext context{
        .event_id = 500,
        .timestamp = Timestamp{12345},
        .symbol = Symbol{"AAPL"}
    };

    recorder.record(trade, context);

    ASSERT_EQ(recorder.size(), 1);

    const Execution& execution =
        recorder.executions().front();

    EXPECT_EQ(execution.execution_id, 1);
    EXPECT_EQ(execution.incoming_order_id, 100);
    EXPECT_EQ(execution.resting_order_id, 50);
    EXPECT_EQ(execution.price, 10100);
    EXPECT_EQ(execution.quantity, 25);
    EXPECT_EQ(execution.event_id, 500);
    EXPECT_EQ(execution.timestamp, Timestamp{12345});
    EXPECT_EQ(execution.symbol, Symbol{"AAPL"});
}

TEST(ExecutionRecorderTest, AssignsSequentialExecutionIds)
{
    ExecutionRecorder recorder;

    ExecutionContext context{
        .event_id = 500,
        .timestamp = Timestamp{12345},
        .symbol = Symbol{"AAPL"}
    };

    recorder.record(
        Trade{
            .incoming_order_id = 100,
            .resting_order_id = 50,
            .price = 10100,
            .quantity = 10
        },
        context
    );

    recorder.record(
        Trade{
            .incoming_order_id = 101,
            .resting_order_id = 51,
            .price = 10200,
            .quantity = 20
        },
        context
    );

    ASSERT_EQ(recorder.size(), 2);

    EXPECT_EQ(recorder.executions()[0].execution_id, 1);
    EXPECT_EQ(recorder.executions()[1].execution_id, 2);
}

TEST(ExecutionRecorderTest, FindsExecutionById)
{
    ExecutionRecorder recorder;

    recorder.record(Trade{
        .incoming_order_id = 100,
        .resting_order_id = 50,
        .price = 10100,
        .quantity = 10
    }, ExecutionContext{
        .event_id = 500,
        .timestamp = Timestamp{12345},
        .symbol = Symbol{"GOOGL"}
    });

    const Execution* execution =
        recorder.find(1);

    ASSERT_NE(execution, nullptr);
    EXPECT_EQ(execution->incoming_order_id, 100);
}

TEST(ExecutionRecorderTest, UnknownExecutionReturnsNull)
{
    ExecutionRecorder recorder;

    EXPECT_EQ(recorder.find(999), nullptr);
}