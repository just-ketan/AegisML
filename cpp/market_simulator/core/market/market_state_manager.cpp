#include "market/market_state_manager.hpp"

MarketStateManager::MarketStateManager(OrderManager& order_manager) : order_manager_(order_manager) {}

MarketState& MarketStateManager::get_or_create(const Symbol& symbol){
    auto it = states_.find(symbol);
    if(it == states_.end()){
        // need to create
        auto state = std::make_unique<MarketState>(order_manager_);
        auto [inserted_it, inserted] = states_.emplace(symbol, std::move(state));

        (void)inserted;
        return *inserted_it->second;
    }
    return *it->second;
}

const MarketState* MarketStateManager::find(const Symbol& symbol) const {
    const auto it = states_.find(symbol);
    if(it == states_.end()){    return nullptr; }
    return it->second.get();
}

std::size_t MarketStateManager::size() const{
    return states_.size();
}