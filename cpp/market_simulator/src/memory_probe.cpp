#include <cstddef>
#include <iostream>

#include "market_event.hpp"

int main() {

    std::cout << "MarketEvent\n";
    std::cout << "-------------------------\n";

    std::cout
        << "sizeof(MarketEvent): "
        << sizeof(MarketEvent)
        << " bytes\n";

    std::cout
        << "alignof(MarketEvent): "
        << alignof(MarketEvent)
        << " bytes\n\n";


    std::cout
        << "sizeof(EventId): "
        << sizeof(EventId)
        << '\n';

    std::cout
        << "sizeof(SequenceNumber): "
        << sizeof(SequenceNumber)
        << '\n';

    std::cout
        << "sizeof(Timestamp): "
        << sizeof(Timestamp)
        << '\n';

    std::cout
        << "sizeof(std::vector<char>): "
        << sizeof(std::vector<char>)
        << '\n';

    std::cout
        << "sizeof(EventType): "
        << sizeof(EventType)
        << '\n';

    std::cout
        << "sizeof(Price): "
        << sizeof(Price)
        << '\n';

    std::cout
        << "sizeof(Quantity): "
        << sizeof(Quantity)
        << '\n';


    std::cout << "\nOffsets\n";
    std::cout << "-------------------------\n";

    std::cout
        << "event_id: "
        << offsetof(MarketEvent, event_id)
        << '\n';

    std::cout
        << "sequence_number: "
        << offsetof(MarketEvent, sequence_number)
        << '\n';

    std::cout
        << "timestamp: "
        << offsetof(MarketEvent, timestamp)
        << '\n';

    std::cout
        << "symbol: "
        << offsetof(MarketEvent, symbol)
        << '\n';

    std::cout
        << "event_type: "
        << offsetof(MarketEvent, event_type)
        << '\n';

    std::cout
        << "price: "
        << offsetof(MarketEvent, price)
        << '\n';

    std::cout
        << "quantity: "
        << offsetof(MarketEvent, quantity)
        << '\n';

    std::cout << "\nSymbol storage\n";
std::cout << "-------------------------\n";

MarketEvent event{
    .event_id = 1,
    .sequence_number = 1,
    .timestamp = Timestamp{100},
    .symbol = {'A', 'A', 'P', 'L'},
    .event_type = EventType::Trade,
    .price = 2254300,
    .quantity = 100
};

std::cout
    << "symbol.size(): "
    << event.symbol.size()
    << '\n';

std::cout
    << "symbol.capacity(): "
    << event.symbol.capacity()
    << '\n';

std::cout
    << "&event.symbol: "
    << static_cast<const void*>(&event.symbol)
    << '\n';

std::cout
    << "event.symbol.data(): "
    << static_cast<const void*>(event.symbol.data())
    << '\n';
    return 0;
}