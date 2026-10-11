
"""Tests for supervised dataset construction."""

import pandas as pd
import pytest

from aegisml.datasets.supervised import (
    MODEL_FEATURE_COLUMNS,
    build_supervised_dataset,
)


def make_features(
    symbol: str,
    prices: list[int],
    start_event_id: int = 1,
    start_timestamp: int = 1000,
) -> pd.DataFrame:
    """Create valid synthetic feature rows for dataset tests."""
    rows = []

    for index, price in enumerate(prices):
        row = {
            "event_id": start_event_id + index,
            "sequence_number": start_event_id + index,
            "timestamp_ms": start_timestamp + index * 1000,
            "symbol": symbol,
            "price": price,
            "quantity": 10.0,
            "return_1": 0.01,
            "rolling_mean_return": 0.01,
            "rolling_volatility": 0.001,
            "rolling_mean_volume": 10.0,
        }
        rows.append(row)

    return pd.DataFrame(rows)


def test_builds_next_trade_direction_targets():
    """Label price increases as 1 and other movements as 0."""
    features = make_features("AAPL", [100, 110, 105, 105])

    dataset = build_supervised_dataset(features)

    assert dataset["target"].tolist() == [1, 0, 0]
    assert dataset["price"].tolist() == [100, 110, 105]


def test_does_not_create_labels_across_symbols():
    """Never use another symbol's price to label a trade."""
    aapl = make_features(
        "AAPL",
        [100, 110],
        start_event_id=1,
        start_timestamp=1000,
    )
    msft = make_features(
        "MSFT",
        [200, 180],
        start_event_id=3,
        start_timestamp=3000,
    )

    dataset = build_supervised_dataset(
        pd.concat([aapl, msft], ignore_index=True)
    )

    assert dataset["symbol"].tolist() == ["AAPL", "MSFT"]
    assert dataset["target"].tolist() == [1, 0]


def test_drops_rows_with_missing_features():
    """Remove rows whose engineered features are not model-ready."""
    features = make_features("AAPL", [100, 110, 120, 130])
    features.loc[1, "rolling_volatility"] = float("nan")

    dataset = build_supervised_dataset(features)

    assert 2 not in dataset["event_id"].tolist()
    assert dataset["target"].notna().all()


def test_drops_last_trade_for_each_symbol():
    """Exclude observations that have no subsequent trade."""
    features = make_features("AAPL", [100, 110, 120])

    dataset = build_supervised_dataset(features)

    assert dataset["event_id"].tolist() == [1, 2]
    assert 3 not in dataset["event_id"].tolist()


def test_rejects_missing_required_columns():
    """Fail clearly when the input schema is incomplete."""
    with pytest.raises(ValueError, match="Missing required feature columns"):
        build_supervised_dataset(pd.DataFrame({"price": [100]}))


def test_empty_input_returns_expected_schema():
    """Return a correctly structured empty dataset."""
    features = pd.DataFrame(
        columns=[
            "event_id",
            "sequence_number",
            "timestamp_ms",
            "symbol",
            *MODEL_FEATURE_COLUMNS,
        ]
    )

    dataset = build_supervised_dataset(features)

    assert dataset.empty
    assert dataset.columns.tolist() == [
        "event_id",
        "sequence_number",
        "timestamp_ms",
        "symbol",
        *MODEL_FEATURE_COLUMNS,
        "target",
    ]