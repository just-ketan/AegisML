#include "market_event.hpp"

bool is_valid(const MarketEvent& event) {

    if (event.event_id == 0) {
        return false;
    }

    if (event.sequence_number == 0) {
        return false;
    }

    if (event.timestamp.count() < 0) {
        return false;
    }

    if (event.symbol.empty()) {
        return false;
    }

    if (!is_valid_event_type(event.event_type)) {
        return false;
    }

    if (event.price <= 0) {
        return false;
    }

    if (event.quantity <= 0) {
        return false;
    }

    return true;
}