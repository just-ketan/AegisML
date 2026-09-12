#pragma once

#include "orders/matching_engine.hpp"
#include "orders/order_book.hpp"

class MarketState{
    public:
        explicit MarketState(OrderManager& order_manager);
        
        OrderBook& order_book();
        const OrderBook& order_book() const;

        MatchingEngine& matching_engine();
        const MatchingEngine& matching_engine() const;
    
    private:
        OrderBook order_book_;
        MatchingEngine matching_engine_;
};