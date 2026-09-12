#include "engine/trading_engine.hpp"

TradingEngine::TradingEngine() : order_manager_(), order_book_(), matching_engine_(order_manager_, order_book_), execution_recorder_() {}

bool TradingEngine::process(const MarketEvent& event){
    if(!is_valid(event)){    return false;   }
    if(std::holds_alternative<AddOrderEvent>(event.payload)){
        const auto& add_order = std::get<AddOrderEvent>(event.payload);
        
        if(!order_manager_.process(event)){ return false;   }
        if(!order_book_.add(*order_manager_.find(add_order.order_id))){ return false;   }

        const MatchResult result = matching_engine_.match(add_order.order_id);
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

const OrderBook& TradingEngine::order_book() const {
    return order_book_;
}

const ExecutionRecorder& TradingEngine::execution_recorder() const {
    return execution_recorder_;
}