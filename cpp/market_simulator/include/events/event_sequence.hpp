#pragma once

#include "common/market_types.hpp"

class EventSequenceValidator{
    public:
        bool accept(SequenceNumber sequence);
        SequenceNumber next_expected() const;
        void reset();

    private:
        SequenceNumber next_expected_ = 1;
};