#include "orders/matching_engine.hpp"

MatchingEngine::MatchingEngine(OrderManager& order_mgr, OrderBook& order_bk):order_manager_(order_mgr), order_book_(order_bk) {}
// does this order still have qtty7, and if so, should it be placed into the book?
bool MatchingEngine::rest_if_needed(OrderId order_id){
    const Order* order = order_manager_.find(order_id);
    if(order == nullptr){   return false;   }
    if(order->remaining_quantity() <= 0){   return true;   }   // already fulfilled, nothing to rest
    
    if(order_book_.contains(order_id)){ return true;    }   // prevents accidental duplicates
    
    return order_book_.add(*order); // if qtty exists, add it to book.
}

std::optional<Trade> MatchingEngine::execute_match(OrderId incoming_order_id, OrderId resting_order_id){
    const Order* incoming = order_manager_.find(incoming_order_id);
    const Order* resting = order_manager_.find(resting_order_id);

    if(incoming==nullptr || resting==nullptr){  return std::nullopt;    }
    if(incoming->remaining_quantity() <= 0 || resting->remaining_quantity() <= 0){  return std::nullopt;    }

    const Quantity qtty = incoming->remaining_quantity() < resting->remaining_quantity() ? incoming->remaining_quantity() : resting->remaining_quantity();

    if(!order_manager_.execute(incoming_order_id, qtty)){   return std::nullopt;    }
    if(!order_manager_.execute(resting_order_id, qtty)){    return std::nullopt;    }

    const Order* updated_resting = order_manager_.find(resting_order_id);
    if(updated_resting == nullptr){ return std::nullopt;    }
    if(updated_resting->state() == OrderState::Filled){ order_book_.remove(resting_order_id);   }

    return Trade {
        .incoming_order_id = incoming_order_id,
        .resting_order_id = resting_order_id,
        .price = resting->price(),
        .quantity = qtty
    };
}

MatchResult MatchingEngine::match(OrderId incoming_order_id){
    MatchResult result;

    const Order* incoming = order_manager_.find(incoming_order_id);
    if(incoming == nullptr){    return result;    }

    if(incoming->state() != OrderState::New && incoming->state() != OrderState::PartiallyFilled){   return result; }
    if(incoming->remaining_quantity() <= 0){    return result; }

    while(true){
        incoming = order_manager_.find(incoming_order_id);
        if(incoming == nullptr){    return result;    }
        if(incoming->remaining_quantity() <= 0){    break;    }

        // buy side
        if(incoming->side() == Side::Buy){
            const auto best_ask = order_book_.best_ask();
            if(!best_ask.has_value()){  break;  }

            const Order* resting = order_manager_.find(*best_ask);
            if(resting == nullptr){ return result;    }

            if(incoming->price() < resting->price()){   break;  }

            const auto trade = execute_match(incoming_order_id, resting->id());
            if(!trade.has_value()){ return result;    }
            result.trades.push_back(*trade);

            continue;
        }

        // sell side
        if(incoming->side() == Side::Sell){
            const auto best_bid = order_book_.best_bid();
            if(!best_bid.has_value()){  break;  }   // no liquidity case
            
            const Order* resting = order_manager_.find(*best_bid);
            if(resting == nullptr){ return result;    }

            if(incoming->price() > resting->price()){   break;  }

            const auto trade = execute_match(incoming_order_id, resting->id());
            if(!trade.has_value()){ return result;    }
            result.trades.push_back(*trade);

            continue;
        }

        return result;
    }
    // now we have no more executable liquidity
    // if theres still remining quantity in the incoming request, place it in the order book
    if(!rest_if_needed(incoming_order_id)){ return result;    }

    return result;
}
