#pragma once

#include <variant>

#include "events/market_event_payload.hpp"
#include "common/market_types.hpp"
#include "common/symbol.hpp"

using MarketEventPayload = std::variant<
    TradeEvent,
    QuoteEvent,
    AddOrderEvent,
    CancelOrderEvent,
    ExecuteOrderEvent
>;

struct MarketEvent {
    EventId event_id;
    SequenceNumber sequence_number;
    Timestamp timestamp;
    Symbol symbol;

    MarketEventPayload payload;
};

bool is_valid(const MarketEvent& event);