#pragma once

#include <cstdint>

using OrderId = std::uint64_t;

enum class Side{
    Buy, 
    Sell
};

constexpr bool is_valid_side(Side side){
    switch(side){
        case Side::Buy:
        case Side::Sell:
            return true;
    }
    return false;
}