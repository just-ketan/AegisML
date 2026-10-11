"""Typed schemas representing market events produced by the C++ simulator."""

from dataclasses import dataclass
from enum import Enum


class EventType(Enum):
    """Identify the event types supported by the market event format."""

    TRADE = 0
    QUOTE = 1
    ADD = 2
    CANCEL = 3
    EXECUTE = 4


class Side(Enum):
    """Represent the side of an order in the market."""

    BUY = 0
    SELL = 1


@dataclass(frozen=True, slots=True)
class TradePayload:
    """Store the price and quantity associated with a trade."""

    price: int
    quantity: int


@dataclass(frozen=True, slots=True)
class QuotePayload:
    """Store the bid and ask prices and quantities of a market quote."""

    bid_price: int
    bid_quantity: int
    ask_price: int
    ask_quantity: int


@dataclass(frozen=True, slots=True)
class AddOrderPayload:
    """Store the details of a newly added order."""

    order_id: int
    side: Side
    price: int
    quantity: int


@dataclass(frozen=True, slots=True)
class CancelOrderPayload:
    """Store the details of a cancellation against an existing order."""

    order_id: int
    quantity: int


@dataclass(frozen=True, slots=True)
class ExecuteOrderPayload:
    """Store the details of an execution against an existing order."""

    order_id: int
    price: int
    quantity: int


EventPayload = (
    TradePayload
    | QuotePayload
    | AddOrderPayload
    | CancelOrderPayload
    | ExecuteOrderPayload
)


@dataclass(frozen=True, slots=True)
class MarketEventRecord:
    """Represent a decoded market event with its typed payload."""

    event_id: int
    sequence_number: int
    timestamp_ms: int
    symbol: str
    event_type: EventType
    payload: EventPayload
