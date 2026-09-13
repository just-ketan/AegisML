#include "events/event_store.hpp"

#include <cstdint>
#include <stdexcept>

#include "events/event_serializer.hpp"

EventStore::EventStore(const std::string& path) : file_(path, std::ios::binary | std::ios::app){
    if(!file_.is_open()){
        throw std::runtime_error("Failed to open event store: "+path);
    }
}

EventStore::~EventStore(){
    flush();
}

void EventStore::append(const MarketEvent& event){
    const std::vector<std::uint8_t> data = EventSerializer::serialize(event);
    const std::uint32_t size = static_cast<std::uint32_t>(data.size());

    // little endian record length
    for(int i=0; i<4; i++){
        const std::uint8_t byte = static_cast<std::uint8_t>(
            (size >> (i*8)) & 0xFF
        );

        file_.put(static_cast<char>(byte));
    }

    file_.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));

    if(!file_){
        throw std::runtime_error("Failed to write event to event store");
    }

    ++size_;
}

std::size_t EventStore::size() const {
    return size_;
}

void EventStore::flush(){
    file_.flush();
    if(!file_){
        throw std::runtime_error("failed to flush event store");
    }
}