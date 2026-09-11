#pragma once
#include <vector>
#include "events/market_event.hpp"

class IMarketDataSource{
    public:
        virtual ~IMarketDataSource() = default;
        virtual bool next_event(MarketEvent& event) = 0;    // pre virtual function
        virtual bool next_batch(std::vector<MarketEvent>& batch, std::size_t max_events) = 0;
};