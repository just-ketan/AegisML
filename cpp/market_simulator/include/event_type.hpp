#pragma once

enum class EventType{
    Trade,
    Quote,
    Add,
    Cancel,
    Execute
};

constexpr bool is_valid_event_type(EventType type) {
    switch (type) {
        case EventType::Trade:
        case EventType::Quote:
        case EventType::Add:
        case EventType::Cancel:
        case EventType::Execute:
            return true;
    }

    return false;
}