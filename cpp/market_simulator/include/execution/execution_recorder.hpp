#pragma once

#include <cstddef>
#include <vector>

#include "execution/execution.hpp"
#include "orders/matching_engine.hpp"
#include "execution/execution_context.hpp"

class ExecutionRecorder{
    public:
        void record(const Trade& trade, const ExecutionContext& context);
        const Execution* find(ExecutionId execution_id) const;
        std::size_t size() const;
        const std::vector<Execution>& executions() const;
    
    private:
        ExecutionId execution_id_ = 1;
        std::vector<Execution> executions_;
};