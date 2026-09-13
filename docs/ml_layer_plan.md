# AegisML ML Layer --- 3-Phase Development Plan

## Scope

This plan is intentionally **separate from the Market Core**.

The ML Layer consumes market information from the completed Market Core
but does not own:

-   OrderBook implementation
-   Matching
-   Order lifecycle
-   Event sequencing
-   Exchange simulation
-   Market-data ingestion
-   Execution mechanics

The boundary is:

``` text
                    MARKET CORE
                         │
                         │ market events
                         │ market state
                         │ executions
                         ▼
                  ┌───────────────┐
                  │   ML LAYER    │
                  └───────────────┘
```

The ML Layer is responsible for:

-   market analytics
-   feature engineering
-   datasets
-   model training
-   inference
-   signal generation
-   strategy research
-   ML evaluation

It does **not** modify the fundamental Market Core to accommodate
individual models.

------------------------------------------------------------------------

# Starting Point

The ML layer should begin only after the Market Core provides a stable
event/state interface.

The Market Core should expose sufficient information to construct:

``` text
Market Events
Market State
Executions
Timestamps
Symbols
Order-book information
```

without exposing internal implementation details unnecessarily.

------------------------------------------------------------------------

# Phase 1 --- Market Intelligence & Feature Layer

## Goal

Convert Market Core output into a reproducible research representation.

## 1. Market Analytics

Build market-level measurements such as:

-   mid price
-   bid/ask spread
-   traded volume
-   trade count
-   VWAP
-   price returns
-   rolling volatility
-   order-book depth
-   order-flow measures

These are **analytics/features**, not part of the matching engine.

## 2. Feature Definitions

Create an explicit feature specification.

Each feature should define:

``` text
name
input fields
window
sampling frequency
formula
normalization
missing-data behavior
look-ahead rules
```

Avoid hidden feature calculations inside model code.

## 3. Time Windows

Support deterministic windows such as:

``` text
1 tick
5 ticks
10 ticks
1 second
5 seconds
1 minute
```

depending on the available data resolution.

The exact windows should be driven by research requirements rather than
hardcoded globally.

## 4. Dataset Generation

Build a reproducible pipeline:

``` text
Market Core Events
        ↓
State Reconstruction
        ↓
Feature Extraction
        ↓
Labels
        ↓
Dataset
```

Dataset generation must be deterministic.

## 5. Data Leakage Protection

Explicitly prevent:

-   future information entering features
-   labels leaking into inputs
-   future order-book state being used accidentally
-   train/test temporal contamination

## Phase 1 Exit Criteria

Given the same Market Core event stream:

``` text
same events
    ↓
same features
    ↓
same labels
    ↓
same dataset
```

The feature pipeline must be independently testable.

------------------------------------------------------------------------

# Phase 2 --- ML Models & Prediction Interface

## Goal

Create a clean boundary between market features and models.

Target architecture:

``` text
Feature Pipeline
       │
       ▼
 Feature Vector
       │
       ▼
 ┌──────────────┐
 │ Model API    │
 └──────┬───────┘
        │
        ▼
 Prediction
```

## 1. Model Interface

Define a model boundary that supports:

-   training
-   inference
-   model metadata
-   versioning
-   serialization
-   deterministic evaluation

The Market Core must not depend on a particular ML framework.

## 2. Baseline Models

Start with interpretable baselines before sophisticated models.

Examples:

-   linear/logistic models
-   tree-based models
-   simple time-series baselines

The purpose is to establish a performance floor and validate the feature
pipeline.

## 3. Prediction Targets

Define targets explicitly.

Examples may include:

``` text
future return
price direction
mid-price movement
volatility regime
liquidity state
```

Each target must specify:

``` text
prediction horizon
label construction
sampling frequency
```

## 4. Training Pipeline

Build reproducible training:

``` text
Dataset
   ↓
Train / Validation / Test
   ↓
Model
   ↓
Metrics
   ↓
Model Artifact
```

Record:

-   dataset version
-   feature version
-   model configuration
-   training period
-   validation period
-   test period
-   random seed
-   metrics

