# AegisML Market Core --- 3-Phase Completion Plan

## Scope

This plan covers **only the Market Core**. The ML/research layer is
deliberately separated and has its own plan.

The Market Core is responsible for:

-   Market/order event domain
-   Order lifecycle
-   Order management
-   Price-time-priority order books
-   Matching
-   Symbol-scoped market state
-   Trading-engine orchestration
-   Execution recording
-   Event sequencing and processing
-   Clock/time semantics
-   Market-data source boundaries
-   Persistence/event log
-   Deterministic replay
-   Deterministic simulation

The Market Core does **not** include:

-   ML models
-   Feature engineering for ML
-   Strategy research
-   Alpha generation
-   Portfolio optimization
-   Risk models
-   Backtest analytics beyond what is required to validate the market
    engine

------------------------------------------------------------------------

# Current Baseline

## Completed

The current implementation has:

-   Strong domain types (`Price`, `Quantity`, `Timestamp`, IDs)
-   Fixed-size `Symbol`
-   `MarketEvent` and payload variants
-   Event validation
-   `Order` lifecycle/state machine
-   `OrderManager`
-   Price-time-priority `OrderBook`
-   `MatchingEngine`
-   `Execution`
-   `ExecutionRecorder`
-   Symbol-scoped `MarketState`
-   `MarketStateManager`
-   `TradingEngine`
-   Cancellation removal from the executable book
-   Multi-symbol isolation
-   Multi-level matching
-   Partial fills
-   Execution recording

## Verified Baseline

**121/121 tests passing.**

This is the baseline for the three remaining Market Core phases.

------------------------------------------------------------------------

# Phase 1 --- Market Core Correctness & Contract Hardening

## Goal

Make the existing market engine internally rigorous before adding
infrastructure around it.

The focus is not new functionality. The focus is eliminating ambiguity,
duplicated responsibility, and partial state mutation.

## 1. Matching Atomicity

Current matching can perform:

1.  Execute incoming order
2.  Execute resting order

as two separate mutations.

Potential failure:

``` text
Incoming order modified
Resting order unchanged
Trade rejected
```

Establish the invariant:

> A match either applies completely or does not produce an execution.

### Deliverables

-   Define matching transaction semantics.
-   Prevent partially applied matches.
-   Add failure-path tests.
-   Verify quantities/states remain unchanged when a match cannot
    complete.
-   Ensure generated `Trade` objects correspond exactly to committed
    state changes.

## 2. Matching Responsibility

Current order-resting responsibility is split between:

-   `TradingEngine`
-   `MatchingEngine::rest_if_needed()`

Resolve this duplication.

### Target contract

Prefer:

``` text
TradingEngine
    = event validation + orchestration

MatchingEngine
    = matching + resting remaining quantity
```

The final ownership must be explicit and tested.

## 3. Order Book Invariants

Formalize and test:

-   No duplicate `OrderId` in a book.
-   Every booked order has positive remaining quantity.
-   Every booked order is executable.
-   Filled orders are not present.
-   Cancelled/cancel-pending orders are not executable.
-   Best bid is the highest bid.
-   Best ask is the lowest ask.
-   FIFO is preserved within a price level.
-   Removing an order cleans up empty price levels.
-   Book membership and order lifecycle cannot silently diverge.

## 4. Matching Invariants

Add tests for:

-   No self-match.
-   No trade without price crossing.
-   Correct resting-price execution.
-   Correct partial-fill quantities.
-   Correct multi-level consumption.
-   Incoming remainder rests exactly once.
-   Fully filled incoming order does not rest.
-   Fully filled resting order leaves the book.
-   Cancelled orders never participate in matching.

## 5. Event Payload Semantics

Resolve currently ambiguous contracts, especially:

``` cpp
CancelOrderEvent {
    OrderId order_id;
    Quantity quantity;
};
```

Explicitly decide whether cancellation means:

-   whole-order cancellation,
-   partial cancellation,
-   or a quantity-bearing request with a defined interpretation.

Do not leave the field semantically undefined.

## 6. Cancellation Semantics

Preserve the established lifecycle:

``` text
Cancel request
    ↓
CancelPending
    ↓
removed from executable book
    ↓
cancellation confirmation
    ↓
Cancelled
```

Define what component/event is responsible for confirmation.

## Phase 1 Exit Criteria

-   Matching is atomic.
-   Resting ownership is unambiguous.
-   Order-book invariants are explicitly tested.
-   Matching invariants are explicitly tested.
-   Cancellation semantics are documented.
-   Existing behavior remains regression-safe.
-   Full test suite remains green.

Target:

``` text
Existing 121+ tests
+
new correctness/invariant tests
= 100% passing
```

------------------------------------------------------------------------

# Phase 2 --- Event, Time & Data-Flow Infrastructure

## Goal

Turn the working market engine into a deterministic event-driven core
that is independent of where events originate.

## 1. Event Protocol

Define the semantics of:

