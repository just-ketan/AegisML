#pragma once

#include<cstddef>
#include<unordered_map>

#include "market_event.hpp"
#include "order.hpp"

class OrderManager {
public:
    bool process(const MarketEvent& event);
    const Order* find(OrderId id) const;
    std::size_t size() const;
private:
    std::unordered_map<OrderId, Order> orders_;
};