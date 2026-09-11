#pragma once

#include <cstddef>
#include <optional>

#include "orders/order_book.hpp"
#include "orders/order_manager.hpp"

struct Trade {
    OrderId incoming_order_id;
    OrderId resting_order_id;
    Price price;
    Quantity quantity;
};

class MatchingEngine{
    public:
    // the engine doesnt own both systems, this is DEPENDENCY INJECTION w/o unnecessary abstraction.
        MatchingEngine(OrderManager& order_manager, OrderBook& order_book);
        std::optional<Trade> match(OrderId incoming_order_id);

    private:
        OrderManager& order_manager_;
        OrderBook& order_book_;

        std::optional<Trade> execute_match(OrderId incoming_order_id, OrderId resting_order_id);
        
        // helper to see if resting is needed
        bool rest_if_needed(OrderId order_id);
};