# the reader processes binary info into immutable dataclass implementation

```py
uv run python -c "
from aegisml.data.schemas import (
    EventType,
    Side,
    MarketEventRecord,
    TradePayload,
)

event = MarketEventRecord(
    event_id=42,
    sequence_number=7,
    timestamp_ms=123456,
    symbol='AAPL',
    event_type=EventType.TRADE,
    payload=TradePayload(
        price=5_000_000_000,
        quantity=6_000_000_000,
    ),
)

print(event)
print('Event type:', event.event_type.name)
print('Price preserved:', event.payload.price == 5_000_000_000)
print('Buy side:', Side.BUY.value)
"
MarketEventRecord(event_id=42, sequence_number=7, timestamp_ms=123456, symbol='AAPL', event_type=<EventType.TRADE: 0>, payload=TradePayload(price=5000000000, quantity=6000000000))
Event type: TRADE
Price preserved: True
Buy side: 0
ketan@Ketan:/mnt/c/Users/ketan/Desktop/Projects/AegisML/python$ 
```

## Understanding the parsing
```yaml
| Field                |     Size | Encoding                            |
| -------------------- | -------: | ----------------------------------- |
| Record length prefix |  4 bytes | Unsigned, little-endian             |
| Magic bytes          |  4 bytes | `AEGM`                              |
| Format version       |  4 bytes | Unsigned, little-endian             |
| Event ID             |  8 bytes | Unsigned, little-endian             |
| Sequence number      |  8 bytes | Unsigned, little-endian             |
| Timestamp            |  8 bytes | Signed, little-endian; milliseconds |
| Symbol               |  8 bytes | Fixed-width byte array              |
| Event type           |   1 byte | Discriminator                       |
| Payload              | Variable | Depends on event type               |

```

The 5 payloads
```ymal
| Type          | Discriminator | Payload fields                                   |
| ------------- | ------------: | ------------------------------------------------ |
| Trade         |             0 | Price, quantity                                  |
| Quote         |             1 | Bid price, bid quantity, ask price, ask quantity |
| Add order     |             2 | Order ID, side, price, quantity                  |
| Cancel order  |             3 | Order ID, quantity                               |
| Execute order |             4 | Order ID, price, quantity                        |
```