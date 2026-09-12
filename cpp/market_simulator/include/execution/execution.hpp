#pragma once

#include "common/market_types.hpp"
#include "orders/order.hpp"
#include "common/symbol.hpp"

using ExecutionId = std::uint64_t;

struct Execution{
    ExecutionId execution_id;
    EventId event_id;
    Timestamp timestamp;
    Symbol symbol;

    OrderId incoming_order_id;
    OrderId resting_order_id;

    Price price;
    Quantity quantity;

    bool operator==(const Execution&) const = default;
};