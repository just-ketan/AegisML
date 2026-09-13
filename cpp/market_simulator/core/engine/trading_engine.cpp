#include "engine/trading_engine.hpp"
#include <vector>

TradingEngine::TradingEngine() : order_manager_(), market_state_manager_(order_manager_), execution_recorder_() {}

bool TradingEngine::process(const MarketEvent& event){
    if(!is_valid(event)){    return false;   }
    if(!sequence_validator_.accept(event.sequence_number)){ return false;   }

    // log the vent
    event_log_.append(event);

    if(event_type(event) == EventType::Add){
        const auto& add_order = std::get<AddOrderEvent>(event.payload);
        if(!order_manager_.process(event)){ return false;   }
    
        MarketState& state = market_state_manager_.get_or_create(event.symbol);
        
        const Order* order = order_manager_.find(add_order.order_id);
        if(order==nullptr){ return false;   }

        const MatchResult result = state.matching_engine().match(add_order.order_id);
        const ExecutionContext context{
            .event_id = event.event_id,
            .timestamp = event.timestamp,
            .symbol = event.symbol
        };

        for(const Trade& trade : result.trades){
            execution_recorder_.record(trade, context);
        }
        return true;
    }
    // return order_manager_.process(event);

    if(event_type(event) == EventType::Cancel){
        const auto& cancel_order = std::get<CancelOrderEvent>(event.payload);

        MarketState* state = market_state_manager_.find(event.symbol);
        if(state == nullptr){   return false;   }

        const Order* order = order_manager_.find(cancel_order.order_id);
        if(order == nullptr){   return false;   }
        
        if(!order_manager_.process(event)){ return false;   }
        if(!state->order_book().remove(cancel_order.order_id)){ return false;   }
        //if(!order_manager_.confirm_cancel(cancel_order.order_id)){  return false;   }

        return true;
    }
    return order_manager_.process(event);
}


std::size_t TradingEngine::process(IMarketDataSource& source){
    std::size_t processed = 0;
    MarketEvent event;
    while(source.next_event(event)){
        if(!process(event)){    break;  }
        ++processed;
    }
    return processed;
}

std::size_t TradingEngine::process_batch(IMarketDataSource& source,std::size_t max_events){
    if(max_events == 0){    return 0;   }
    std::vector<MarketEvent> batch;
    if(!source.next_batch(batch, max_events)){  return 0;   }

    std::size_t processed = 0;
    for(const MarketEvent& event : batch){
        if(!process(event)){    break;  }
        ++processed;
    }

    return processed;
}

const OrderManager& TradingEngine::order_manager() const {
    return order_manager_;
}

const MarketStateManager& TradingEngine::market_state_manager() const {
    return market_state_manager_;
}

const ExecutionRecorder& TradingEngine::execution_recorder() const {
    return execution_recorder_;
}

const EventLog& TradingEngine::event_log() const {
    return event_log_;
}