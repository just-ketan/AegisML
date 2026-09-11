#pragma once

#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <unordered_map>
#include <optional>
#include "orders/order.hpp"

/*
class OrderBook{
    public:
        virtual ~OrderBook() = default;
        virtual bool add(const Order& order) = 0;
        virtual bool remove(OrderId order_id) = 0;
        virtual const Order* best_bid() const = 0;  // gets best bid in O(nlogn) time
        virtual const Order* best_ask() const = 0;  // gets best ask in O(nlgn) time
        virtual std::size_t size() const = 0;
};

we defining behaviors not data structure
the pure virtual functions make this class abstract
the implementations will make concrete nature
but the thing is, we dont necessarily need RUNTIME POLYMORPHISM to add up to latency
for now we are not planning for multiple orderbooks so concrete implementation should be better
as we are considering low latency systems
*/

class OrderBook{
    public:
        bool add(const Order& order);
        bool remove(OrderId order_id);
        // OrderBook reveive a lookup function from OrderManager cause we dont own Order object, thus no Order* can be returned
        // but giving access to Orders is not good here, so we have optional order id
        std::optional<OrderId> best_bid() const;
        std::optional<OrderId> best_ask() const;
        std::size_t size() const;
        bool contains(OrderId order_id) const;

    private:
        // lets define the data structures for our requirements
        using OrderQueue = std::list<OrderId>;
        using BidLevels = std::map<Price, OrderQueue, std::greater<Price>>;
        using AskLevels = std::map<Price, OrderQueue>;
        
        struct OrderLocation{
            Side side;
            Price price;
            OrderQueue::iterator position;
        };

        BidLevels bids_;
        AskLevels asks_;
        std::unordered_map<OrderId, OrderLocation> order_lookup_;
};