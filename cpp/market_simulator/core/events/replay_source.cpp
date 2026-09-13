#include "events/replay_source.hpp"

ReplaySource::ReplaySource(const EventLog& log) : log_(log) {}

bool ReplaySource::next_event(MarketEvent& event){
    if(position_ >= log_.size()){   return false;   }
    event = log_.at(position_);
    ++position_;
    return true;
}

bool ReplaySource::next_batch(std::vector<MarketEvent>& batch, std::size_t max_events){
    batch.clear();
    if(max_events == 0 || position_ >= log_.size()){ return false;   }

    const std::size_t remaining = log_.size() - position_;
    const std::size_t count = remaining < max_events ? remaining : max_events;
    batch.reserve(count);

    for(std::size_t i=0; i<count; i++){
        batch.push_back(log_.at(position_));
        ++position_;
    }
    return true;
}

void ReplaySource::reset(){
    position_=0;
}

std::size_t ReplaySource::position() const{
    return position_;
}

bool ReplaySource::exhausted() const {
    return position_ >= log_.size();
}