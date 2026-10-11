
"""Tests for chronological dataset splitting."""

import pandas as pd
import pytest

from aegisml.datasets.splitting import chronological_split


def make_dataset(size: int = 100) -> pd.DataFrame:
    """Create a dataset with one observation per timestamp."""
    return pd.DataFrame(
        {
            "timestamp_ms": range(size),
            "sequence_number": range(size),
            "feature": range(size),
            "target": [index % 2 for index in range(size)],
        }
    )


def test_creates_three_chronological_splits():
    """Use earlier observations for training and later ones for testing."""
    train, validation, test = chronological_split(make_dataset())

    assert len(train) == 70
    assert len(validation) == 15
    assert len(test) == 15

    assert train["timestamp_ms"].max() < validation["timestamp_ms"].min()
    assert (
        validation["timestamp_ms"].max()
        < test["timestamp_ms"].min()
    )


def test_keeps_equal_timestamps_in_one_split():
    """Never distribute simultaneous observations across different splits."""
    dataset = pd.DataFrame(
        {
            "timestamp_ms": [1, 2, 3, 4, 5, 6],
            "sequence_number": [1, 2, 3, 4, 5, 6],
            "feature": [10, 20, 30, 40, 50, 60],
        }
    )

    # Duplicate timestamps at potential split boundaries.
    duplicates = pd.DataFrame(
        {
            "timestamp_ms": [2, 5],
            "sequence_number": [7, 8],
            "feature": [21, 51],
        }
    )
    dataset = pd.concat([dataset, duplicates], ignore_index=True)

    train, validation, test = chronological_split(
        dataset,
        train_fraction=0.5,
        validation_fraction=0.25,
    )

    train_times = set(train["timestamp_ms"])
    validation_times = set(validation["timestamp_ms"])
    test_times = set(test["timestamp_ms"])

    assert train_times.isdisjoint(validation_times)
    assert train_times.isdisjoint(test_times)
    assert validation_times.isdisjoint(test_times)


@pytest.mark.parametrize(
    ("train_fraction", "validation_fraction"),
    [
        (0.0, 0.15),
        (1.0, 0.15),
        (0.70, 0.0),
        (0.70, 1.0),
        (0.80, 0.20),
    ],
)
def test_rejects_invalid_fractions(
    train_fraction: float,
    validation_fraction: float,
):
    """Reject fractions that cannot produce three valid splits."""
    with pytest.raises(ValueError):
        chronological_split(
            make_dataset(),
            train_fraction=train_fraction,
            validation_fraction=validation_fraction,
        )


def test_rejects_missing_timestamps():
    """Require valid timestamps for temporal splitting."""
    dataset = make_dataset()
    dataset.loc[0, "timestamp_ms"] = None

    with pytest.raises(ValueError, match="cannot contain missing values"):
        chronological_split(dataset)


def test_rejects_too_few_unique_timestamps():
    """Require enough distinct times to form three non-empty splits."""
    dataset = pd.DataFrame(
        {
            "timestamp_ms": [1, 1, 1],
            "feature": [10, 20, 30],
        }
    )

    with pytest.raises(ValueError, match="unique timestamps"):
        chronological_split(dataset)