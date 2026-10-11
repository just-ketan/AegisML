import pandas as pd
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import (
    accuracy_score,
    balanced_accuracy_score,
    confusion_matrix,
    f1_score,
    precision_score,
    recall_score,
    roc_auc_score,
)
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import StandardScaler
from aegisml.datasets.supervised import MODEL_FEATURE_COLUMNS

def train_baseline(training_data: pd.DataFrame,) -> Pipeline:
    # training standard logistic regression classifier

    if training_data.empty:
        raise ValueError("Training data cannot be empty")

    required_columns = set(MODEL_FEATURE_COLUMNS) | {"target"}
    missing = required_columns.difference(training_data.columns)
    if missing:
        raise ValueError(f"Missing required training columns: {sorted(missing)}")
    if training_data[MODEL_FEATURE_COLUMNS].isna().any().any():
        raise ValueError("Training data cant contain missing values")
    if not training_data[MODEL_FEATURE_COLUMNS].apply(lambda column : column.map(lambda val: isinstance(val, (int,float)))).all().all():
        raise ValueError("Training features must be numeric")
    if not training_data["target"].isin([0,1]).all():
        raise ValueError("Targets must contain only 0 and 1")

    # Ensure the target contains only valid binary labels.
    if not training_data["target"].isin([0, 1]).all():
        raise ValueError("Targets must contain only 0 and 1.")

    # A binary classifier requires examples from both classes.
    if training_data["target"].nunique() != 2:
        raise ValueError(
            "Training data must contain both target classes, 0 and 1."
        )

    # Convert features to numeric values for reliable validation.
    try:
        numeric_features = training_data[MODEL_FEATURE_COLUMNS].to_numpy(
            dtype="float64",
            na_value=np.nan,
        )
    except (TypeError, ValueError) as exc:
        raise ValueError(
            "Training features must be numeric."
        ) from exc

    # Reject NaN, positive infinity, and negative infinity.
    if not np.isfinite(numeric_features).all():
        raise ValueError(
            "Training features cannot contain missing values "
            "or infinite values."
        )

    # pipeline fitting ensures the scaler learns only from training data
    model = Pipeline(
        steps=[("scaler", StandardScaler()),
               ("classifier", LogisticRegression(max_iter=1000,class_weight="balanced",random_state=42,),),
            ])

    model.fit(training_data[MODEL_FEATURE_COLUMNS], training_data["target"].astype(int),)
    return model

def evaluate_classifier(model: Pipeline, dataset:pd.DataFrame) -> dict:
    if dataset.empty:
        raise ValueError("Evaluation dataset cannot be empty")
    required_columns = set(MODEL_FEATURE_COLUMNS) | {"target"}
    missing = required_columns.difference(dataset.columns)
    if missing:
        raise ValueError(f"Missing required evaluation columns: {sorted(missing)}")
    if dataset[MODEL_FEATURE_COLUMNS].isna().any().any():
        raise ValueError("Evaluation features cannot contain missing values")
    if not dataset["target"].isin([0,1]).all():
        raise ValueError("Target must contain only 0 and 1")

        # Ensure evaluation data contains valid numeric features.
    try:
        numeric_features = dataset[MODEL_FEATURE_COLUMNS].to_numpy(
            dtype="float64",
            na_value=np.nan,
        )
    except (TypeError, ValueError) as exc:
        raise ValueError(
            "Evaluation features must be numeric."
        ) from exc

    if not np.isfinite(numeric_features).all():
        raise ValueError(
            "Evaluation features cannot contain missing values "
            "or infinite values."
        )

    features = dataset[MODEL_FEATURE_COLUMNS]
    targets = dataset["target"].astype(int)
    predictions = model.predict(features)

    # report class specific metrics and an overall performance summary
    results = {
        "samples": len(dataset),
        "accuracy": float(accuracy_score(targets, predictions)),
        "balanced_accuracy": float(balanced_accuracy_score(targets, predictions)),
        "precision": float(precision_score(targets, predictions, zero_division=0)),
        "recall": float(recall_score(targets, predictions, zero_division=0)),
        "f1": float(f1_score(targets, predictions, zero_division=0)),
        "confusion_matrix": confusion_matrix(targets, predictions, labels=[0,1],).tolist(),
    }

    # AUC requires both positive and negative examples
    if targets.nunique() == 2:
        probabilities = model.predict_proba(features)[:,1]
        results["roc_auc"] = float(roc_auc_score(targets, probabilities))
    else:
        results["roc_auc"] = None

    return results