## 5. Evaluation

Evaluate both statistical and practical performance.

At minimum:

-   appropriate classification/regression metrics
-   temporal validation
-   baseline comparison
-   robustness across market regimes

Do not interpret predictive accuracy as trading profitability.

## Phase 2 Exit Criteria

A model can be:

``` text
trained
versioned
serialized
loaded
evaluated
run deterministically
```

through a stable ML interface.

------------------------------------------------------------------------

# Phase 3 --- Strategy Research & Market-Core Integration

## Goal

Connect predictions to simulated decisions without contaminating the
Market Core with ML-specific logic.

Target:

``` text
Market Core
     │
     ▼
Features
     │
     ▼
Model
     │
     ▼
Prediction
     │
     ▼
Strategy
     │
     ▼
Order Intent
     │
     ▼
Market Core
```

## 1. Strategy Boundary

Define:

``` text
Prediction
    ↓
Strategy
    ↓
Order Intent
```

The strategy decides:

-   whether to act
-   direction
-   desired quantity
-   price logic
-   cancellation decisions

The Market Core decides:

-   whether the order is valid
-   how it enters the book
-   how it matches
-   what execution occurs

## 2. Order Intent

Introduce an ML-side representation such as:

``` text
OrderIntent
```

rather than allowing model code to manipulate `OrderBook` directly.

This maintains architectural isolation.

## 3. Simulation Integration

Connect strategy-generated intents to the Market Core:

``` text
Feature
   ↓
Model
   ↓
Prediction
   ↓
Strategy
   ↓
OrderIntent
   ↓
Market Core
   ↓
Execution
```

## 4. Backtest Evaluation

Build a research evaluation layer around the completed Market Core.

Evaluate:

-   P&L
-   returns
-   drawdown
-   turnover
-   trade count
-   execution quality
-   slippage
-   transaction costs
-   latency assumptions

These metrics belong in the ML/research layer, not inside the matching
engine.

## 5. Experiment Reproducibility

Every experiment should identify:

``` text
Market event dataset
Feature version
Label version
Model version
Strategy version
Configuration
Random seed
Market Core version
```

This allows:

``` text
Experiment A
     ↓
exactly reproduce Experiment A
```

## Phase 3 Exit Criteria

The ML layer can:

1.  Consume Market Core output.
2.  Generate reproducible features.
3.  Train and evaluate models.
4.  Produce predictions.
5.  Convert predictions into strategy decisions.
6.  Generate order intents.
7.  Feed intents into the Market Core.
8.  Evaluate resulting executions and P&L.
9.  Reproduce experiments from versioned inputs/configuration.

------------------------------------------------------------------------

# Final ML Architecture

``` text
                         ML / RESEARCH LAYER
                                  │
                 ┌────────────────┴────────────────┐
                 │                                 │
           Feature Pipeline                   Dataset Store
                 │                                 │
                 ▼                                 │
             Features                             │
                 │                                 │
                 ▼                                 │
              Models ◄────────────────────────────┘
                 │
                 ▼
             Prediction
                 │
                 ▼
              Strategy
                 │
                 ▼
            Order Intent
                 │
                 ▼
          ┌────────────────┐
          │  MARKET CORE   │
          └───────┬────────┘
                  │
             Executions
                  │
                  ▼
           Research Metrics
                  │
                  ▼
             Experiments
```

------------------------------------------------------------------------

# Architectural Rule

The dependency direction should remain:

``` text
ML Layer
    ↓
Market Core interface
```

and never:

``` text
Market Core
    ↓
Specific ML model
```

The Market Core must remain useful even if the ML layer is completely
removed.

Likewise, the ML layer should be able to swap models without rewriting
the matching engine.

------------------------------------------------------------------------

# Definition of Done

The ML Layer is complete when it forms a reproducible research loop:

``` text
Market Events
     ↓
Features
     ↓
Model
     ↓
Prediction
     ↓
Strategy
     ↓
Order Intent
     ↓
Market Core
     ↓
Executions
     ↓
Evaluation
     ↓
Experiment
```

The ML layer should remain independently testable at every boundary.
