#pragma once

#include "market_types.hpp"
#include "order.hpp"

struct TradeEvent {
    Price price;
    Quantity quantity;

    bool operator==(const TradeEvent&) const = default;
};

struct QuoteEvent {
    Price bid_price;
    Quantity bid_quantity;
    Price asking_price;
    Quantity asking_quantity;

    bool operator==(const QuoteEvent&) const = default;
};

struct AddOrderEvent {
    OrderId order_id;
    Side side;
    Price price;
    Quantity quantity;

    bool operator==(const AddOrderEvent&) const = default;
};

struct CancelOrderEvent {
    OrderId order_id;
    Quantity quantity;

    bool operator==(const CancelOrderEvent&) const = default;
};

struct ExecuteOrderEvent {
    OrderId order_id;
    Price price;
    Quantity quantity;

    bool operator==(const ExecuteOrderEvent&) const = default;
};