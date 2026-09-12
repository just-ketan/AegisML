#include "market/market_state.hpp"

MarketState::MarketState(OrderManager& order_mgr) : order_book_(), matching_engine_(order_mgr, order_book_) {}

OrderBook& MarketState::order_book(){
    return order_book_;
}

const OrderBook& MarketState::order_book() const {
    return order_book_;
}

MatchingEngine& MarketState::matching_engine() {
    return matching_engine_;
}

const MatchingEngine& MarketState::matching_engine() const {
    return matching_engine_;
}

// simple composition