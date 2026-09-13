#pragma once

#include "events/market_event.hpp"
#include "execution/execution_recorder.hpp"
#include "orders/matching_engine.hpp"
#include "orders/order_book.hpp"
#include "orders/order_manager.hpp"
#include "market/market_state_manager.hpp"

class TradingEngine{
    public:
        TradingEngine();

        bool process(const MarketEvent& event);
        const OrderManager& order_manager() const;
        const MarketStateManager& market_state_manager() const;
        const ExecutionRecorder& execution_recorder() const;

    private:
        OrderManager order_manager_;
        MarketStateManager market_state_manager_;
        ExecutionRecorder execution_recorder_;
};
/*
the ownership is now exclusive
TradingEngine owns:
├── OrderManager
├── OrderBook
├── MatchingEngine
└── ExecutionRecorder
and MatchinEngine needs reference to OrderManager and OrderBook

*/