#include "market_simulator.hpp"

#include <stdexcept>
#include <utility>

MarketSimulator::MarketSimulator(const SimulationConfig& config)
    :config_(config), 
    rng_(config.seed), 
    symbol_distribution_(0, config.symbols.empty() ? 0 : config.symbols.size()-1),
    event_type_distribution_(0,4),
    price_distribution_(1'000'000,2'000'000),
    quantity_distribution_(1,1'000){
        if(config_.symbols.empty()){
            throw std::invalid_argument("MarketSimulator requires atleast one symbol");
        }
        if(config_.event_count==0){
            throw std::invalid_argument("MarketSimulator requires event_count > 0");
        }
}

bool MarketSimulator::next_event(MarketEvent& event){
    if(events_generated_ >= config_.event_count){
        return false;
    }
    event.event_id = next_event_id_++;
    event.sequence_number = next_sequence_number_++;
    current_timestamp_ += std::chrono::milliseconds{1};
    event.timestamp = current_timestamp_;

    // select random symbol
    const auto symbol_index = symbol_distribution_(rng_);
    event.symbol = config_.symbols[symbol_index];

    // select random event type
    const int event_type_value = event_type_distribution_(rng_);
    event.event_type = static_cast<EventType>(event_type_value);

    // fixed point price range from 1,000,000 -> 100,0000 and 2,000,000 -> 200,0000
    event.price = price_distribution_(rng_);
    event.quantity = quantity_distribution_(rng_);

    ++events_generated_;
    if (!is_valid(event)) {
        throw std::runtime_error("MarketSimulator generated invalid MarketEvent");
    }

    return true;
}

bool MarketSimulator::next_batch(std::vector<MarketEvent>& batch, std::size_t max_events){
    batch.clear();
    batch.reserve(max_events);
    while(batch.size() < max_events){
        MarketEvent event;
        if(!next_event(event)){ break;  }
        batch.push_back(std::move(event));
    }
    return !batch.empty();
}