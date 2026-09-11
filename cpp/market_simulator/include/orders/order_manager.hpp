#pragma once

#include<cstddef>
#include<unordered_map>

#include "events/market_event.hpp"
#include "orders/order.hpp"

class OrderManager {
public:
    bool process(const MarketEvent& event);
    bool execute(OrderId order_id, Quantity qtty);
    
    const Order* find(OrderId id) const;
    std::size_t size() const;
    
    bool confirm_cancel(OrderId order_id);

private:
    std::unordered_map<OrderId, Order> orders_;
};