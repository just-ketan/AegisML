#include "market_simulator.hpp"
#include "event_type.hpp"
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
    
    switch(static_cast<EventType>(event_type_value)){
        case EventType::Trade:{
            TradeEvent trade_event;
            trade_event.price = price_distribution_(rng_);
            trade_event.quantity = quantity_distribution_(rng_);
            event.payload = std::move(trade_event);
            break;
        }
        case EventType::Quote:{
            QuoteEvent quote_event;
            quote_event.bid_price = price_distribution_(rng_);
            quote_event.bid_quantity = quantity_distribution_(rng_);
            quote_event.asking_price = price_distribution_(rng_);
            quote_event.asking_quantity = quantity_distribution_(rng_);
            event.payload = std::move(quote_event);
            break;
        }
        case EventType::Add:{
            AddOrderEvent add_order_event;
            add_order_event.order_id = next_order_id_++;
            add_order_event.side = (event_type_distribution_(rng_) % 2 == 0) ? Side::Buy : Side::Sell;
            add_order_event.price = price_distribution_(rng_);
            add_order_event.quantity = quantity_distribution_(rng_);
            event.payload = std::move(add_order_event);
            break;
        }
        case EventType::Cancel:{
            CancelOrderEvent cancel_order_event;
            cancel_order_event.order_id = next_sequence_number_++;
            cancel_order_event.quantity = quantity_distribution_(rng_);
            event.payload = std::move(cancel_order_event);
            break;
        }
        case EventType::Execute:{
            ExecuteOrderEvent execute_order_event;
            execute_order_event.order_id = next_sequence_number_++;
            execute_order_event.price = price_distribution_(rng_);
            execute_order_event.quantity = quantity_distribution_(rng_);
            event.payload = std::move(execute_order_event);
            break;
        }
    };

    ++events_generated_;

    if(!is_valid(event)){
        throw std::runtime_error("Generated invalid event");
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