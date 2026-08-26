# The running design discussions

## CPP domain

we have `MarketEvent` as the first c++ domain object. what should be the definition of `MarketEvent` ?

- we need some interface that takes in the MarketEvent struct and then we can have multiple sources that can process MarketEvent later.

```yaml
                    ┌─────────────────────┐
                    │    MarketEvent      │
                    │       struct        │
                    │─────────────────────│
                    │ event_id            │
                    │ sequence_number     │
                    │ timestamp           │
                    │ symbol              │
                    │ event_type          │
                    │ price               │
                    │ quantity            │
                    └──────────┬──────────┘
                               │
                         data contract
                               │
              ┌────────────────┴────────────────┐
              │                                 │
              ▼                                 ▼
    ┌───────────────────┐             ┌───────────────────┐
    │ IMarketDataSource │             │  Future source    │
    │     interface     │             │                   │
    └─────────┬─────────┘             │ RealFeedSource    │
              │                       │ ReplaySource      │
              │                       │ FileSource        │
              │                       └───────────────────┘
              ▼
    ┌───────────────────┐
    │ MarketSimulator   │
    │                   │
    │ next_event()      │
    │ reset()           │
    │ has_next()        │
    └───────────────────┘
```

AegisML should depend on `IMarketDataSource` rather than `MarketSimulator` so later it becomes
```yaml
MarketSimulator ──────┐
                      │
ReplaySource ─────────┼──► IMarketDataSource
                      │
LiveFeedSource ───────┘
```
the consumer dont care about where event comes from. this serves as `ABSTRACTION BOUNDARY` we want.

### Optional<T> for MarketEvent ??

the absence of value is a legitimate part of API's result. for ex: `std::optional<MarketEvent> next_event()` has two outcomes
```yaml
              next_event()
                  │
          ┌───────┴───────┐
          ▼               ▼
    MarketEvent         nothing
```
this is semantically beautiful, but that does not make this a beautiful API.