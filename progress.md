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

so we have succesfully implemented `selection logic -> match()`, `execution logic -> execute_match()` and `book lifecycle -> rest_if_needed()`. Now the pipeline coverts to 
```yaml
incoming
   ↓
best opposing order
   ↓
trade
   ↓
still quantity remaining?
   │
   ├── YES → find NEW best opposing order
   │            ↓
   │          trade
   │            ↓
   │          repeat
   │
   └── NO → Filled
```

so after multi level matching the flow converge to 
```yaml
                 Incoming Order
                       │
                       ▼
              ┌─────────────────┐
              │ Find best level  │
              └────────┬────────┘
                       │
                 Does it cross?
                  /          \
                NO            YES
                │              │
                ▼              ▼
              REST       execute_match()
                               │
                         remove if filled
                               │
                               ▼
                       quantity remaining?
                          /          \
                        YES           NO
                         │             │
                         └─── loop ───► Filled
```

### Refining architectural boundary
our engine can now execute T1 T2 T3.... trades, but `match(...)` returns `ONE` std::optional<Trade>. we need proper tracking, so we need a MatchResult to store the vector of Trades done.
```yaml
MatchingEngine
      │
      ▼
 MatchResult
      │
      ├── Execution events
      ├── P&L
      ├── Market data
      ├── Trade logging
      └── Simulator statistics
```

### current verification status of AegisML

| Component                 |      Status |
| ------------------------- | ----------: |
| MarketEvent validation    |           ✅ |
| Order state machine       |           ✅ |
| Order                     |           ✅ |
| OrderManager              |           ✅ |
| OrderBook                 |           ✅ |
| MatchingEngine            |     ✅ 15/15 |
| Multi-level matching      |           ✅ |
| FIFO price/time priority  |           ✅ |
| Incoming order resting    |           ✅ |
| MarketSimulator           |           ✅ |
| Deterministic generation  |           ✅ |
| Batch generation          |           ✅ |
| CMake / CTest integration |           ✅ |
| **Total**                 | **99/99 ✅** |

moreover, we have a coherent system
```yaml
                    AegisML
                       │
              MarketSimulator
                       │
                  MarketEvent
                       │
          ┌────────────┴────────────┐
          │                         │
     OrderManager              Market Data
          │
          │
      Order State
          │
          ▼
      MatchingEngine
          │
          ▼
       Trade[]
          │
          ▼
     [NEXT LAYER]
```

### Execution and Trade History Subsystems

right now `MatchingEngine` produces 
```yaml
struct Trade {
    OrderId incoming_order_id;
    OrderId resting_order_id;
    Price price;
    Quantity quantity;
};
```
but the trade is currently just returned to caller and disappears. we eventually need something like
```yaml
MatchingEngine
      │
      ├── Trade
      │
      ▼
ExecutionRecorder
      │
      ├── execution history
      ├── trade IDs
      ├── timestamps
      └── query/access layer
```
now an `Execution` is the persisted occurance of that trade, it should eventually carry the state as 
```yaml
Execution
├── execution_id
├── incoming_order_id
├── resting_order_id
├── symbol
├── price
├── quantity
└── timestamp
```
we will start with minimal execution and then find our way to complete suite. SO the latest Event processing pipeline sits well as 
```yaml
                    MarketEvent
                        │
                        ▼
                ┌──────────────┐
                │ TradingEngine │
                └───────┬──────┘
                        │
             ┌──────────┼──────────┐
             ▼          ▼          ▼
       OrderManager  OrderBook  MatchingEngine
             │          ▲          │
             │          │          ▼
             └──────────┴──────── Trade
                                  │
                                  ▼
                         ExecutionRecorder
                                  │
                                  ▼
                              Execution
```

### symbol scoped market state

we made the key architectural decision that EACH SYMBOL GETS ITS OWN MARKET STATE, and orderBook maintains a concrete class
```yaml
                         TradingEngine
                              │
                              ▼
                    MarketStateManager
                              │
              ┌───────────────┼───────────────┐
              ▼               ▼               ▼
            AAPL            GOOGL            MSFT
              │               │               │
         MarketState     MarketState     MarketState
          ┌────┴────┐     ┌────┴────┐     ┌────┴────┐
          │         │     │         │     │         │
     OrderBook  Matching OrderBook Matching ...    ...
```
so the ownership model translates to 
```yaml
TradingEngine
│
├── OrderManager
│      └── ALL orders
│
├── MarketStateManager
│      │
│      ├── AAPL → MarketState
│      │             ├── OrderBook
│      │             └── MatchingEngine
│      │
│      ├── GOOGL → MarketState
│      │             ├── OrderBook
│      │             └── MatchingEngine
│      │
│      └── MSFT → MarketState
│                    ├── OrderBook
│                    └── MatchingEngine
│
└── ExecutionRecorder
```

### architectural milestone

the current architecure evolves as
```yaml
TradingEngine
│
├── OrderManager
│
├── MarketStateManager
│     │
│     ├── AAPL → MarketState
│     │           ├── OrderBook
│     │           └── MatchingEngine
│     │
│     └── GOOGL → MarketState
│                 ├── OrderBook
│                 └── MatchingEngine
│
└── ExecutionRecorder
```

