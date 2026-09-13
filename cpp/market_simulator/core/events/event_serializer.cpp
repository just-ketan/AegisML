#include "events/event_serializer.hpp"

#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace{
    constexpr std::uint8_t TRADE_EVENT = 0;
    constexpr std::uint8_t QUOTE_EVENT = 1;
    constexpr std::uint8_t ADD_ORDER_EVENT = 2;
    constexpr std::uint8_t CANCEL_ORDER_EVENT = 3;
    constexpr std::uint8_t EXECUTE_ORDER_EVENT = 4;
    
    constexpr std::uint8_t BUY_SIDE = 0;
    constexpr std::uint8_t SELL_SIDE = 1;

    
    constexpr std::uint8_t MAGIC[] = {'A', 'E', 'G', 'M'};

    class Writer{
        public:
            void write_u8(std::uint8_t value){
                data_.push_back(value);
            }

            void write_u32(std::uint32_t value){
                for(int i=0; i<4; i++){
                    data_.push_back(static_cast<std::uint8_t>((value >> (i*8)) & 0xFF));
                }
            }

            void write_u64(std::uint64_t value)
            {
                for (int i = 0; i < 8; ++i) {
                    data_.push_back(
                        static_cast<std::uint8_t>(
                            (value >> (i * 8)) & 0xFF
                        )
                    );
                }
            }

            void write_i64(std::uint32_t value){
                write_u64(static_cast<std::uint64_t>(value));
            }

            void write_symbol(const Symbol& symbol){
                for(char c : symbol){
                    data_.push_back(static_cast<std::uint8_t>(c));
                }
            }

            const std::vector<uint8_t> data() const {
                return data_;
            }
        
        private:
            std::vector<std::uint8_t> data_;
    };

    class Reader {
public:
    explicit Reader(const std::vector<std::uint8_t>& data) : data_(data) {}
    std::uint8_t read_u8(){
        require(1);
        return data_[position_++];
    }

    std::uint32_t read_u32(){
        require(4);
        std::uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            value |= static_cast<std::uint32_t>(data_[position_++]) << (i * 8);
        }
        return value;
    }

    std::uint64_t read_u64()
    {
        require(8);

        std::uint64_t value = 0;

        for (int i = 0; i < 8; ++i) {
            value |=
                static_cast<std::uint64_t>(
                    data_[position_++]
                ) << (i * 8);
        }

        return value;
    }

    std::int64_t read_i64(){
        return static_cast<std::int64_t>(read_u64());
    }

    Symbol read_symbol(){
        require(MAX_SYMBOL_LENGTH);
        Symbol symbol{};

        for (std::size_t i = 0; i < MAX_SYMBOL_LENGTH; ++i) {
            symbol[i] = static_cast<char>(data_[position_++]);
        }
        return symbol;
    }

    bool finished() const{
        return position_ == data_.size();
    }

private:
    void require(std::size_t bytes){
        if (position_ + bytes > data_.size()) {
            throw std::runtime_error("Truncated serialized event");
        }
    }

    const std::vector<std::uint8_t>& data_;
    std::size_t position_ = 0;
};

std::uint8_t encode_side(Side side){
    switch (side) {
        case Side::Buy:
            return BUY_SIDE;

        case Side::Sell:
            return SELL_SIDE;
    }

    throw std::runtime_error("Invalid order side");
}

Side decode_side(std::uint8_t value){
    switch (value) {
        case BUY_SIDE:
            return Side::Buy;

        case SELL_SIDE:
            return Side::Sell;

        default:
            throw std::runtime_error("Invalid serialized order side");
    }
}

} // namespace

