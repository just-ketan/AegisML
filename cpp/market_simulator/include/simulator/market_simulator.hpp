#pragma once
#include <chrono>
#include <cstdint>
#include <random>
#include <vector>
#include <unordered_map>    // order lookup


#include "interfaces/market_data_source.hpp"
#include "orders/order.hpp"


struct SimulationConfig{
    std::uint64_t event_count;
    std::uint64_t seed;
    std::vector<Symbol> symbols;
};

class MarketSimulator final : public IMarketDataSource{
    public:
        explicit MarketSimulator(const SimulationConfig& config);
        bool next_event(MarketEvent& event) override;
        bool next_batch(std::vector<MarketEvent>& batch, std::size_t max_events) override;
    
    private:
        SimulationConfig config_;
        std::mt19937_64 rng_;
        std::uint64_t events_generated_ = 0;
        std::uint64_t next_event_id_ = 1;
        std::uint64_t next_order_id_ = 1;
        std::uint64_t next_sequence_number_ = 1;
        Timestamp current_timestamp_{0};

        std::uniform_int_distribution<std::size_t> symbol_distribution_;
        std::uniform_int_distribution<int> event_type_distribution_;
        std::uniform_int_distribution<Price> price_distribution_;
        std::uniform_int_distribution<Quantity> quantity_distribution_;

        // order lookup
        std::unordered_map<OrderId, Order> active_order_ids_;
};