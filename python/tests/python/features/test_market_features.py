"""Tests for trade-level market feature engineering."""

import pandas as pd
import pytest

from aegisml.data.event_frame import events_to_frame
from aegisml.data.schemas import (
    EventType,
    MarketEventRecord,
    QuotePayload,
    TradePayload,
)
from aegisml.features.market_features import build_trade_features


def make_trade(
    event_id: int,
    symbol: str,
    timestamp_ms: int,
    price: int,
    quantity: int,
) -> MarketEventRecord:
    """Create a trade event for feature-engineering tests."""
    return MarketEventRecord(
        event_id=event_id,
        sequence_number=event_id,
        timestamp_ms=timestamp_ms,
        symbol=symbol,
        event_type=EventType.TRADE,
        payload=TradePayload(price=price, quantity=quantity),
    )


def test_calculates_trade_returns():
    """Calculate returns from consecutive trades."""
    events = events_to_frame([
        make_trade(1, "AAPL", 1000, 100, 10),
        make_trade(2, "AAPL", 2000, 110, 20),
        make_trade(3, "AAPL", 3000, 99, 30),
    ])

    features = build_trade_features(events, window=2)

    assert len(features) == 3
    assert pd.isna(features.loc[0, "return_1"])
    assert features.loc[1, "return_1"] == pytest.approx(0.10)
    assert features.loc[2, "return_1"] == pytest.approx(-0.10)


def test_calculates_rolling_features():
    """Calculate rolling returns, volatility, and average trade quantity."""
    events = events_to_frame([
        make_trade(1, "AAPL", 1000, 100, 10),
        make_trade(2, "AAPL", 2000, 110, 20),
        make_trade(3, "AAPL", 3000, 121, 30),
    ])

    features = build_trade_features(events, window=2)

    assert features.loc[2, "rolling_mean_return"] == pytest.approx(0.10)
    assert features.loc[2, "rolling_volatility"] == pytest.approx(0.0)
    assert features.loc[2, "rolling_mean_volume"] == pytest.approx(25.0)


def test_calculates_features_per_symbol():
    """Prevent one symbol's prices from influencing another symbol's returns."""
    events = events_to_frame([
        make_trade(1, "AAPL", 1000, 100, 10),
        make_trade(2, "MSFT", 1500, 200, 20),
        make_trade(3, "AAPL", 2000, 110, 30),
        make_trade(4, "MSFT", 2500, 180, 40),
    ])

    features = build_trade_features(events, window=2)
    aapl = features.loc[features["symbol"] == "AAPL"].reset_index(drop=True)
    msft = features.loc[features["symbol"] == "MSFT"].reset_index(drop=True)

    assert aapl.loc[1, "return_1"] == pytest.approx(0.10)
    assert msft.loc[1, "return_1"] == pytest.approx(-0.10)


def test_ignores_non_trade_events():
    """Keep quote events out of the initial trade-only feature pipeline."""
    trade = make_trade(1, "AAPL", 1000, 100, 10)
    quote = MarketEventRecord(
        event_id=2,
        sequence_number=2,
        timestamp_ms=2000,
        symbol="AAPL",
        event_type=EventType.QUOTE,
        payload=QuotePayload(
            bid_price=99,
            bid_quantity=100,
            ask_price=101,
            ask_quantity=100,
        ),
    )

    features = build_trade_features(events_to_frame([trade, quote]))

    assert len(features) == 1
    assert features.loc[0, "event_id"] == 1


def test_rejects_invalid_window():
    """Require a window of at least two observations."""
    with pytest.raises(ValueError, match="at least 2"):
        build_trade_features(pd.DataFrame(), window=1)
