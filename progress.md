# AegisML coding journey

## Phase 1 :  Order Manager
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

### So where we are as of today (11-9-26 18:20 IST)
```yaml
                    AegisML Market Engine
                           │
                           ▼
                    ┌──────────────┐
                    │ OrderManager │  ← WE ARE HERE
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │  OrderBook   │
                    │ Bid / Ask    │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │ Matching     │
                    │ Engine       │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │ Execution    │
                    │ Events       │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │ Market Data  │
                    │ / Metrics    │
                    └──────────────┘
```

## Phase 2: Order Book
for each symbol, 
```yaml
                    ORDER BOOK
                 ┌──────────────┐
       SELL      │  ASK LEVELS  │
                 │ 100.20 → ... │
                 │ 100.15 → ... │
                 │ 100.10 → ... │ ← BEST ASK
                 ├──────────────┤
                 │ 100.05 → ... │ ← BEST BID
                 │ 100.00 → ... │
       BUY       │  99.95 → ... │
                 └──────────────┘
```
we need two priority `1. price priority (for buyers -> higher priority associated to higher price, for sellers -> higher priority associated to lower price)` and `2.Time priority, at the same price, earlier order executes first`
```yaml
100.00:
    Order 101 → Order 105 → Order 109
       ↑           ↑           ↑
     first       second       third
```

SO the natural structure is to look at price levels as FIFO queue
```yaml
Price Level
    │
    └── FIFO queue of orders
```
and to look at orderbook as two queues (FIFO) with associated priority requirements
```yaml
             OrderBook
              /      \
             /        \
        Bids            Asks
     descending       ascending
         │                │
     price → FIFO     price → FIFO
```

so conceptually, 
```yaml
                 ┌───────────────┐
                 │ OrderManager  │
                 │               │
                 │ Order objects │
                 └───────┬───────┘
                         │
                    references
                         │
                         ▼
                 ┌───────────────┐
                 │   OrderBook   │
                 │               │
                 │ price levels  │
                 │ FIFO queues   │
                 └───────────────┘
```             

### Final DS call
So, we need something like:
```yaml
                         OrderBook
                            │
              ┌─────────────┼─────────────┐
              │             │             │
              ▼             ▼             ▼
            bids           asks       order_lookup
              │             │             │
          Price →        Price →       OrderId →
          FIFO queue     FIFO queue    location
```
as we need `price priority` , `FIFO time priority at same price` and `Fast cancellation by order id` all at the same time. we will use a std::list<OrderId> rather than storing order as `OrderManager owns the actual order`, thus OrderBook should maintain matchingt priority NOT DUPLICATE the order state
```yaml
OrderManager
    └── Order 12
          │
          │ OrderId = 12
          ▼
OrderBook
    └── 100.05
          └── [7 → 12]
```

### what happends with OrderBook

we dont want `orderbook` to become the matching engine. the responsibilities should be:
```yaml
                    Incoming Order
                          │
                          ▼
                  ┌───────────────┐
                  │ MatchingEngine│
                  └───────┬───────┘
                          │
             ┌────────────┴────────────┐
             │                         │
             ▼                         ▼
        OrderBook                  OrderManager
        priority                  lifecycle/state
             │                         │
             └──────────┬──────────────┘
                        ▼
                     Execution
```
`orderbook` asnwers "which resting order has priority" it owns `price levels`, `FIFO queues` and `order-location lookup`. `OrderManager` answers "what is the state of this order?", it owns `order`, `quantity`, `filled quantity`, `remaining quantity` and `lifecycle state`. Finally `MatchingEngine` answers "does this incoming order cross the book, and if so, what trades occur?"

### MatchingEngine

now the target flow becomes
```yaml
Incoming Order
      │
      ▼
┌───────────────┐
│ MatchingEngine│
└───────┬───────┘
        │
        ▼
   Is there a cross?
      /       \
    YES        NO
     │          │
     ▼          ▼
  Execute     Rest in
   trade      OrderBook
```
[BUY] : incoming price >= best_ask, for [SELL] incoming price <= best_bid, then `[execution quantity] = min(incoming.remaining_qtty(), resting.reamining_qtty())`

```yaml
MatchingEngine
       │
       │ "fill order 42 by 100"
       ▼
OrderManager::execute()
       │
       ▼
Order::apply_fill()
```
the ownership boundary will translate to "Machine Engine -> [fill order x by y qtty] -> ordermanager::execute() -> [order::apply_fill()]" THIS IS A GOOD ABSTRACTION BOUNDARY.

```yaml
Incoming BUY
      │
      ▼
best ASK exists?
      │
      ├── no  → no match
      │
      └── yes
            │
            ▼
      incoming.price >= ask.price?
            │
            ├── no  → no match
            │
            └── yes → EXECUTE
FOR SELL THE INEQUALITY IS REVERSED
```
is the matching and execution engine working hand in hand.

### Who puts the incoming order into the book, and when does the book remove a fully filled resting order?

we need to know what happens to resting stocks, leyts say demanded 100 @ 100, so we fulfil 40 in first go, then remaining 60 we need to check. this leads to multilevel matching
```yaml
              Incoming BUY
                   │
                   ▼
             Best ASK #1
                   │
             ┌─────┴─────┐
             │           │
          consume      remaining
             │           │
             ▼           ▼
          ASK #2  ───────┘
             │
             ▼
          ASK #3
             │
             ▼
       no more crossing
             │
             ▼
       remaining rests
```

so the whole flow becomes
```yaml
BUY                              SELL

best ask                         best bid
   │                                │
   ├── none → REST                  ├── none → REST
   │                                │
   ├── doesn't cross → REST        ├── doesn't cross → REST
   │                                │
   └── crosses                      └── crosses
          │                                │
          ▼                                ▼
       execute                          execute
          │                                │
          ▼                                ▼
    resting filled?                resting filled?
       │      │                       │      │
      yes     no                     yes     no
       │      │                       │      │
     remove  keep                    remove  keep
          \      /                       \      /
           incoming remaining             incoming remaining
                  │                              │
                  ▼                              ▼
              REST if needed                REST if needed
```

so finally the current invariant becomes
```yaml
Incoming Order
      │
      ▼
  Is there liquidity?
   │          │
  NO         YES
   │          │
   ▼          ▼
  REST      Does it cross?
             │       │
            NO      YES
             │       │
             ▼       ▼
            REST   EXECUTE
                     │
              ┌──────┴──────┐
              ▼             ▼
        Resting filled?   Remaining?
              │             │
             YES            YES
              │             │
           REMOVE         REST
```

so now we have `execute_match` so the architecture flow becomes

```yaml
match()
 ├── determine side
 ├── find best opposing order
 ├── determine whether prices cross
 └── execute_match()
        ├── validate
        ├── calculate quantity
        ├── execute incoming
        ├── execute resting
        └── remove filled resting order
```