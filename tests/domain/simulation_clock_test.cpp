#include <gtest/gtest.h>

#include "time/simulation_clock.hpp"


TEST(SimulationClockTest, StartsAtInitialTimestamp)
{
    SimulationClock clock{Timestamp{1000}};

    EXPECT_EQ(clock.now(), Timestamp{1000});
}


TEST(SimulationClockTest, DefaultsToZero)
{
    SimulationClock clock;

    EXPECT_EQ(clock.now(), Timestamp{0});
}


TEST(SimulationClockTest, AdvanceMovesTimeForward)
{
    SimulationClock clock{Timestamp{1000}};

    clock.advance(Timestamp{250});

    EXPECT_EQ(clock.now(), Timestamp{1250});
}


TEST(SimulationClockTest, MultipleAdvancesAccumulate)
{
    SimulationClock clock;

    clock.advance(Timestamp{100});
    clock.advance(Timestamp{200});
    clock.advance(Timestamp{300});

    EXPECT_EQ(clock.now(), Timestamp{600});
}


TEST(SimulationClockTest, NowDoesNotAdvanceTime)
{
    SimulationClock clock{Timestamp{1000}};

    EXPECT_EQ(clock.now(), Timestamp{1000});
    EXPECT_EQ(clock.now(), Timestamp{1000});
}