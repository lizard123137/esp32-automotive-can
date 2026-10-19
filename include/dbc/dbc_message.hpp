#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "dbc_signal.hpp"

class DBCMessage {
public:
    std::string name;

    MCP_CAN& canbus;
    
    uint32_t id;
    uint8_t dlc;
    
    std::array<uint8_t, 8> default_payload{};
    std::vector<DBCSignal> signals;

    bool use_crc = false;
    std::function<uint8_t(const uint8_t*, size_t, uint8_t)> crc_func;

    bool use_counter = false;
    uint8_t counter_start_bit = 0;
    uint8_t counter_bit_len = 4;

    DBCMessage() = default;
    DBCMessage(
        std::string name,
        uint32_t id,
        uint8_t dlc,
        MCP_CAN& canbus
    ) : name(std::move(name)),
        id(id),
        dlc(dlc),
        canbus(canbus) {}

    std::array<uint8_t, 8> getBytes(uint8_t counter = 0) {
        auto bytes = this->default_payload;

        for (const auto& signal : this->signals) {
            if (signal.bit_length == 0 || signal.bit_length > 64) {
                continue;
            }

            const double raw_double = (signal.value - signal.offset) / signal.factor;
            const uint64_t raw = static_cast<uint64_t>(raw_double);

            for (uint16_t i = 0; i < signal.bit_length; ++i) {
                const uint16_t bit_position = signal.start_bit + i;
                const uint16_t byte_idx = bit_position / 8;
                const uint8_t bit_idx = bit_position % 8;

                if (byte_idx >= dlc || byte_idx >= bytes.size()) {
                    break;
                }

                const uint8_t mask = 1u << bit_idx;
                const bool bit_value = (raw >> i) & 1u;

                bytes[byte_idx] &= static_cast<uint8_t>(~mask);

                if (bit_value) {
                    bytes[byte_idx] |= mask;
                }
            }
        }

        if (this->use_counter) {
            for (uint8_t i = 0; i < this->counter_bit_len; ++i) {
                const uint16_t bit_position = this->counter_start_bit + i;
                const uint16_t byte_idx = bit_position / 8;
                const uint8_t bit_idx = bit_position % 8;

                if (byte_idx >= dlc || byte_idx >= bytes.size()) {
                    break;
                }

                uint8_t mask = 1u << bit_idx;

                bytes[byte_idx] &= static_cast<uint8_t>(~mask);

                if ((counter >> i) & 1u) {
                    bytes[byte_idx] |= mask;
                }
            }
        }

        return bytes;
    }

    void sendMessage(uint8_t final_xor = 0, uint8_t counter = 0) {
        auto bytes = this->getBytes(counter);
        uint8_t len = this->dlc;

        if (this->use_crc) {
            uint8_t crc = this->crc_func(bytes.data(), this->dlc, final_xor);
            std::move_backward(bytes.begin(), bytes.end() - 1, bytes.end());
            bytes[0] = crc;
            len++;
        }

        this->canbus.sendMsgBuf(this->id, 0, len, bytes.data());
    }
};