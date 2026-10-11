"""Tests for decoding binary market events."""

import struct

import pytest

from aegisml.data.event_reader import (
    EventReaderError,
    decode_event,
    iter_events,
)
from aegisml.data.schemas import (
    AddOrderPayload,
    CancelOrderPayload,
    EventType,
    ExecuteOrderPayload,
    QuotePayload,
    Side,
    TradePayload,
)

MAGIC = b"AEGM"
VERSION = struct.pack("<I", 1)
COMMON_HEADER = struct.Struct("<QQq8s")


def make_record(event_type: int, payload: bytes) -> bytes:
    """Build one serialized event using the documented C++ binary layout."""
    header = COMMON_HEADER.pack(
        42,
        7,
        123456,
        b"AAPL\x00\x00\x00\x00",
    )

    return MAGIC + VERSION + header + struct.pack("<B", event_type) + payload


def frame_record(record: bytes) -> bytes:
    """Add the EventStore four-byte little-endian record-length prefix."""
    return struct.pack("<I", len(record)) + record


@pytest.mark.parametrize(
    ("event_type", "payload_bytes", "expected_payload"),
    [
        (
            EventType.TRADE,
            struct.pack("<qq", 15000, 250),
            TradePayload(price=15000, quantity=250),
        ),
        (
            EventType.QUOTE,
            struct.pack("<qqqq", 14900, 100, 15100, 200),
            QuotePayload(
                bid_price=14900,
                bid_quantity=100,
                ask_price=15100,
                ask_quantity=200,
            ),
        ),
        (
            EventType.ADD,
            struct.pack("<QBqq", 1234, Side.BUY.value, 15000, 50),
            AddOrderPayload(
                order_id=1234,
                side=Side.BUY,
                price=15000,
                quantity=50,
            ),
        ),
        (
            EventType.CANCEL,
            struct.pack("<Qq", 1234, 25),
            CancelOrderPayload(order_id=1234, quantity=25),
        ),
        (
            EventType.EXECUTE,
            struct.pack("<Qqq", 1234, 15000, 25),
            ExecuteOrderPayload(
                order_id=1234,
                price=15000,
                quantity=25,
            ),
        ),
    ],
)
def test_decodes_all_event_types(event_type, payload_bytes, expected_payload):
    """Verify common metadata and typed payload decoding for every event."""
    event = decode_event(make_record(event_type.value, payload_bytes))

    assert event.event_id == 42
    assert event.sequence_number == 7
    assert event.timestamp_ms == 123456
    assert event.symbol == "AAPL"
    assert event.event_type is event_type
    assert event.payload == expected_payload


def test_preserves_large_and_negative_int64_values():
    """Ensure signed 64-bit fields are decoded without 32-bit truncation."""
    payload = struct.pack("<qq", 5_000_000_000, -6_000_000_000)
    event = decode_event(make_record(EventType.TRADE.value, payload))

    assert event.payload == TradePayload(
        price=5_000_000_000,
        quantity=-6_000_000_000,
    )


def test_rejects_invalid_magic():
    """Reject records that do not begin with the expected magic bytes."""
    record = b"NOPE" + VERSION + bytes(29)

    with pytest.raises(EventReaderError, match="magic"):
        decode_event(record)


def test_rejects_unsupported_version():
    """Reject records encoded with an unsupported format version."""
    record = MAGIC + struct.pack("<I", 99) + bytes(29)

    with pytest.raises(EventReaderError, match="version"):
        decode_event(record)


def test_rejects_truncated_payload():
    """Reject a trade record missing part of its payload."""
    record = make_record(
        EventType.TRADE.value,
        struct.pack("<q", 15000),
    )

    with pytest.raises(EventReaderError, match="Truncated"):
        decode_event(record)


def test_rejects_invalid_order_side():
    """Reject add-order payloads with unknown side discriminators."""
    payload = struct.pack("<QBqq", 1234, 99, 15000, 50)
    record = make_record(EventType.ADD.value, payload)

    with pytest.raises(EventReaderError, match="side"):
        decode_event(record)


def test_rejects_trailing_bytes():
    """Reject records containing data beyond the declared payload."""
    record = make_record(
        EventType.TRADE.value,
        struct.pack("<qq", 15000, 250) + b"\x00",
    )

    with pytest.raises(EventReaderError, match="trailing"):
        decode_event(record)


def test_iterates_over_multiple_framed_events(tmp_path):
    """Read multiple length-prefixed events sequentially from a file."""
    trade = make_record(
        EventType.TRADE.value,
        struct.pack("<qq", 15000, 250),
    )
    cancel = make_record(
        EventType.CANCEL.value,
        struct.pack("<Qq", 1234, 25),
    )

    log_path = tmp_path / "events.bin"
    log_path.write_bytes(frame_record(trade) + frame_record(cancel))

    events = list(iter_events(log_path))

    assert len(events) == 2
    assert events[0].event_type is EventType.TRADE
    assert events[1].event_type is EventType.CANCEL


def test_rejects_truncated_record_length_prefix(tmp_path):
    """Reject a file ending partway through the four-byte length prefix."""
    log_path = tmp_path / "truncated.bin"
    log_path.write_bytes(b"\x10\x00")

    with pytest.raises(EventReaderError, match="prefix"):
        list(iter_events(log_path))


def test_rejects_truncated_record_body(tmp_path):
    """Reject a record whose declared length exceeds its available bytes."""
    log_path = tmp_path / "truncated.bin"
    log_path.write_bytes(struct.pack("<I", 20) + b"AEGM")

    with pytest.raises(EventReaderError, match="Truncated"):
        list(iter_events(log_path))
