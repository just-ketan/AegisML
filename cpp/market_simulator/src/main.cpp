#include <iostream>

#include "market_simulator.hpp"

int main() {

    SimulationConfig config{
        .event_count = 10,
        .seed = 42,
        .symbols = {
            {'A', 'A', 'P', 'L'},
            {'M', 'S', 'F', 'T'},
            {'G', 'O', 'O', 'G'}
        }
    };

    MarketSimulator simulator(config);

    MarketEvent event;

    while (simulator.next_event(event)) {

        std::cout
            << "id=" << event.event_id
            << " seq=" << event.sequence_number
            << " timestamp="
            << event.timestamp.count()
            << " symbol=";

        for (char c : event.symbol) {
            std::cout << c;
        }

        std::cout
            << " price=" << event.price
            << " quantity=" << event.quantity
            << '\n';
    }

    std::vector<MarketEvent> batch;
    while (simulator.next_batch(batch, 3)) {

        std::cout << "\nBatch size: " << batch.size() << '\n';

        for (const auto& event : batch) {
            std::cout
                << "id=" << event.event_id
                << " seq=" << event.sequence_number
                << '\n';
        }
    }

    return 0;
}