-   `EventId`
-   `SequenceNumber`
-   `Timestamp`
-   Event type
-   Event payload

Establish:

``` text
EventId
    = identity

SequenceNumber
    = ordering
```

Define behavior for:

-   duplicate sequence numbers
-   gaps
-   out-of-order events
-   duplicate event IDs
-   invalid timestamps
-   invalid event payloads

## 2. Event Dispatcher / Processing Pipeline

Introduce a minimal event-flow abstraction around the existing engine.

Target:

``` text
EventSource
    ↓
EventStream / Dispatcher
    ↓
TradingEngine
    ↓
Execution / State
```

Do not turn `TradingEngine` into a giant dispatcher.

## 3. Clock Abstraction

Introduce a logical clock boundary.

Required modes:

``` text
RealTimeClock
SimulationClock
ReplayClock
```

The engine should not depend directly on wall-clock time for
deterministic processing.

## 4. Market Data Source Boundary

Use the existing `MarketDataSource` abstraction as the source boundary.

Target:

``` text
MarketDataSource
    ├── HistoricalSource
    ├── SimulationSource
    └── LiveSource boundary
```

The trading engine should consume events without knowing their origin.

## 5. Event Ordering

Implement deterministic event processing.

For an event stream:

``` text
sequence 101
sequence 102
sequence 103
```

the engine must have defined behavior.

Make ordering rules explicit and test them.

## 6. Deterministic Simulation Foundation

Refactor the simulator around:

-   deterministic seed
-   logical clock
-   deterministic event generation
-   reproducible event ordering
-   reproducible execution output

### Phase 2 Exit Criteria

A given:

``` text
seed
+
configuration
+
initial state
```

must produce the same:

``` text
event sequence
+
execution sequence
+
final market state
```

on repeated runs.

------------------------------------------------------------------------

# Phase 3 --- Event Log, Replay & Market-Core Completion

## Goal

Complete the infrastructure required for a reusable, replayable market
engine.

## 1. Event Log

Introduce a logical event-log abstraction.

Responsibilities:

-   append events
-   preserve ordering
-   read events
-   support sequential replay
-   preserve required event metadata

Keep storage implementation separate from the engine.

## 2. Serialization

Define a stable representation for:

-   `MarketEvent`
-   event payloads
-   `Execution`
-   required state metadata

Serialization must preserve enough information for deterministic replay.

## 3. Replay Engine

Target:

``` text
Recorded Event Log
       ↓
Replay Source
       ↓
Clock
       ↓
TradingEngine
       ↓
Executions
```

Replay must not require special matching logic.

## 4. Deterministic Replay Verification

Given:

``` text
same initial state
+
same event log
```

verify:

``` text
same executions
same quantities
same order states
same final book
```

## 5. Market-State Snapshots

If required by replay/performance, define a snapshot boundary:

``` text
Snapshot
    +
Events after snapshot
    ↓
Recovered state
```

Do not prematurely build a database-backed persistence layer.

First establish the logical snapshot/recovery contract.

## 6. End-to-End Market Core Test

Build a complete deterministic scenario:

``` text
Market data
    ↓
Events
    ↓
Sequencer
    ↓
TradingEngine
    ↓
OrderBook / Matching
    ↓
Executions
    ↓
Event Log
    ↓
Replay
    ↓
Same final state
```

### Phase 3 Exit Criteria

The Market Core is considered complete when it can:

1.  Accept deterministic events.
2.  Validate and sequence them.
3.  Maintain symbol-isolated order books.
4.  Match orders correctly.
5.  Maintain correct order lifecycle.
6.  Generate executions.
7.  Record events/executions.
8.  Persist an event stream.
9.  Replay the event stream.
10. Reproduce the same final state and executions.
11. Run deterministic simulations.

------------------------------------------------------------------------

# Final Market Core Architecture

``` text
                       MARKET CORE
                            │
                ┌───────────┴───────────┐
                │                       │
          Event Sources             Replay Log
                │                       │
                └───────────┬───────────┘
                            ▼
                    Event / Clock Layer
                            │
                    Sequence / Validate
                            │
                            ▼
                     TradingEngine
                            │
          ┌─────────────────┼─────────────────┐
          │                 │                 │
          ▼                 ▼                 ▼
    OrderManager     MarketStateManager   ExecutionRecorder
                            │
                ┌───────────┼───────────┐
                ▼           ▼           ▼
              AAPL        GOOGL        ...
                │
         ┌──────┴──────┐
         ▼             ▼
     OrderBook    MatchingEngine
                            │
                            ▼
                       Executions
                            │
                            ▼
                       Event Log
                            │
                            ▼
                         Replay
```

------------------------------------------------------------------------

# Definition of Done

The Market Core is done when the system is a **deterministic
event-driven market simulator/replay engine**, not merely a collection
of order-management classes.

The final property we want is:

``` text
Same input events
      +
Same initial state
      ↓
Same executions
      +
Same final market state
```

Performance optimization comes **after** correctness and deterministic
replay are established.
