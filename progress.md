
### Define event semantics

Let's first define what each event actually means.
| Event     | Meaning                     | Required information            |
| --------- | --------------------------- | ------------------------------- |
| `Trade`   | A transaction occurred      | price, quantity                 |
| `Quote`   | Current best bid/ask        | bid price/qty, ask price/qty    |
| `Add`     | New order entered book      | order ID, side, price, quantity |
| `Cancel`  | Existing order cancelled    | order ID, quantity              |
| `Execute` | Existing order was executed | order ID, price, quantity       |

### Use a tagged payload

Instead, let's separate the event envelope from the event payload
```yaml
                    MarketEvent
                         │
          ┌──────────────┴──────────────┐
          │                             │
       Envelope                       Payload
          │                             │
   event_id                         TradeEvent
   sequence_number                  QuoteEvent
   timestamp                        AddOrderEvent
   symbol                           CancelOrderEvent
   event_type                       ExecuteEvent
```

### Phase goal: order should have a lifecycle
```yaml
NEW
 │
 ├──> PARTIALLY_FILLED
 │         │
 │         ├──> FILLED
 │         └──> CANCEL_PENDING ──> CANCELLED
 │
 ├──> CANCEL_PENDING ──> CANCELLED
 │
 └──> REJECTED
```
`Market events describe what happened; order state describes the current truth of the order.`
```yaml
             Order
               │
       ┌───────┴────────┐
       │                │
 original            lifecycle
 quantity             state
       │                │
      100              NEW
       │
 ┌─────┴─────┐
 │           │
filled     remaining
  0           100
```

### connect executeOrderEvent to `order` lifecycle
```yaml
AddOrderEvent
      │
      ▼
 Order created
      │
      │ order_id = 42
      ▼
Order stored in lookup
      │
      ├──────────────┐
      ▼              ▼
CancelOrder(42)   ExecuteOrder(42)
                     │
                     ▼
              Order::apply_fill()
                     │
             ┌───────┴───────┐
             ▼               ▼
        Partial fill     Full fill
```

### how the data moves ?

```yaml
IMarketDataSource
        │
        ▼
MarketSimulator
        │
        │ produces
        ▼
   MarketEvent
```
MarketEvent is deliberately just a value object and payload is std::variant
```yaml
event_id
sequence_number
timestamp
symbol
payload
```
we will keep `MarketSimulator::next_event()` as purely an event generator so that `MarketSimulator generates events` and `OrderManager maintains order state + processes (add/exec/cancel) tasks`

### Repo restructure

we are restructuring the repo to have the following structure for maintainance and readability
```yaml
                    AegisML Market Engine
                            │
          ┌─────────────────┼─────────────────┐
          │                 │                 │
       Events             Orders           Market
          │                 │                 │
          │                 │                 │
          ▼                 ▼                 ▼
      event bus        order manager      data source
                            │
                            ▼
                     matching engine
                            │
                            ▼
                       order book
```

### how to handle cancel events ?

```yaml
                  AddOrderEvent
                       │
                       ▼
                     New
                       │
              ┌────────┴────────┐
              │                 │
         Execute             Cancel
              │                 │
              ▼                 ▼
     PartiallyFilled      CancelPending
              │                 │
         Execute           confirmation
              │                 │
              ▼                 ▼
           Filled             Cancelled
```
cancel event goes throught intermediate steps before considered as terminated. so `it need not necessarily mean that the cancellation has already completed`

