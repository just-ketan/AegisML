# AegisML

## Development Plan

AegisML is being developed as a high-performance C++ market simulation and machine-learning experimentation platform.

The core objective is to bridge the gap between:

**Market events → deterministic simulation → high-performance systems → feature generation → ML-driven intelligence**

The project will be developed in five deliberate phases. Each phase must leave the system in a working, testable state before the next phase begins.

---

# Development Philosophy

AegisML is not being built as a collection of isolated features.

Every phase should improve one or more of:

* Correctness
* Performance
* Memory efficiency
* Determinism
* Extensibility
* Observability
* ML readiness
* Production-quality C++ engineering

### Core principles

1. **Correctness before optimization**
2. **Measure before optimizing**
3. **Explicit ownership and lifetime**
4. **Deterministic simulation**
5. **Cache-friendly data structures**
6. **Minimal unnecessary allocation**
7. **Test every architectural change**
8. **Benchmark performance changes**
9. **Keep simulation and ML concerns decoupled**
10. **Document important design decisions**

---

# Phase Overview

| Phase       | Focus                          | Primary Outcome                                  |
| ----------- | ------------------------------ | ------------------------------------------------ |
| **Phase 1** | Core Simulation Engine         | Correct, deterministic market simulator          |
| **Phase 2** | High-Performance Systems       | Memory-efficient and cache-aware engine          |
| **Phase 3** | Market Realism & Data Pipeline | Realistic market data generation and datasets    |
| **Phase 4** | ML Integration                 | ML-ready features, models and inference pipeline |
| **Phase 5** | Production & Research Platform | Benchmarkable, extensible end-to-end system      |

---

# Phase 1 — Core Simulation Engine

## Objective

Establish a clean and deterministic C++ market simulation foundation.

The simulator should correctly represent:

* Market events
* Symbols
* Event types
* Market data sources
* Event processing
* Simulation time
* Basic market state

### Current architecture

```text
Market Data Source
        │
        ▼
   Market Event
        │
        ▼
Market Simulator
        │
        ▼
   Market State
```

## Development Goals

### 1. Event Model

Define a robust representation for market events such as:

* Trade
* Quote
* Order
* Cancel
* Modify
* Market open/close
* Other system events

The event representation should explicitly define:

* Timestamp
* Symbol
* Event type
* Relevant numerical fields
* Ownership/lifetime semantics

### 2. Symbol Representation

Establish a well-defined `Symbol` abstraction.

Questions to resolve:

* Is a symbol stored as a `std::string`?
* Is it interned?
* Can it be represented using an integer ID?
* Does it require `.data()` / string-like access?
* What are its copy/move semantics?

### 3. Simulator

Implement deterministic event processing.

The simulator should support:

```text
initialize()
     ↓
process(event)
     ↓
update state
     ↓
advance simulation
```

### 4. Testing

Establish unit tests for:

* Event construction
* Event fields
* Symbol behavior
* Event ordering
* Simulator state transitions
* Edge cases

### 5. Build System

Maintain reproducible builds using:

* CMake
* Make
* GoogleTest

## Phase 1 Exit Criteria

* [ ] Core classes have clearly defined responsibilities
* [ ] Simulator is deterministic
* [ ] Event processing is correct
* [ ] Ownership/lifetime is explicit
* [ ] Unit tests cover core behavior
* [ ] CMake build works cleanly
* [ ] No known memory errors
* [ ] Design decisions are documented

---

# Phase 2 — High-Performance Systems

## Objective

Transform the correct simulator into a performance-oriented C++ systems implementation.

This phase is where AegisML becomes a serious systems project rather than simply a functional simulator.

## Focus Areas

### 1. Memory Layout

Investigate:

* `sizeof`
* Alignment
* Padding
* Cache-line behavior
* Struct layout
* AoS vs SoA
* Hot vs cold data

Example:

```cpp
struct MarketEvent {
    uint64_t timestamp;
    uint32_t symbol;
    uint32_t price;
    uint32_t quantity;
};
```

The actual memory layout must be measured rather than assumed.

### 2. Allocation Strategy

Minimize unnecessary dynamic allocation.

Investigate:

* Stack allocation
* `std::vector`
* Object pools
* Arenas
* Ring buffers
* Preallocation
* Small-buffer techniques

### 3. Cache Efficiency

