# chronological spl;itting utils for market-data dataset

import pandas as pd

def chronological_split(dataset:pd.DataFrame, train_fraction: float = 0.70, validation_fraction: float=0.15) -> tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]:
    # split obs into chronological train validation and test suite frames

    if not 0 < train_fraction < 1:
        raise ValueError("train_fraction must be between 0 and 1")
    if not 0 < validation_fraction < 1:
        raise ValueError("validation_fraction must be between 0 and 1")
    if train_fraction + validation_fraction >= 1:
        raise ValueError("train_fraction + validation_fraction must be less than 1")

    if "timestamp_ms" not in dataset.columns:
        raise ValueError("Dataset must contain timestamp_ms")
    if dataset.empty:
        raise ValueError("Cannot split an empty dataset")
    if dataset["timestamp_ms"].isna().any():
        raise ValueError("timestamp_ms cannot contain missing values")

    # sort gloablly by time, using sequence number to order simulataneous entries
    sort_columns = ["timestamp_ms"]
    if "sequence_number" in dataset.columns:
        sort_columns.append("sequence_number")

    ordered = dataset.sort_values(sort_columns, kind="stable",).reset_index(drop=True)

    # split by unique timestamps
    timestamps = ordered["timestamp_ms"].drop_duplicates().tolist()
    timestamp_count = len(timestamps)

    train_end = int(timestamp_count * train_fraction)
    validation_end = int(timestamp_count * (train_fraction + validation_fraction))
    if train_end < 1 or validation_end <= train_end:
        raise ValueError("Not enough unique timestamps for a non-empty test split")
    if validation_end >= timestamp_count:
        raise ValueError("Not enough timestamps for non empty test split")

    train_cutoff = timestamps[train_end]
    validation_cutoff = timestamps[validation_end]

    train = ordered.loc[ordered["timestamp_ms"] < train_cutoff].copy()
    validation = ordered.loc[(ordered["timestamp_ms"] >= train_cutoff) & (ordered["timestamp_ms"] < validation_cutoff)].copy()
    test = ordered.loc[ordered["timestamp_ms"] >= validation_cutoff].copy()

    return (
        train.reset_index(drop=True),
        validation.reset_index(drop=True),
        test.reset_index(drop=True),
    )


