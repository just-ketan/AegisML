#include "execution/execution_recorder.hpp"

void ExecutionRecorder::record(const Trade& trade, const ExecutionContext& context){
    executions_.push_back(
        Execution{
            .execution_id = execution_id_++,
            .event_id = context.event_id,
            .timestamp = context.timestamp,
            .symbol = context.symbol,
            .incoming_order_id = trade.incoming_order_id,
            .resting_order_id = trade.resting_order_id,
            .price = trade.price,
            .quantity = trade.quantity
        }
    );
}

const Execution* ExecutionRecorder::find(ExecutionId execution_id) const {
    for(const auto& execution : executions_){
        if(execution.execution_id == execution_id){
            return &execution;
        }
    }
    return nullptr;
}

std::size_t ExecutionRecorder::size() const {
    return executions_.size();
}

const std::vector<Execution>& ExecutionRecorder::executions() const{
    return executions_;
}