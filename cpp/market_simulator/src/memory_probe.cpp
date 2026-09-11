#include <cstddef>
#include <iostream>
#include <variant>

#include "market_event.hpp"
#include "market_event_payload.hpp"
#include "market_types.hpp"
#include "order.hpp"
#include "symbol.hpp"


int main()
{
    std::cout << "MarketEvent Memory Layout\n";
    std::cout << "========================\n\n";

    std::cout
        << "sizeof(MarketEvent): "
        << sizeof(MarketEvent)
        << " bytes\n";

    std::cout
        << "alignof(MarketEvent): "
        << alignof(MarketEvent)
        << " bytes\n\n";


    std::cout << "Payload Sizes\n";
    std::cout << "-------------\n";

    std::cout
        << "sizeof(TradeEvent): "
        << sizeof(TradeEvent)
        << " bytes\n";

    std::cout
        << "sizeof(QuoteEvent): "
        << sizeof(QuoteEvent)
        << " bytes\n";

    std::cout
        << "sizeof(AddOrderEvent): "
        << sizeof(AddOrderEvent)
        << " bytes\n";

    std::cout
        << "sizeof(CancelOrderEvent): "
        << sizeof(CancelOrderEvent)
        << " bytes\n";

    std::cout
        << "sizeof(ExecuteOrderEvent): "
        << sizeof(ExecuteOrderEvent)
        << " bytes\n";

    std::cout
        << "sizeof(MarketEventPayload): "
        << sizeof(MarketEventPayload)
        << " bytes\n\n";


    std::cout << "Primitive Types\n";
    std::cout << "---------------\n";

    std::cout
        << "sizeof(EventId): "
        << sizeof(EventId)
        << " bytes\n";

    std::cout
        << "sizeof(SequenceNumber): "
        << sizeof(SequenceNumber)
        << " bytes\n";

    std::cout
        << "sizeof(Timestamp): "
        << sizeof(Timestamp)
        << " bytes\n";

    std::cout
        << "sizeof(Symbol): "
        << sizeof(Symbol)
        << " bytes\n";

    std::cout
        << "sizeof(OrderId): "
        << sizeof(OrderId)
        << " bytes\n";

    std::cout
        << "sizeof(Price): "
        << sizeof(Price)
        << " bytes\n";

    std::cout
        << "sizeof(Quantity): "
        << sizeof(Quantity)
        << " bytes\n\n";


    std::cout << "MarketEvent Offsets\n";
    std::cout << "-------------------\n";

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
        << "payload: "
        << offsetof(MarketEvent, payload)
        << '\n';


    std::cout << "\nVariant Information\n";
    std::cout << "-------------------\n";

    std::cout
        << "variant alternatives: "
        << std::variant_size_v<MarketEventPayload>
        << '\n';

    std::cout
        << "MarketEventPayload alignment: "
        << alignof(MarketEventPayload)
        << " bytes\n";


    std::cout << "\nSymbol Storage\n";
    std::cout << "--------------\n";

    MarketEvent event{
        .event_id = 1,
        .sequence_number = 1,
        .timestamp = Timestamp{1000},
        .symbol = make_symbol("AAPL"),
        .payload = TradeEvent{
            .price = 15000,
            .quantity = 100
        }
    };

    std::cout
        << "symbol.size(): "
        << event.symbol.size()
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