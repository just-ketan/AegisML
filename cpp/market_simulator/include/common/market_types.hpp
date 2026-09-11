#pragma once

#include <chrono>
#include <cstdint>

using EventId = std::uint64_t;
using SequenceNumber = std::uint64_t;
using Timestamp = std::chrono::milliseconds;
using Price = std::int64_t;
using Quantity = std::int64_t;
