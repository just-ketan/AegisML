#include "events/event_log.hpp"

void EventLog::append(const MarketEvent& event){
    events_.push_back(event);
}

std::size_t EventLog::size() const {
    return events_.size();
}

const MarketEvent& EventLog::at(std::size_t index) const {
    return events_.at(index);
}

const std::vector<MarketEvent>& EventLog::events() const {
    return events_;
}

void EventLog::clear() {
    events_.clear();
}