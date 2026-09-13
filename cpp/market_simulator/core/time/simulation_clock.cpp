#include "time/simulation_clock.hpp"

SimulationClock::SimulationClock(Timestamp initial) : current_(initial) {}

Timestamp SimulationClock::now() const{
    return current_;
}

void SimulationClock::advance(Timestamp delta){
    current_ += delta;
}