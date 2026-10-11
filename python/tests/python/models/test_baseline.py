"""Tests for baseline model training and evaluation."""

import numpy as np
import pandas as pd
import pytest
from sklearn.pipeline import Pipeline

from aegisml.datasets.supervised import MODEL_FEATURE_COLUMNS
from aegisml.models.baseline import (
    evaluate_classifier,
    train_baseline,
)


def make_dataset(size: int = 100) -> pd.DataFrame:
    """Create a reproducible dataset with learnable binary targets."""
    rng = np.random.default_rng(42)

    returns = rng.normal(0.0, 0.02, size)
    quantities = rng.integers(1, 100, size)

    # Create a target with a learnable relationship to the features.
    targets = (returns > 0).astype(int)

    return pd.DataFrame(
        {
            "price": 100 + np.cumsum(returns),
            "quantity": quantities,
            "return_1": returns,
            "rolling_mean_return": returns,
            "rolling_volatility": np.abs(returns),
            "rolling_mean_volume": quantities.astype(float),
            "target": targets,
        }
    )


def test_trains_logistic_regression_pipeline():
    """Return a fitted scaler and logistic regression classifier."""
    dataset = make_dataset()
    model = train_baseline(dataset)

    assert isinstance(model, Pipeline)
    assert model.named_steps["scaler"] is not None
    assert model.named_steps["classifier"] is not None

    predictions = model.predict(dataset[MODEL_FEATURE_COLUMNS])

    assert len(predictions) == len(dataset)
    assert set(predictions).issubset({0, 1})


def test_evaluates_classifier():
    """Return the expected evaluation metrics."""
    dataset = make_dataset()
    model = train_baseline(dataset)

    metrics = evaluate_classifier(model, dataset)

    assert metrics["samples"] == len(dataset)
    assert 0.0 <= metrics["accuracy"] <= 1.0
    assert 0.0 <= metrics["balanced_accuracy"] <= 1.0
    assert 0.0 <= metrics["precision"] <= 1.0
    assert 0.0 <= metrics["recall"] <= 1.0
    assert 0.0 <= metrics["f1"] <= 1.0
    assert 0.0 <= metrics["roc_auc"] <= 1.0
    assert len(metrics["confusion_matrix"]) == 2


def test_rejects_empty_training_data():
    """Reject empty training datasets."""
    with pytest.raises(ValueError, match="cannot be empty"):
        train_baseline(pd.DataFrame())


def test_rejects_single_class_training_data():
    """Require both classes to fit a binary classifier."""
    dataset = make_dataset()
    dataset["target"] = 1

    with pytest.raises(ValueError, match="both target classes"):
        train_baseline(dataset)


def test_rejects_missing_training_features():
    """Reject missing values rather than silently imputing them."""
    dataset = make_dataset()
    dataset.loc[0, "return_1"] = np.nan

    with pytest.raises(ValueError, match="missing values"):
        train_baseline(dataset)


def test_evaluation_does_not_refit_scaler():
    """Ensure evaluation leaves the fitted scaler unchanged."""
    training_data = make_dataset()
    model = train_baseline(training_data)

    original_mean = model.named_steps["scaler"].mean_.copy()

    evaluation_data = make_dataset()
    evaluation_data["price"] += 10000

    evaluate_classifier(model, evaluation_data)

    np.testing.assert_array_equal(
        model.named_steps["scaler"].mean_,
        original_mean,
    )


def test_roc_auc_is_none_when_evaluation_has_one_class():
    """Handle evaluation data without both target classes."""
    training_data = make_dataset()
    model = train_baseline(training_data)

    evaluation_data = make_dataset(10)
    evaluation_data["target"] = 1

    metrics = evaluate_classifier(model, evaluation_data)

    assert metrics["roc_auc"] is None