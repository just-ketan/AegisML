#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>

#include "common/symbol.hpp"
#include "market/market_state.hpp"

class MarketStateManager{
    public:
        explicit MarketStateManager(OrderManager& order_manager);

        MarketState& get_or_create(const Symbol& symbol);
        MarketState* find(const Symbol& symbol);
        const MarketState* find(const Symbol& symbol) const;
        std::size_t size() const;

    private:
        OrderManager& order_manager_;
        std::unordered_map<Symbol, std::unique_ptr<MarketState>, SymbolHash> states_;
};