Study:

```text
CPU
 │
 ├── Registers
 ├── L1
 ├── L2
 ├── L3
 └── RAM
```

The simulator should favor predictable, contiguous access patterns.

### 4. Data-Oriented Design

Evaluate whether critical structures should transition from:

```text
Array of Structures
```

to:

```text
Structure of Arrays
```

when workloads justify it.

### 5. Benchmarking

Introduce repeatable benchmarks measuring:

* Events/second
* Latency/event
* Memory usage
* Allocation count
* Cache behavior
* Scaling behavior

### 6. Memory Correctness

Use tooling such as:

* AddressSanitizer
* UndefinedBehaviorSanitizer
* Valgrind
* Compiler warnings

## Phase 2 Exit Criteria

* [ ] Hot paths identified
* [ ] Baseline benchmarks recorded
* [ ] Memory layout documented
* [ ] Allocation behavior measured
* [ ] Critical paths optimized
* [ ] Sanitizers pass
* [ ] Performance improvements backed by measurements
* [ ] No optimization without benchmark evidence

---

# Phase 3 — Market Realism & Data Pipeline

## Objective

Turn the simulator into a realistic market-data generation platform.

The simulator should produce structured datasets suitable for analysis and machine learning.

## Market Components

Introduce progressively richer behavior:

```text
Orders
  ↓
Order Book
  ↓
Matching Engine
  ↓
Trades
  ↓
Quotes
  ↓
Market State
```

Potential components:

* Limit order book
* Market orders
* Limit orders
* Cancellations
* Order modifications
* Matching
* Spread
* Bid/ask dynamics
* Market depth
* Order-flow imbalance

## Data Generation

Generate event streams containing:

```text
timestamp
symbol
event_type
price
quantity
side
bid
ask
spread
depth
volume
```

## Feature Engineering

Build features such as:

* Returns
* Mid-price
* Spread
* Volume
* Volatility
* Order-flow imbalance
* Book imbalance
* Trade intensity
* Price momentum
* Short-term liquidity

## Dataset Pipeline

```text
Simulator
    │
    ▼
Raw Events
    │
    ▼
Feature Generator
    │
    ▼
Dataset
    │
    ├── Training
    ├── Validation
    └── Testing
```

## Reproducibility

Every generated dataset should be reproducible using:

* Random seed
* Configuration
* Simulation parameters
* Version information

## Phase 3 Exit Criteria

* [ ] Order-book model exists
* [ ] Matching engine works
* [ ] Realistic event stream generated
* [ ] Feature pipeline implemented
* [ ] Dataset format defined
* [ ] Seeds produce reproducible datasets
* [ ] Dataset validation exists
* [ ] Simulation configuration is externalized

---

# Phase 4 — ML Integration

## Objective

Connect the simulation and data pipeline to machine-learning experimentation without polluting the core simulator with ML-specific logic.

Architecture:

```text
                ┌──────────────────┐
                │ Market Simulator │
                └────────┬─────────┘
                         │
                         ▼
                  Event / State Data
                         │
                         ▼
                ┌──────────────────┐
                │ Feature Pipeline │
                └────────┬─────────┘
                         │
                         ▼
                ┌──────────────────┐
                │ ML Dataset       │
                └────────┬─────────┘
                         │
                         ▼
                ┌──────────────────┐
                │ Training         │
                └────────┬─────────┘
                         │
                         ▼
                ┌──────────────────┐
                │ Model             │
                └────────┬─────────┘
                         │
                         ▼
                ┌──────────────────┐
                │ Inference        │
                └──────────────────┘
```

## ML Tasks

Potential tasks include:

### Prediction

* Short-term price movement
* Volatility
* Spread movement
* Liquidity changes

### Classification

```text
BUY
SELL
HOLD
```

or:

```text
UP
DOWN
STABLE
```

### Anomaly Detection

Detect:

* Unusual order flow
* Abnormal volume
* Liquidity shocks
* Market regime changes

## Python Integration

Python should primarily handle:

* Dataset processing
* Model development
* Experimentation
* Visualization
* Evaluation

C++ should remain responsible for:

* Simulation
* Event processing
* Performance-critical computation
* Market mechanics

## Model Evaluation

Track:

* Accuracy
* Precision
* Recall
* F1
* ROC-AUC where appropriate
* Prediction latency
* Throughput
* Dataset leakage

