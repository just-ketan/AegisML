#pragma once

#include "common/market_types.hpp"
#include "common/symbol.hpp"

struct ExecutionContext {
    EventId event_id;
    Timestamp timestamp;
    Symbol symbol;
};