namespace EventSerializer {

std::vector<std::uint8_t> serialize(const MarketEvent& event)
{
    if (!is_valid(event)) {
        throw std::invalid_argument(
            "Cannot serialize invalid MarketEvent"
        );
    }

    Writer writer;

    // File/event header.
    for (std::uint8_t byte : MAGIC) {
        writer.write_u8(byte);
    }

    writer.write_u32(FORMAT_VERSION);

    // Common event fields.
    writer.write_u64(event.event_id);
    writer.write_u64(event.sequence_number);
    writer.write_i64(event.timestamp.count());
    writer.write_symbol(event.symbol);

    // Payload discriminator.
    const EventType type = event_type(event);

    switch (type) {
        case EventType::Trade: {
            writer.write_u8(TRADE_EVENT);

            const auto& payload =
                std::get<TradeEvent>(event.payload);

            writer.write_i64(payload.price);
            writer.write_i64(payload.quantity);

            break;
        }

        case EventType::Quote: {
            writer.write_u8(QUOTE_EVENT);

            const auto& payload =
                std::get<QuoteEvent>(event.payload);

            writer.write_i64(payload.bid_price);
            writer.write_i64(payload.bid_quantity);
            writer.write_i64(payload.asking_price);
            writer.write_i64(payload.asking_quantity);

            break;
        }

        case EventType::Add: {
            writer.write_u8(ADD_ORDER_EVENT);

            const auto& payload =
                std::get<AddOrderEvent>(event.payload);

            writer.write_u64(payload.order_id);
            writer.write_u8(encode_side(payload.side));
            writer.write_i64(payload.price);
            writer.write_i64(payload.quantity);

            break;
        }

        case EventType::Cancel: {
            writer.write_u8(CANCEL_ORDER_EVENT);

            const auto& payload =
                std::get<CancelOrderEvent>(event.payload);

            writer.write_u64(payload.order_id);
            writer.write_i64(payload.quantity);

            break;
        }

        case EventType::Execute: {
            writer.write_u8(EXECUTE_ORDER_EVENT);

            const auto& payload =
                std::get<ExecuteOrderEvent>(event.payload);

            writer.write_u64(payload.order_id);
            writer.write_i64(payload.price);
            writer.write_i64(payload.quantity);

            break;
        }
    }

    return writer.data();
}

MarketEvent deserialize(
    const std::vector<std::uint8_t>& data
)
{
    Reader reader(data);

    // Validate magic.
    for (std::uint8_t expected : MAGIC) {
        if (reader.read_u8() != expected) {
            throw std::runtime_error(
                "Invalid event log magic"
            );
        }
    }

    // Validate format version.
    const std::uint32_t version =
        reader.read_u32();

    if (version != FORMAT_VERSION) {
        throw std::runtime_error(
            "Unsupported event log version"
        );
    }

    // Common event fields.
    const EventId event_id =
        reader.read_u64();

    const SequenceNumber sequence_number =
        reader.read_u64();

    const Timestamp timestamp{
        reader.read_i64()
    };

    const Symbol symbol =
        reader.read_symbol();

    const std::uint8_t type =
        reader.read_u8();

    MarketEvent event{
        .event_id = event_id,
        .sequence_number = sequence_number,
        .timestamp = timestamp,
        .symbol = symbol,
        .payload = TradeEvent{
            .price = 1,
            .quantity = 1
        }
    };

    switch (type) {
        case TRADE_EVENT:
            event.payload = TradeEvent{
                .price = reader.read_i64(),
                .quantity = reader.read_i64()
            };
            break;

        case QUOTE_EVENT:
            event.payload = QuoteEvent{
                .bid_price = reader.read_i64(),
                .bid_quantity = reader.read_i64(),
                .asking_price = reader.read_i64(),
                .asking_quantity = reader.read_i64()
            };
            break;

        case ADD_ORDER_EVENT:
            event.payload = AddOrderEvent{
                .order_id = reader.read_u64(),
                .side = decode_side(reader.read_u8()),
                .price = reader.read_i64(),
                .quantity = reader.read_i64()
            };
            break;

        case CANCEL_ORDER_EVENT:
            event.payload = CancelOrderEvent{
                .order_id = reader.read_u64(),
                .quantity = reader.read_i64()
            };
            break;

        case EXECUTE_ORDER_EVENT:
            event.payload = ExecuteOrderEvent{
                .order_id = reader.read_u64(),
                .price = reader.read_i64(),
                .quantity = reader.read_i64()
            };
            break;

        default:
            throw std::runtime_error(
                "Unknown serialized event type"
            );
    }

    if (!reader.finished()) {
        throw std::runtime_error(
            "Trailing bytes after serialized event"
        );
    }

    if (!is_valid(event)) {
        throw std::runtime_error(
            "Deserialized event is invalid"
        );
    }

    return event;
}

} // namespace EventSerializer