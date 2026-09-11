#include <iostream>
#include <variant>

#include "market_simulator.hpp"
#include "symbol.hpp"

int main() {
    SimulationConfig config{
        .event_count = 10,
        .seed = 42,
        .symbols = {
            make_symbol("AAPL"),
            make_symbol("MSFT"),
            make_symbol("GOOGL"),
            make_symbol("AMZN"),
            make_symbol("TSLA")
        }
    };

    MarketSimulator simulator(config);

    MarketEvent event;

    while (simulator.next_event(event)) {
        std::cout
            << "id=" << event.event_id
            << " seq=" << event.sequence_number
            << " timestamp=" << event.timestamp.count()
            << " symbol=" << symbol_view(event.symbol);

        std::visit(
            [](const auto& payload) {
                using T = std::decay_t<decltype(payload)>;

                if constexpr (std::is_same_v<T, TradeEvent>) {
                    std::cout
                        << " type=Trade"
                        << " price=" << payload.price
                        << " quantity=" << payload.quantity;
                }
                else if constexpr (std::is_same_v<T, QuoteEvent>) {
                    std::cout
                        << " type=Quote"
                        << " bid=" << payload.bid_price
                        << " ask=" << payload.asking_price;
                }
                else if constexpr (std::is_same_v<T, AddOrderEvent>) {
                    std::cout
                        << " type=Add"
                        << " order_id=" << payload.order_id
                        << " price=" << payload.price
                        << " quantity=" << payload.quantity;
                }
                else if constexpr (std::is_same_v<T, CancelOrderEvent>) {
                    std::cout
                        << " type=Cancel"
                        << " order_id=" << payload.order_id
                        << " quantity=" << payload.quantity;
                }
                else if constexpr (std::is_same_v<T, ExecuteOrderEvent>) {
                    std::cout
                        << " type=Execute"
                        << " order_id=" << payload.order_id
                        << " price=" << payload.price
                        << " quantity=" << payload.quantity;
                }
            },
            event.payload
        );

        std::cout << '\n';
    }

    return 0;
}