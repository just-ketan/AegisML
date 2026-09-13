#pragma once

#include <cstddef>
#include <string>
#include <fstream>

#include "events/market_event.hpp"

class EventStore{
    public:
        explicit EventStore(const std::string& path);
        
        ~EventStore();
        EventStore(const EventStore&) = delete;
        EventStore& operator==(const EventStore&) = delete;

        void append(const MarketEvent& event);
        std::size_t size() const;
        void flush();
    
    private:
        std::string path_;
        std::ofstream file_;
        std::size_t size_=0;
};