#pragma once

#include "time/clock.hpp"

class SimulationClock final : public IClock {
    public:
        explicit SimulationClock(Timestamp initial = Timestamp{0});
        Timestamp now() const override;
        void advance(Timestamp delta);
    private:
        Timestamp current_;
};