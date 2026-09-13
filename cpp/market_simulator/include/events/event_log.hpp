#pragma once

#include <cstddef>
#include <vector>

#include "events/market_event.hpp"

class EventLog{
    public:
        void append(const MarketEvent& event);
        std::size_t size() const;
        const MarketEvent& at(std::size_t index) const;
        const std::vector<MarketEvent>& events() const;
        void clear();
    
    private:
        std::vector<MarketEvent> events_;
};