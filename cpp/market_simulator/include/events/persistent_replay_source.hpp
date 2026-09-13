#pragma once

#include <cstddef>
#include <fstream>
#include <string>

#include "interfaces/market_data_source.hpp"

class PersistentReplaySource final : public IMarketDataSource{
    public:
        explicit PersistentReplaySource(const std::string& path);
        bool next_event(MarketEvent& event) override;
        bool next_batch(std::vector<MarketEvent>& batch, std::size_t max_events) override;

        void reset();
        std::size_t position() const;
        bool exhausted() const;

    private:
        std::ifstream file_;
        std::string path_;
        std::size_t position_=0;
        bool exhausted_ = false;
};
