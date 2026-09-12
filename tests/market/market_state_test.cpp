#include <gtest/gtest.h>

#include "market/market_state.hpp"

TEST(MarketStateTest, StartsWithEmptyOrderBook)
{
    OrderManager order_manager;
    MarketState state(order_manager);

    EXPECT_EQ(state.order_book().best_bid(), std::nullopt);
    EXPECT_EQ(state.order_book().best_ask(), std::nullopt);
}