so now the StateManage rmaintains separation between books
```yaml
              MarketStateManager
                    │
          ┌─────────┴─────────┐
          ▼                   ▼
        AAPL                 GOOGL
          │                   │
          ▼                   ▼
      OrderBook A         OrderBook B
          │                   │
       SELL 50             BUY 100
       @10100              @10200
```

### Symbol-aware TradingEngine

currently TradingEngine owns OrderManager, OrderBook, MatchingEngine and ExecutionRecorder. but OrderBook and MatchingEngine are singular, so we change it ot MarketStateManagement taking care of symbols.
```yaml
TradingEngine
├── OrderManager
├── MarketStateManager
│     ├── AAPL  → MarketState
│     ├── GOOGL → MarketState
│     └── MSFT  → MarketState
└── ExecutionRecorder
```

now that there are tests for the same, the behavioral work is done, the architecture evolves to
```yaml
                         TradingEngine
                              │
             ┌────────────────┼────────────────┐
             ▼                ▼                ▼
       OrderManager    MarketStateManager   ExecutionRecorder
                              │
                  ┌───────────┼───────────┐
                  ▼           ▼           ▼
                AAPL        GOOGL        MSFT
                  │           │           │
             MarketState  MarketState  MarketState
              ┌───┴───┐    ┌───┴───┐
              ▼       ▼    ▼       ▼
         OrderBook MatchingEngine
```
all while OrderBook remains `concrete`, we currently dont have multiple orderbook implementations so we good. `MarketState` holds `OrderBook` and `MatchingEngine`, all while `OrderManager` is globally owned by `TradingEngine`. this allows global order lookup while keeping machine strictly symbol-scoped. `MarketStateManager` handles `synbol -> MarketState` mappings with lazy creation.

### Major mismatch, what happend to cancelled orders?
or target invariant is 
```yaml
OrderBook contains order
        ↓
Cancel requested
        ↓
Order → CancelPending
        ↓
OrderBook removes order
        ↓
Order → Cancelled
```
if removal fails, we must not blindly mark the order `Cancelled`. The current implementation is checked against tests that it fails if OrderManager cancels an order, it moves the state to CancelPending, the OrderManager then processes it and all while `order remains in OrderBook`. So now, cancellation correctly crosses the two ownership domains,
```yaml
CancelOrderEvent
       │
       ▼
TradingEngine
       │
       ├──────────────► OrderManager
       │                    │
       │                    ▼
       │              CancelPending
       │
       └──────────────► Symbol MarketState
                            │
                            ▼
                       OrderBook.remove()
```

### Event Dispatch Processor
right now `TradingEngine::process()` does several jobs
```yaml
MarketEvent
    │
    ▼
TradingEngine::process()
    │
    ├── structural validation
    ├── sequence validation
    ├── identify event type
    ├── route AddOrder
    ├── route CancelOrder
    ├── route other events
    ├── matching
    └── execution recording
```
this makes dependency around process(), we want to move the routing capacbilities to dispatcher. so the target architecture becomes
```yaml
                    MarketEvent
                         │
                         ▼
                ┌─────────────────┐
                │  TradingEngine  │
                │                 │
                │ validation      │
                │ sequencing      │
                └────────┬────────┘
                         │
                         ▼
                ┌─────────────────┐
                │ EventDispatcher │
                └────────┬────────┘
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
      AddOrder        CancelOrder    Other events
          │              │              │
          ▼              ▼              ▼
     OrderManager    OrderManager   OrderManager
          │
          ▼
    MatchingEngine
          │
          ▼
 ExecutionRecorder
 ```

 ### EventLog

 so the interpretation of EventLog is as follows, it Logs every structurally valid incoming event, even if the engine later rejects it semantically.
 ```yaml
 MarketDataSource
       │
       ▼
    Event
       │
       ├──────────► EventLog
       │
       ▼
 TradingEngine

seq 1 → valid Add      → accepted
seq 2 → duplicate Add → rejected
seq 3 → Trade          → accepted

LOG CONTAINS: 1,2,3

REPLAY PROCESS: 
      1 → success
      2 → failure
      3 → success
```

This gives a `record of what actually entered the system`. therefore

```yaml
                    MarketEvent
                         │
                         ▼
                    is_valid()
                         │
                  ┌──────┴──────┐
                false          true
                  │              │
                reject           ▼
                         sequence.accept()
                                │
                         ┌──────┴──────┐
                       false          true
                         │              │
                       reject           ▼
                                  EventLog.append()
                                        │
                                        ▼
                                semantic processing
                                        │
                              ┌─────────┴─────────┐
                           success             failure
                              │                    │
                              ▼                    ▼
                           true                  false
```
naturally TradingEngine owns EventLog

### Replay System and Deterministic replay
```yaml
                 Original run
MarketEvent ──► TradingEngine A
                    │
                    ├── Market State A
                    ├── Executions A
                    └── EventLog
                           │
                           ▼
                     ReplaySource
                           │
                           ▼
                    TradingEngine B
                           │
                           ├── Market State B
                           └── Executions B

             A == B
```

the seriallization should wire the format explicit and testable
```yaml
File
├── Header
│   ├── Magic       4 bytes
│   └── Version     uint32
│
└── Events
    └── Event
        ├── event_id         uint64
        ├── sequence_number  uint64
        ├── timestamp        int64
        ├── symbol           8 bytes
        ├── event_type       uint8
        └── payload
```
we will be using `little-endian` encoding, so that we could process from LSB to MSB