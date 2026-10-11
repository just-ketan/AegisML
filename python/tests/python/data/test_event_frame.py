"""Tests for converting decoded market events into DataFrames."""

import pandas as pd

from aegisml.data.event_frame import events_to_frame
from aegisml.data.schemas import (
    AddOrderPayload,
    EventType,
    MarketEventRecord,
    QuotePayload,
    Side,
    TradePayload,
)


def test_converts_trade_to_frame():
    """Preserve trade values and common event metadata."""
    event = MarketEventRecord(
        event_id=1,
        sequence_number=10,
        timestamp_ms=123456,
        symbol="AAPL",
        event_type=EventType.TRADE,
        payload=TradePayload(price=5_000_000_000, quantity=250),
    )

    frame = events_to_frame([event])

    assert len(frame) == 1
    assert frame.loc[0, "symbol"] == "AAPL"
    assert frame.loc[0, "event_type"] == "TRADE"
    assert frame.loc[0, "price"] == 5_000_000_000
    assert frame.loc[0, "quantity"] == 250
    assert str(frame["price"].dtype) == "Int64"


def test_converts_quote_fields():
    """Keep bid and ask fields separate."""
    event = MarketEventRecord(
        event_id=2,
        sequence_number=11,
        timestamp_ms=123457,
        symbol="AAPL",
        event_type=EventType.QUOTE,
        payload=QuotePayload(
            bid_price=14900,
            bid_quantity=100,
            ask_price=15100,
            ask_quantity=200,
        ),
    )

    frame = events_to_frame([event])

    assert frame.loc[0, "bid_price"] == 14900
    assert frame.loc[0, "ask_price"] == 15100
    assert pd.isna(frame.loc[0, "price"])


def test_converts_add_order():
    """Preserve order identity, side, price, and quantity."""
    event = MarketEventRecord(
        event_id=3,
        sequence_number=12,
        timestamp_ms=123458,
        symbol="AAPL",
        event_type=EventType.ADD,
        payload=AddOrderPayload(
            order_id=99,
            side=Side.BUY,
            price=15000,
            quantity=50,
        ),
    )

    frame = events_to_frame([event])

    assert frame.loc[0, "order_id"] == 99
    assert frame.loc[0, "side"] == "BUY"
    assert frame.loc[0, "price"] == 15000
    assert frame.loc[0, "quantity"] == 50


def test_empty_input_produces_empty_frame():
    """Return a DataFrame with the expected schema for no events."""
    frame = events_to_frame([])

    assert frame.empty
    assert "timestamp_ms" in frame.columns
    assert str(frame["timestamp_ms"].dtype) == "Int64"