## Phase 4 Exit Criteria

* [ ] C++ simulator produces ML-ready datasets
* [ ] Python pipeline consumes datasets
* [ ] At least one baseline ML model exists
* [ ] Training pipeline is reproducible
* [ ] Evaluation methodology is documented
* [ ] Inference path exists
* [ ] C++/Python boundary is clearly defined

---

# Phase 5 — Production & Research Platform

## Objective

Transform AegisML into a polished research and systems platform.

The final system should support:

```text
Configuration
     │
     ▼
Simulation
     │
     ├──────────────┐
     ▼              ▼
Dataset         Benchmark
     │              │
     ▼              ▼
ML Training     Performance
     │
     ▼
Inference
     │
     ▼
Evaluation
     │
     ▼
Experiment Results
```

## Production Engineering

Introduce:

* Configuration management
* Logging
* Metrics
* CLI
* Error handling
* Serialization
* Versioning
* Experiment tracking

## Performance Engineering

Build a standardized benchmark suite.

Example:

```text
Benchmark
├── Event throughput
├── Order-book throughput
├── Matching latency
├── Feature generation
├── Dataset generation
├── Memory usage
└── ML inference latency
```

## Concurrency

Evaluate:

* Producer/consumer architecture
* Lock-free structures
* Thread pools
* Parallel simulation
* Batch processing
* Multi-symbol simulation

Only introduce concurrency where profiling demonstrates a meaningful benefit.

## Experiment Framework

Every experiment should record:

```text
Experiment ID
Git commit
Configuration
Random seed
Dataset version
Model version
Metrics
Runtime
Hardware
Results
```

This makes results scientifically reproducible.

## Documentation

Final documentation should include:

* Architecture
* API
* Data model
* Performance results
* ML methodology
* Benchmark methodology
* Design decisions
* Reproducibility instructions

## Phase 5 Exit Criteria

* [ ] End-to-end pipeline works
* [ ] CLI available
* [ ] Configuration system exists
* [ ] Benchmark suite exists
* [ ] Experiment tracking exists
* [ ] Reproducible experiments work
* [ ] Performance characteristics documented
* [ ] Architecture documentation complete
* [ ] Project can be built from a clean environment
* [ ] Project is portfolio/interview ready

---

# Final Architecture

The intended long-term architecture is:

```text
                         ┌────────────────────┐
                         │   Configuration    │
                         └─────────┬──────────┘
                                   │
                                   ▼
┌────────────────────────────────────────────────────────┐
│                  C++ Simulation Engine                  │
│                                                        │
│  Market Data → Event Engine → Order Book → Matching   │
│                              │                         │
│                              ▼                         │
│                         Market State                   │
└──────────────────────────────┬─────────────────────────┘
                               │
                               ▼
                     ┌──────────────────┐
                     │ Feature Pipeline │
                     └────────┬─────────┘
                              │
                              ▼
                     ┌──────────────────┐
                     │ Dataset Storage  │
                     └────────┬─────────┘
                              │
                ┌─────────────┴─────────────┐
                ▼                           ▼
        ┌──────────────┐            ┌──────────────┐
        │ ML Training  │            │ Benchmarking │
        └──────┬───────┘            └──────────────┘
               │
               ▼
        ┌──────────────┐
        │ Model Store  │
        └──────┬───────┘
               │
               ▼
        ┌──────────────┐
        │  Inference  │
        └──────────────┘
```

---

# Development Rule

**Do not jump phases because a feature looks interesting.**

A phase is considered complete only when its exit criteria are satisfied.

In particular:

> **Phase 2 optimization must be driven by measurements from Phase 1.**

> **Phase 4 ML must be driven by datasets produced by Phase 3.**

> **Phase 5 must integrate proven components rather than introduce uncontrolled complexity.**

---

# Immediate Next Step

Before implementing anything further:

1. Review the existing Phase 1 codebase.
2. Identify the current architecture.
3. Identify incomplete or questionable abstractions.
4. Establish the Phase 1 baseline.
5. Establish correctness tests.
6. Establish a performance baseline.
7. Only then begin Phase 2.

**Current status: Phase 1 — Core Simulation Engine**

The immediate goal is **not to add more features**.

The immediate goal is to understand and solidify what already exists.
