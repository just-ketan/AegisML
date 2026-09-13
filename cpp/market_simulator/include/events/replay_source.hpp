#pragma once

#include <cstddef>

#include "events/event_log.hpp"
#include "interfaces/market_data_source.hpp"

class ReplaySource final : public IMarketDataSource {
    public:
        explicit ReplaySource(const EventLog& log);
        bool next_event(MarketEvent& event) override;
        bool next_batch(std::vector<MarketEvent>& batch, std::size_t max_events) override;
        void reset();
        std::size_t position() const;
        bool exhausted() const;
    
    private:
        const EventLog& log_;
        std::size_t position_ = 0;
};
