#pragma once

#include "common/market_types.hpp"

class IClock{
    public:
        virtual ~IClock() = default;
        virtual Timestamp now() const = 0;  // pure so this class is abstract
};