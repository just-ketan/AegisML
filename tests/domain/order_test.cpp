#include <gtest/gtest.h>

#include "order.hpp"

TEST(OrderStateTest, NewTransitions) {
    EXPECT_TRUE(
        can_transition(OrderState::New,
                       OrderState::PartiallyFilled));

    EXPECT_TRUE(
        can_transition(OrderState::New,
                       OrderState::Filled));

    EXPECT_TRUE(
        can_transition(OrderState::New,
                       OrderState::CancelPending));

    EXPECT_TRUE(
        can_transition(OrderState::New,
                       OrderState::Rejected));
}

TEST(OrderStateTest, PartiallyFilledTransitions) {
    EXPECT_TRUE(
        can_transition(OrderState::PartiallyFilled,
                       OrderState::Filled));

    EXPECT_TRUE(
        can_transition(OrderState::PartiallyFilled,
                       OrderState::CancelPending));
}

TEST(OrderStateTest, CancelPendingTransitionsToCancelled) {
    EXPECT_TRUE(
        can_transition(OrderState::CancelPending,
                       OrderState::Cancelled));
}

TEST(OrderStateTest, FilledIsTerminal) {
    EXPECT_FALSE(
        can_transition(OrderState::Filled,
                       OrderState::Cancelled));

    EXPECT_FALSE(
        can_transition(OrderState::Filled,
                       OrderState::New));
}

TEST(OrderStateTest, CancelledIsTerminal) {
    EXPECT_FALSE(
        can_transition(OrderState::Cancelled,
                       OrderState::New));

    EXPECT_FALSE(
        can_transition(OrderState::Cancelled,
                       OrderState::Filled));
}

TEST(OrderStateTest, RejectedIsTerminal) {
    EXPECT_FALSE(
        can_transition(OrderState::Rejected,
                       OrderState::New));

    EXPECT_FALSE(
        can_transition(OrderState::Rejected,
                       OrderState::Filled));
}

TEST(OrderStateTest, CannotMoveBackward) {
    EXPECT_FALSE(
        can_transition(OrderState::PartiallyFilled,
                       OrderState::New));
}

TEST(OrderStateTest, CannotCancelFilledOrder) {
    EXPECT_FALSE(
        can_transition(OrderState::Filled,
                       OrderState::CancelPending));
}

TEST(OrderStateTest, CannotFillCancelledOrder) {
    EXPECT_FALSE(
        can_transition(OrderState::Cancelled,
                       OrderState::Filled));
}

TEST(OrderStateTest, CannotRejectFilledOrder) {
    EXPECT_FALSE(
        can_transition(OrderState::Filled,
                       OrderState::Rejected));
}

TEST(OrderTest, ConstructsWithCorrectInitialState) {
    Order order(42, Side::Buy, 10000, 100);

    EXPECT_EQ(order.id(), 42);
    EXPECT_EQ(order.side(), Side::Buy);
    EXPECT_EQ(order.price(), 10000);
    EXPECT_EQ(order.quantity(), 100);

    EXPECT_EQ(order.filled_quantity(), 0);
    EXPECT_EQ(order.remaining_quantity(), 100);
    EXPECT_EQ(order.state(), OrderState::New);
}

TEST(OrderTest, AppliesFullFill) {
    Order order(42, Side::Buy, 10000, 100);

    EXPECT_TRUE(order.apply_fill(100));

    EXPECT_EQ(order.filled_quantity(), 100);
    EXPECT_EQ(order.remaining_quantity(), 0);
    EXPECT_EQ(order.state(), OrderState::Filled);
}

TEST(OrderTest, AppliesMultipleFills) {
    Order order(42, Side::Buy, 10000, 100);

    EXPECT_TRUE(order.apply_fill(30));
    EXPECT_TRUE(order.apply_fill(20));
    EXPECT_TRUE(order.apply_fill(50));

    EXPECT_EQ(order.filled_quantity(), 100);
    EXPECT_EQ(order.remaining_quantity(), 0);
    EXPECT_EQ(order.state(), OrderState::Filled);
}

TEST(OrderTest, RejectsOverFill) {
    Order order(42, Side::Buy, 10000, 100);

    EXPECT_FALSE(order.apply_fill(101));

    EXPECT_EQ(order.filled_quantity(), 0);
    EXPECT_EQ(order.remaining_quantity(), 100);
    EXPECT_EQ(order.state(), OrderState::New);
}

TEST(OrderTest, RejectsInvalidFillQuantity) {
    Order order(42, Side::Buy, 10000, 100);

    EXPECT_FALSE(order.apply_fill(0));
    EXPECT_FALSE(order.apply_fill(-10));

    EXPECT_EQ(order.filled_quantity(), 0);
    EXPECT_EQ(order.remaining_quantity(), 100);
    EXPECT_EQ(order.state(), OrderState::New);
}

TEST(OrderTest, FilledOrderCannotBeModified) {
    Order order(42, Side::Buy, 10000, 100);

    EXPECT_TRUE(order.apply_fill(100));

    EXPECT_FALSE(order.transition(OrderState::Cancelled));
    EXPECT_FALSE(order.transition(OrderState::Rejected));

    EXPECT_EQ(order.state(), OrderState::Filled);
}

TEST(OrderTest, CancelledOrderCannotBeFilled) {
    Order order(42, Side::Buy, 10000, 100);

    EXPECT_TRUE(order.transition(OrderState::CancelPending));
    EXPECT_TRUE(order.transition(OrderState::Cancelled));

    EXPECT_FALSE(order.apply_fill(50));

    EXPECT_EQ(order.filled_quantity(), 0);
    EXPECT_EQ(order.remaining_quantity(), 100);
    EXPECT_EQ(order.state(), OrderState::Cancelled);
}

TEST(OrderTest, CancelPendingOrderCannotBeFilled) {
    Order order(42, Side::Buy, 10000, 100);

    EXPECT_TRUE(order.transition(OrderState::CancelPending));

    EXPECT_FALSE(order.apply_fill(50));

    EXPECT_EQ(order.filled_quantity(), 0);
    EXPECT_EQ(order.remaining_quantity(), 100);
}