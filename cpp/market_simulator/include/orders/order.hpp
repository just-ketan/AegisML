#pragma once

#include <cstdint>
#include "common/market_types.hpp"

using OrderId = std::uint64_t;

enum class Side{
    Buy, 
    Sell
};

constexpr bool is_valid_side(Side side){
    switch(side){
        case Side::Buy:
        case Side::Sell:
            return true;
    }
    return false;
}

enum class OrderState : std::uint64_t {
    New,
    PartiallyFilled,
    Filled,
    CancelPending,
    Cancelled,
    Rejected
};

constexpr bool can_transition(OrderState from, OrderState to){
    // no direct edge from New ───────────────► Cancelled
    switch(from){
        case OrderState::New:
            return to==OrderState::PartiallyFilled || to==OrderState::Filled || to==OrderState::CancelPending || to==OrderState::Rejected;
        case OrderState::PartiallyFilled:
            return to==OrderState::Filled || to==OrderState::CancelPending;
        case OrderState::CancelPending:
            return to==OrderState::Cancelled;

        case OrderState::Filled:
        case OrderState::Cancelled:
        case OrderState::Rejected:
            return false;
    }
    return false;
}

class Order{
    public:
        constexpr Order(OrderId id, Side side, Price price, std::uint64_t quantity)
            : id_(id), side_(side), price_(price), quantity_(quantity), filled_quantity_(0), remaining_quantity_(quantity),state_(OrderState::New) {}
        
        constexpr OrderId id() const { return id_; }
        constexpr Side side() const { return side_; }
        constexpr Price price() const { return price_; }
        constexpr Quantity quantity() const { return quantity_; }
        constexpr Quantity filled_quantity() const { return filled_quantity_; }
        constexpr Quantity remaining_quantity() const { return remaining_quantity_; }
        constexpr OrderState state() const { return state_; }
        constexpr bool transition(OrderState next){
            if(!can_transition(state_, next)){
                return false;
            }
            state_ = next;
            return true;
        }
        constexpr bool apply_fill(Quantity fill_quantity){
            if(fill_quantity <= 0){
                return false;
            }
            if (state_ == OrderState::Filled || state_ == OrderState::Cancelled || state_ == OrderState::Rejected || state_ == OrderState::CancelPending) { return false;}

            if(fill_quantity > remaining_quantity_){
                return false;
            }

            filled_quantity_ += fill_quantity;
            remaining_quantity_ -= fill_quantity;

            if(remaining_quantity_ == 0){
                return transition(OrderState::Filled);
            }
            if(state_ == OrderState::New){
                return transition(OrderState::PartiallyFilled);
            }
            return true;
        }
    private:
        OrderId id_;
        Side side_;
        Price price_;
        Quantity quantity_;
        Quantity filled_quantity_;
        Quantity remaining_quantity_;
        OrderState state_;
};