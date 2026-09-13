#pragma once

#include <cstddef>

#include "events/market_event.hpp"
#include "execution/execution_recorder.hpp"
#include "orders/matching_engine.hpp"
#include "orders/order_book.hpp"
#include "orders/order_manager.hpp"
#include "market/market_state_manager.hpp"
#include "events/event_sequence.hpp"
#include "interfaces/market_data_source.hpp"
#include "events/event_log.hpp"

class TradingEngine{
    public:
        TradingEngine();

        bool process(const MarketEvent& event);
        std::size_t process(IMarketDataSource& source);
        std::size_t process_batch(IMarketDataSource& source,std::size_t max_events);

        const OrderManager& order_manager() const;
        const MarketStateManager& market_state_manager() const;
        const ExecutionRecorder& execution_recorder() const;

        const EventLog& event_log() const;

    private:
        OrderManager order_manager_;
        MarketStateManager market_state_manager_;
        ExecutionRecorder execution_recorder_;
        EventSequenceValidator sequence_validator_;
        EventLog event_log_;
};

