#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "events/event_store.hpp"
#include "events/persistent_replay_source.hpp"

namespace {

class PersistentReplaySourceTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        path_ =
            std::filesystem::temp_directory_path()
            / "aegisml_persistent_replay_test.bin";

        std::filesystem::remove(path_);

        {
            EventStore store(path_.string());

            store.append(make_trade(1, 1));
            store.append(make_trade(2, 2));
            store.append(make_trade(3, 3));

            store.flush();
        }

        ASSERT_TRUE(std::filesystem::exists(path_));
        ASSERT_GT(std::filesystem::file_size(path_), 0);
    }

    void TearDown() override
    {
        // std::filesystem::remove(path_);
    }

    MarketEvent make_trade(
        EventId id,
        SequenceNumber sequence
    ) const
    {
        return MarketEvent{
            .event_id = id,
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

TEST_F(PersistentReplaySourceTest, ReadsEventsSequentially)
{
    PersistentReplaySource source(path_.string());

    MarketEvent event;

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 1);
    EXPECT_EQ(event.sequence_number, 1);

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 2);
    EXPECT_EQ(event.sequence_number, 2);

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 3);
    EXPECT_EQ(event.sequence_number, 3);

    EXPECT_FALSE(source.next_event(event));
    EXPECT_TRUE(source.exhausted());
}

TEST_F(PersistentReplaySourceTest, TracksPosition)
{
    PersistentReplaySource source(path_.string());

    EXPECT_EQ(source.position(), 0);

    MarketEvent event;

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(source.position(), 1);

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(source.position(), 2);
}

TEST_F(PersistentReplaySourceTest, ResetReplaysFromBeginning)
{
    PersistentReplaySource source(path_.string());

    MarketEvent event;

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 1);

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 2);

    source.reset();

    EXPECT_EQ(source.position(), 0);
    EXPECT_FALSE(source.exhausted());

    ASSERT_TRUE(source.next_event(event));
    EXPECT_EQ(event.event_id, 1);
}

TEST_F(PersistentReplaySourceTest, ReadsBatch)
{
    PersistentReplaySource source(path_.string());

    std::vector<MarketEvent> batch;

    ASSERT_TRUE(source.next_batch(batch, 2));

    ASSERT_EQ(batch.size(), 2);
    EXPECT_EQ(batch[0].event_id, 1);
    EXPECT_EQ(batch[1].event_id, 2);

    ASSERT_TRUE(source.next_batch(batch, 2));

    ASSERT_EQ(batch.size(), 1);
    EXPECT_EQ(batch[0].event_id, 3);
}

TEST_F(PersistentReplaySourceTest, ZeroBatchSizeReturnsFalse)
{
    PersistentReplaySource source(path_.string());

    std::vector<MarketEvent> batch;

    EXPECT_FALSE(source.next_batch(batch, 0));
    EXPECT_TRUE(batch.empty());
    EXPECT_EQ(source.position(), 0);
}

} // namespace