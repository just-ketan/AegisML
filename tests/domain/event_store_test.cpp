#include <filesystem>
#include <fstream>
#include <vector>

#include <gtest/gtest.h>

#include "events/event_store.hpp"

namespace {

class EventStoreTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        path_ = "event_store_test.bin";
        std::filesystem::remove(path_);
    }

    void TearDown() override
    {
        std::filesystem::remove(path_);
    }

    MarketEvent make_trade(
        EventId event_id,
        SequenceNumber sequence
    ) const
    {
        return MarketEvent{
            .event_id = event_id,
            .sequence_number = sequence,
            .timestamp = Timestamp{1000},
            .symbol = make_symbol("AAPL"),
            .payload = TradeEvent{
                .price = 15000,
                .quantity = 100
            }
        };
    }

    std::filesystem::path path_;
};

TEST_F(EventStoreTest, StartsEmpty)
{
    EventStore store(path_.string());

    EXPECT_EQ(store.size(), 0);
}

TEST_F(EventStoreTest, AppendSingleEvent)
{
    EventStore store(path_.string());

    const MarketEvent event = make_trade(1, 1);

    store.append(event);

    EXPECT_EQ(store.size(), 1);
}

TEST_F(EventStoreTest, AppendMultipleEvents)
{
    EventStore store(path_.string());

    store.append(make_trade(1, 1));
    store.append(make_trade(2, 2));
    store.append(make_trade(3, 3));

    EXPECT_EQ(store.size(), 3);
}

TEST_F(EventStoreTest, FlushWritesData)
{
    {
        EventStore store(path_.string());

        store.append(make_trade(1, 1));
        store.flush();
    }

    ASSERT_TRUE(std::filesystem::exists(path_));
    EXPECT_GT(
        std::filesystem::file_size(path_),
        0
    );
}

TEST_F(EventStoreTest, AppendsToExistingFile)
{
    {
        EventStore store(path_.string());
        store.append(make_trade(1, 1));
        store.flush();
    }

    const auto first_size =
        std::filesystem::file_size(path_);

    {
        EventStore store(path_.string());
        store.append(make_trade(2, 2));
        store.flush();
    }

    const auto second_size =
        std::filesystem::file_size(path_);

    EXPECT_GT(second_size, first_size);
}

} // namespace