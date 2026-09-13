#pragma once

#include <cstdint>
#include <vector>

#include "events/market_event.hpp"

namespace EventSerializer{
    constexpr std::uint32_t FORMAT_VERSION = 1;

    std::vector<std::uint8_t> serialize(const MarketEvent& event);
    MarketEvent deserialize(const std::vector<std::uint8_t>& data);
}