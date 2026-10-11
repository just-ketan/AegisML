# reject malformed records and invalid event data before it enters the ML pipeline

from aegisml.data.schemas import (
    AddOrderPayload,
    CancelOrderPayload,
    EventType,
    ExecuteOrderPayload,
    MarketEventRecord,
    QuotePayload,
    TradePayload,
)

EXPECTED_PAYLOAD_TYPES = {
    EventType.TRADE: TradePayload,
    EventType.QUOTE: QuotePayload,
    EventType.ADD: AddOrderPayload,
    EventType.CANCEL: CancelOrderPayload,
    EventType.EXECUTE: ExecuteOrderPayload,
}


class EventValidationError(ValueError):
    """Raised when a market event violates the Python event contract."""


def validate_event(event: MarketEventRecord) -> None:
    """Validate an event's structure and its compatibility with the C++ contract."""
    if not isinstance(event, MarketEventRecord):
        raise EventValidationError("Expected a MarketEventRecord.")

    if not isinstance(event.event_type, EventType):
        raise EventValidationError("Unknown event type.")

    if not isinstance(event.payload, EXPECTED_PAYLOAD_TYPES[event.event_type]):
        raise EventValidationError(
            f"{event.event_type.name} requires "
            f"{EXPECTED_PAYLOAD_TYPES[event.event_type].__name__}."
        )

    for name, value in (
        ("event_id", event.event_id),
        ("sequence_number", event.sequence_number),
    ):
        if type(value) is not int or not 0 <= value < 2**64:
            raise EventValidationError(
                f"{name} must be an unsigned 64-bit integer."
            )

    if (
        type(event.timestamp_ms) is not int
        or not -(2**63) <= event.timestamp_ms < 2**63
    ):
        raise EventValidationError(
            "timestamp_ms must be a signed 64-bit integer."
        )

    if not isinstance(event.symbol, str) or not event.symbol:
        raise EventValidationError("Symbol must be a nonempty string.")

    try:
        encoded_symbol = event.symbol.encode("ascii")
    except UnicodeEncodeError as exc:
        raise EventValidationError("Symbol must contain ASCII characters.") from exc

    if len(encoded_symbol) > 8:
        raise EventValidationError("Symbol cannot exceed eight bytes.")

    payload = event.payload

    for name, value in vars_for_payload(payload).items():
        if type(value) is not int:
            raise EventValidationError(f"{name} must be an integer.")

    if isinstance(payload, AddOrderPayload):
        from aegisml.data.schemas import Side

        if not isinstance(payload.side, Side):
            raise EventValidationError("Order side must be BUY or SELL.")

    for name, value in vars_for_payload(payload).items():
        if name in {"order_id"}:
            if not 0 <= value < 2**64:
                raise EventValidationError(
                    f"{name} must be an unsigned 64-bit integer."
                )
        elif not -(2**63) <= value < 2**63:
            raise EventValidationError(
                f"{name} must fit in a signed 64-bit integer."
            )


def vars_for_payload(payload: object) -> dict[str, int]:
    """Extract integer fields from a known payload dataclass."""
    return {
        name: value
        for name, value in (
            (field, getattr(payload, field))
            for field in payload.__dataclass_fields__
        )
    }