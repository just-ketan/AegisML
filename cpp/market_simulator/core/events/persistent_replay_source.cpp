#include "events/persistent_replay_source.hpp"
#include "events/event_serializer.hpp"

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace{
    std::uint32_t read_u32(std::ifstream& file){
        std::uint32_t val = 0;
        for(int i=0; i<4; i++){
            char byte = 0;
            if(!file.get(byte)){    throw std::runtime_error("Truncated event : store record length");  }
            val |= static_cast<std::uint32_t>(static_cast<std::uint8_t>(byte)) << (i*8);
        }
        return val;
    }
}   // namespace ends

PersistentReplaySource::PersistentReplaySource(
    const std::string& path
)
    : file_(path, std::ios::in | std::ios::binary),
      path_(path)
{
    if (!file_.is_open()) {
        throw std::runtime_error(
            "Failed to open persistent replay file: " + path
        );
    }
}

bool PersistentReplaySource::next_event(MarketEvent& event){
    if(exhausted_){ return false;   }

    // clean EOF
    if(file_.peek() == std::ifstream::traits_type::eof()){
        exhausted_ = true;
        return false;
    }

    const std::uint32_t record_size = read_u32(file_);
    if(record_size == 0){
        throw std::runtime_error("Invalid zero lengthed event record");
    }

    std::vector<std::uint8_t> data(record_size);

    file_.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(record_size));

    if(!file_){
        throw std::runtime_error("Truncated event store event");
    }

    event = EventSerializer::deserialize(data);
    ++position_;
    return true;
}

bool PersistentReplaySource::next_batch(std::vector<MarketEvent>& batch, std::size_t max_events){
    batch.clear();
    if(max_events == 0){    return false;   }
    batch.reserve(max_events);

    while(batch.size() < max_events){
        MarketEvent event;
        if(!next_event(event)){ break;   }
        batch.push_back(event);
    }
    return !batch.empty();
}

void PersistentReplaySource::reset(){
    file_.close();
    file_.open(path_, std::ios::binary);
    if(!file_.is_open()){
        throw std::runtime_error("failed to reopen event store: " + path_);
    }
    position_ = 0;
    exhausted_ = false;
}

std::size_t PersistentReplaySource::position() const {
    return position_;
}

bool PersistentReplaySource::exhausted() const {
    return exhausted_;
}