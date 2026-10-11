# accepts decode MarketEventRecord and produces a pandas DataFrame with consistent schema

from collections.abc import Iterable

import pandas as pd

from aegisml.data.schemas import (
    AddOrderPayload,
    CancelOrderPayload,
    ExecuteOrderPayload,
    MarketEventRecord,
    QuotePayload,
    TradePayload,
)

COLUMNS = [
    "event_id",
    "sequence_number",
    "timestamp_ms",
    "symbol",
    "event_type",
    "order_id",
    "side",
    "price",
    "quantity",
    "bid_price",
    "bid_quantity",
    "ask_price",
    "ask_quantity",
]

def events_to_frame(events: Iterable[MarketEventRecord]) -> pd.DataFrame:
    rows = []
    for event in events:
        row = {
            "event_id": event.event_id,
            "sequence_number": event.sequence_number,
            "timestamp_ms": event.timestamp_ms,
            "symbol": event.symbol,
            "event_type": event.event_type.name,
            "order_id": None,
            "side": None,
            "price": None,
            "quantity": None,
            "bid_price": None,
            "bid_quantity": None,
            "ask_price": None,
            "ask_quantity": None,
        }

        payload = event.payload
        if isinstance(payload, TradePayload):
            row["price"] = payload.price
            row["quantity"] = payload.quantity

        elif isinstance(payload, QuotePayload):
            row["bid_price"] = payload.bid_price
            row["bid_quantity"] = payload.bid_quantity
            row["ask_price"] = payload.ask_price
            row["ask_quantity"] = payload.ask_quantity

        elif isinstance(payload, AddOrderPayload):
            row["order_id"] = payload.order_id
            row["side"] = payload.side.name
            row["price"] = payload.price
            row["quantity"] = payload.quantity

        elif isinstance(payload, CancelOrderPayload):
            row["order_id"] = payload.order_id
            row["quantity"] = payload.quantity

        elif isinstance(payload, ExecuteOrderPayload):
            row["order_id"] = payload.order_id
            row["price"] = payload.price
            row["quantity"] = payload.quantity

        rows.append(row)

    frame = pd.DataFrame(rows, columns = COLUMNS)

    # preserve integer precision in nullable columns
    integer_columns = [
        "event_id",
        "sequence_number",
        "timestamp_ms",
        "order_id",
        "price",
        "quantity",
        "bid_price",
        "bid_quantity",
        "ask_price",
        "ask_quantity",
    ]
    for column in integer_columns:
        frame[column] = pd.array(frame[column], dtype = "Int64")

    return frame