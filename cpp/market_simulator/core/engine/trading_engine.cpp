#include "engine/trading_engine.hpp"

TradingEngine::TradingEngine() : order_manager_(), market_state_manager_(order_manager_), execution_recorder_() {}

bool TradingEngine::process(const MarketEvent& event){
    if(!is_valid(event)){    return false;   }
    if(std::holds_alternative<AddOrderEvent>(event.payload)){
        const auto& add_order = std::get<AddOrderEvent>(event.payload);
        
        if(!order_manager_.process(event)){ return false;   }
        
        MarketState& state = market_state_manager_.get_or_create(event.symbol);
        
        const Order* order = order_manager_.find(add_order.order_id);
        if(order==nullptr){ return false;   }
        if(!state.order_book().add(*order)){    return false;   }

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
    return order_manager_.process(event);
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