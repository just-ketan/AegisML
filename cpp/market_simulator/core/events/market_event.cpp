#include "events/market_event.hpp"
#include <type_traits>

bool is_valid(const MarketEvent& event) {
    if (event.event_id == 0)    return false;
    if (event.sequence_number == 0) return false;
    if (event.timestamp.count() < 0)    return false;
    if (symbol_view(event.symbol).empty())   return false;

    // we can unpack each vairant type and validate the payload
    return std::visit([](const auto& payload){
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, TradeEvent>) {
            return payload.price > 0 && payload.quantity > 0;
        } else if constexpr (std::is_same_v<T, QuoteEvent>) {
            return payload.bid_price > 0 && payload.bid_quantity > 0 &&
                   payload.asking_price > 0 && payload.asking_quantity > 0 && payload.bid_price < payload.asking_price;
        } else if constexpr (std::is_same_v<T, AddOrderEvent>) {
            return payload.order_id > 0 && is_valid_side(payload.side) &&
                   payload.price > 0 && payload.quantity > 0;
        } else if constexpr (std::is_same_v<T, CancelOrderEvent>) {
            return payload.order_id > 0 && payload.quantity > 0;
        } else if constexpr (std::is_same_v<T, ExecuteOrderEvent>) {
            return payload.order_id > 0 && payload.price > 0 && payload.quantity > 0;
        }
        return false;
    }, event.payload
    );
}