#include "orders/order_manager.hpp"
#include <type_traits>

bool OrderManager::process(const MarketEvent& event){
    return std::visit([this](const auto& payload) -> bool {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, AddOrderEvent>){
            if (orders_.find(payload.order_id) != orders_.end()) {
                return false;
            }

            orders_.emplace(
                payload.order_id,
                Order{
                    payload.order_id,
                    payload.side,
                    payload.price,
                    payload.quantity
                }
            );

            return true;
        }else if constexpr (std::is_same_v<T, ExecuteOrderEvent>){
            auto it = orders_.find(payload.order_id);
            if(it == orders_.end()){
                return false;
            }
            return it->second.apply_fill(payload.quantity);
        }else if constexpr (std::is_same_v<T, CancelOrderEvent>){
            auto it = orders_.find(payload.order_id);
            if(it == orders_.end()){ return false;  }

            return it->second.transition(OrderState::CancelPending);
        }else{
            // TradeEvent and QuoteEvent are not owned by OrderManager
            return true;
        }
    }, event.payload );
}

bool OrderManager::confirm_cancel(OrderId order_id){
    auto it = orders_.find(order_id);
    if(it == orders_.end()) return false;

    return it->second.transition(OrderState::Cancelled);
}

const Order* OrderManager::find(OrderId id) const {
    const auto it = orders_.find(id);
    if(it == orders_.end()){
        return nullptr;
    }
    return &it->second;
}

std::size_t OrderManager::size() const {
    return orders_.size();
}