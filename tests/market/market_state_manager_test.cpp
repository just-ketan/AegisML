#include <gtest/gtest.h>

#include "market/market_state_manager.hpp"

TEST(MarketStateManagerTest, StartsEmpty)
{
    OrderManager order_manager;
    MarketStateManager manager(order_manager);

    EXPECT_EQ(manager.size(), 0);
    EXPECT_EQ(
        manager.find(Symbol{"AAPL"}),
        nullptr
    );
}

TEST(MarketStateManagerTest, CreatesStateForSymbol)
{
    OrderManager order_manager;
    MarketStateManager manager(order_manager);

    MarketState& state =
        manager.get_or_create(Symbol{"AAPL"});

    EXPECT_EQ(manager.size(), 1);

    const MarketState* found =
        manager.find(Symbol{"AAPL"});

    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found, &state);
}

TEST(MarketStateManagerTest, ReturnsExistingState)
{
    OrderManager order_manager;
    MarketStateManager manager(order_manager);

    MarketState& first =
        manager.get_or_create(Symbol{"AAPL"});

    MarketState& second =
        manager.get_or_create(Symbol{"AAPL"});

    EXPECT_EQ(manager.size(), 1);
    EXPECT_EQ(&first, &second);
}

TEST(MarketStateManagerTest, MaintainsIndependentStatePerSymbol)
{
    OrderManager order_manager;
    MarketStateManager manager(order_manager);

    MarketState& aapl =
        manager.get_or_create(Symbol{"AAPL"});

    MarketState& googl =
        manager.get_or_create(Symbol{"GOOGL"});

    ASSERT_NE(&aapl, &googl);
    EXPECT_EQ(manager.size(), 2);

    Order aapl_order{
        1,
        Side::Sell,
        10100,
        100
    };

    Order googl_order{
        2,
        Side::Buy,
        10200,
        40
    };

    ASSERT_TRUE(aapl.order_book().add(aapl_order));
    ASSERT_TRUE(googl.order_book().add(googl_order));

    ASSERT_TRUE(aapl.order_book().best_ask().has_value());
    ASSERT_TRUE(googl.order_book().best_bid().has_value());

    EXPECT_EQ(
        aapl.order_book().best_ask().value(),
        10100
    );

    EXPECT_EQ(
        googl.order_book().best_bid().value(),
        10200
    );

    EXPECT_FALSE(
        aapl.order_book().best_bid().has_value()
    );

    EXPECT_FALSE(
        googl.order_book().best_ask().has_value()
    );
}