# feature engineering for trade level market data

import pandas as pd

FEATURE_COLUMNS = [
    "event_id",
    "sequence_number",
    "timestamp_ms",
    "symbol",
    "price",
    "quantity",
    "return_1",
    "rolling_mean_return",
    "rolling_volatility",
    "rolling_mean_volume",
]

def build_trade_features(events:pd.DataFrame, window:int = 5,) -> pd.DataFrame:
    if window < 2:
        raise ValueError("window must be at least 2")

    required_columns = {
        "event_id",
        "sequence_number",
        "timestamp_ms",
        "symbol",
        "event_type",
        "price",
        "quantity",
        }
    missing = required_columns.difference(events.columns)

    if missing:
        raise ValueError(f"missing required market data columns: {sorted(missing)}")

    trades = events.loc[
        events["event_type"].eq("TRADE") & events["price"].notna() & events["quantity"].notna()
        ].copy()
    if trades.empty:
        return pd.DataFrame(columns = FEATURE_COLUMNS)

    trades = trades.sort_values(["symbol", "timestamp_ms", "sequence_number"], kind="stable").reset_index(drop=True)
    trades["return_1"] = trades.groupby("symbol", sort=False)["price"].pct_change(fill_method = None)
    grouped_returns = trades.groupby("symbol", sort=False)["return_1"]
    grouped_volume = trades.groupby("symbol", sort=False)["quantity"]
    trades["rolling_mean_return"] = grouped_returns.transform(lambda series: series.rolling(window, min_periods=2).mean())
    trades["rolling_volatility"] = grouped_returns.transform(lambda series: series.rolling(window, min_periods=2).std())
    trades["rolling_mean_volume"] = grouped_volume.transform(lambda series: series.rolling(window, min_periods=1).mean())

    return trades[FEATURE_COLUMNS].reset_index(drop=True)

'''
This ensure that a trade in AAPL must not affect rolling volatility calculated for MSFT
rolling statistics are meaningful only when observations are processed in chronological order
first trade for a sumbol has no previous trade price, so return is undefined
the first few rows contains missing rolling stats, will decide how to handle when constructing training dataset
'''