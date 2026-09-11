#include "orders/order_book.hpp"
#include <iterator>

bool OrderBook::add(const Order& order){
    // id must be unique within the book
    if(order_lookup_.find(order.id())!=order_lookup_.end()){ return false;   }
    if(order.remaining_quantity()<=0){  return false;   }
    if(order.state()!= OrderState::New && order.state()!=OrderState::PartiallyFilled){  return false;   }

    // if buying
    if(order.side() == Side::Buy){
        auto& queue = bids_[order.price()];
        queue.push_back(order.id());
        auto position = std::prev(queue.end());

        order_lookup_.emplace(
            order.id(), 
            OrderLocation{
                Side::Buy,
                order.price(),
                position
            }
        );
        return true;
    }

    // if selling
    if(order.side() == Side::Sell){
        auto& queue = asks_[order.price()];
        queue.push_back(order.id());
        auto position = std::prev(queue.end());

        order_lookup_.emplace(
            order.id(),
            OrderLocation{
                Side::Sell,
                order.price(),
                position
            }
        );
        return true;
    }
    return false;
}

std::size_t OrderBook::size() const {
    return order_lookup_.size();
}

bool OrderBook::remove(OrderId order_id){
    auto lookup_it = order_lookup_.find(order_id);
    if(lookup_it == order_lookup_.end()){    return false;   }
    
    const OrderLocation& location = lookup_it->second;
    if(location.side == Side::Buy){
        auto level_it = bids_.find(location.price);
        if(level_it == bids_.end()){    return false;   }
        
        auto& queue = level_it->second;
        queue.erase(location.position);

        if(queue.empty()){  bids_.erase(level_it);  }   // remove the iterator is list empty
    }else if(location.side == Side::Sell){
        auto level_it = asks_.find(location.price);
        if(level_it == asks_.end()){    return false;   }

        auto& queue = level_it->second;
        queue.erase(location.position);

        if(queue.empty()){  asks_.erase(level_it);  }
    }else{
        return false;
    }
    // need to remove entry from lookup table as well
    order_lookup_.erase(lookup_it);
    return true;
}

// best_bid has bids_ that is mpa<> with greater<> so it will always pick the largest entry based on Price
std::optional<OrderId> OrderBook::best_bid() const {
    if(bids_.empty()){  return std::nullopt;    }
    const auto& queue = bids_.begin()->second;  // bids_ is an iterator and second is object
    if(queue.empty()){  return std::nullopt;    }

    return queue.front();   // front always largest
}

// best_ask has asks_ that has map<> with lowest Price as priority
std::optional<OrderId> OrderBook::best_ask() const {
    if(asks_.empty()){  return std::nullopt;    }
    auto& queue = asks_.begin()->second;
    if(queue.empty()){  return std::nullopt; }
    return queue.front();
}

bool OrderBook::contains(OrderId order_id) const {
    return order_lookup_.find(order_id) != order_lookup_.end(); // this is O(1) average operation
}