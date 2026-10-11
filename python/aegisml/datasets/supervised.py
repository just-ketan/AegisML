# construct a supervised learning datasets from trade features

import numpy as np
import pandas as pd

MODEL_FEATURE_COLUMNS = [
    "price",
    "quantity",
    "return_1",
    "rolling_mean_return",
    "rolling_volatility",
    "rolling_mean_volume",
]

DATASET_COLUMNS = [
    "event_id",
    "sequence_number",
    "timestamp_ms",
    "symbol",
    *MODEL_FEATURE_COLUMNS,
    "target",
]

def build_supervised_dataset(features: pd.DataFrame,) -> pd.DataFrame:
    # build a dataset predicting the next trade's price direction
    # 1: next trade price for same symbol is higher
    # 2. next trade price is unchanged or lower
    # Feature calculations must only use info at current trade
    # discard improper rows

    required_columns = {
        "event_id",
        "sequence_number",
        "timestamp_ms",
        "symbol",
        *MODEL_FEATURE_COLUMNS,
    }

    missing = required_columns.difference(features.columns)
    if missing:
        raise ValueError(f"Missing required feature columns: {sorted(missing)}")
    if features.empty:
        return pd.DataFrame(columns=DATASET_COLUMNS)

    # work on copy so dataset comnstruction never mutates its inpuits
    dataset = features.copy()
    dataset = dataset.sort_values(["symbol","timestamp_ms","sequence_number"], kind="stable",).reset_index(drop=True)

    # shift withing each symbol so the next price never crosses symbols
    next_price = dataset.groupby("symbol", sort=False,)["price"].shift(-1)

    # a positive next-price movement is class 1, otw class 0
    valid_target = next_price.notna()
    dataset["target"] = (next_price > dataset["price"]).astype("int8")
    dataset.loc[~valid_target, "target"] = pd.NA

    # reject missing and infinite values before model training
    numeric_features = dataset[MODEL_FEATURE_COLUMNS].to_numpy(dtype="float64", na_value=np.nan,)
    valid_features = pd.Series(np.isfinite(numeric_features).all(axis=1), index=dataset.index,)

    dataset = dataset.loc[valid_target & valid_features].copy()
    dataset = dataset.sort_values(["timestamp_ms", "sequence_number"], kind="stable",)
    # maintain globally chronological dataset for later temportal splits

    return dataset[DATASET_COLUMNS].reset_index(drop=True)