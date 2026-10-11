"""Read and decode binary market events produced by the C++ simulator."""

import struct
from collections.abc import Iterator
from pathlib import Path

from aegisml.data.schemas import (
    AddOrderPayload,
    CancelOrderPayload,
    EventPayload,
    EventType,
    ExecuteOrderPayload,
    MarketEventRecord,
    QuotePayload,
    Side,
    TradePayload,
)

MAGIC = b"AEGM"
FORMAT_VERSION = 1

# Common event header:
# event_id, sequence_number, timestamp_ms, symbol.
COMMON_HEADER = struct.Struct("<QQq8s")

UINT32 = struct.Struct("<I")
UINT64 = struct.Struct("<Q")
INT64 = struct.Struct("<q")
UINT8 = struct.Struct("<B")

# Maximum record size accepted by the reader.
# All currently supported events are much smaller than this limit.
MAX_RECORD_SIZE = 1024


class EventReaderError(ValueError):
    """Raised when a binary market event record is invalid."""


def read_exact(stream, size: int) -> bytes:
    """Read exactly the requested number of bytes or reject truncated input."""
    data = stream.read(size)

    if len(data) != size:
        raise EventReaderError(
            f"Truncated event log: expected {size} bytes, received {len(data)}."
        )

    return data


def read_record(stream) -> bytes | None:
    """Read one length-prefixed event; return None when clean EOF is reached."""
    prefix = stream.read(UINT32.size)

    if not prefix:
        return None

    if len(prefix) != UINT32.size:
        raise EventReaderError("Truncated event log: incomplete record-length prefix.")

    record_size = UINT32.unpack(prefix)[0]

    if record_size == 0:
        raise EventReaderError("Invalid event record: record length cannot be zero.")

    if record_size > MAX_RECORD_SIZE:
        raise EventReaderError(
            f"Invalid event record: {record_size} bytes exceeds "
            f"the {MAX_RECORD_SIZE}-byte limit."
        )

    return read_exact(stream, record_size)


def decode_payload(
    event_type: EventType,
    data: memoryview,
    offset: int,
) -> tuple[EventPayload, int]:
    """Decode a payload and return it together with the next unread byte offset."""

    def read_i64() -> int:
        """Read a signed 64-bit payload field and advance the local offset."""
        nonlocal offset

        if offset + INT64.size > len(data):
            raise EventReaderError("Truncated event payload: expected an int64 field.")

        value = INT64.unpack_from(data, offset)[0]
        offset += INT64.size
        return value

    def read_u64() -> int:
        """Read an unsigned 64-bit payload field and advance the local offset."""
        nonlocal offset

        if offset + UINT64.size > len(data):
            raise EventReaderError("Truncated event payload: expected a uint64 field.")

        value = UINT64.unpack_from(data, offset)[0]
        offset += UINT64.size
        return value

    def read_u8() -> int:
        """Read an unsigned 8-bit payload field and advance the local offset."""
        nonlocal offset

        if offset + UINT8.size > len(data):
            raise EventReaderError("Truncated event payload: expected a uint8 field.")

        value = UINT8.unpack_from(data, offset)[0]
        offset += UINT8.size
        return value

    if event_type is EventType.TRADE:
        payload = TradePayload(
            price=read_i64(),
            quantity=read_i64(),
        )

    elif event_type is EventType.QUOTE:
        payload = QuotePayload(
            bid_price=read_i64(),
            bid_quantity=read_i64(),
            ask_price=read_i64(),
            ask_quantity=read_i64(),
        )

    elif event_type is EventType.ADD:
        order_id = read_u64()
        raw_side = read_u8()

        try:
            side = Side(raw_side)
        except ValueError as exc:
            raise EventReaderError(
                f"Invalid add-order payload: unknown side value {raw_side}."
            ) from exc

        payload = AddOrderPayload(
            order_id=order_id,
            side=side,
            price=read_i64(),
            quantity=read_i64(),
        )

    elif event_type is EventType.CANCEL:
        payload = CancelOrderPayload(
            order_id=read_u64(),
            quantity=read_i64(),
        )

    elif event_type is EventType.EXECUTE:
        payload = ExecuteOrderPayload(
            order_id=read_u64(),
            price=read_i64(),
            quantity=read_i64(),
        )

    else:
        raise EventReaderError(f"Unsupported event type: {event_type!r}.")

    return payload, offset


def decode_event(record: bytes) -> MarketEventRecord:
    """Decode one serialized C++ event, excluding its outer length prefix."""
    data = memoryview(record)
    offset = 0

    minimum_header_size = len(MAGIC) + UINT32.size

    if len(data) < minimum_header_size:
        raise EventReaderError("Invalid event record: header is incomplete.")

    if bytes(data[:len(MAGIC)]) != MAGIC:
        raise EventReaderError("Invalid event record: incorrect magic bytes.")

    offset += len(MAGIC)

    version = UINT32.unpack_from(data, offset)[0]
    offset += UINT32.size

    if version != FORMAT_VERSION:
        raise EventReaderError(
            f"Unsupported event format version: {version}; "
            f"expected {FORMAT_VERSION}."
        )

    header_size = COMMON_HEADER.size

    if offset + header_size + UINT8.size > len(data):
        raise EventReaderError("Invalid event record: common header is incomplete.")

    event_id, sequence_number, timestamp_ms, raw_symbol = (
        COMMON_HEADER.unpack_from(data, offset)
    )
    offset += header_size

    try:
        symbol = raw_symbol.split(b"\x00", maxsplit=1)[0].decode("ascii")
    except UnicodeDecodeError as exc:
        raise EventReaderError("Invalid event record: symbol is not ASCII.") from exc

    raw_event_type = UINT8.unpack_from(data, offset)[0]
    offset += UINT8.size

    try:
        event_type = EventType(raw_event_type)
    except ValueError as exc:
        raise EventReaderError(
            f"Invalid event record: unknown event type {raw_event_type}."
        ) from exc

    payload, offset = decode_payload(event_type, data, offset)

    if offset != len(data):
        raise EventReaderError(
            f"Invalid event record: {len(data) - offset} unexpected trailing bytes."
        )

    return MarketEventRecord(
        event_id=event_id,
        sequence_number=sequence_number,
        timestamp_ms=timestamp_ms,
        symbol=symbol,
        event_type=event_type,
        payload=payload,
    )


def iter_events(path: str | Path) -> Iterator[MarketEventRecord]:
    """Stream decoded events from a binary event log without loading it all."""
    with Path(path).open("rb") as stream:
        while True:
            record = read_record(stream)

            if record is None:
                break

            yield decode_event(record)