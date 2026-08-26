#pragma once

#include <cstdint>
#include <array>
#include <chrono>
#include "event_type.hpp"

using EventId = std::uint64_t;
using SequenceNumber = std::uint64_t;
using Timestamp = std::chrono::milliseconds;
constexpr std::size_t MAX_SYMBOL_LENGTH = 8;
using Symbol = std::array<char,MAX_SYMBOL_LENGTH>;
using Price = std::int64_t; // fixed point prices, scale is 1e4. so if 225.4300 so price = 2254300
using Quantity = std::int64_t;

struct MarketEvent{
    EventId event_id;
    SequenceNumber sequence_number;
    Timestamp timestamp;
    Symbol symbol;
    EventType event_type;
    Price price;
    Quantity quantity;
};
// now the entire event is self-contained
/*

MarketEvent
┌────────────────────────────────────┐
│ event_id             8             │
│ sequence_number      8             │
│ timestamp            8             │
│ symbol               8             │
│ event_type           4             │
│ padding              4             │
│ price                8             │
│ quantity             8             │
└────────────────────────────────────┘
             48 bytes
*/

bool is_valid(const MarketEvent& event);

