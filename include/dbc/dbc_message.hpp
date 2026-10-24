#pragma once

#include <cstdio>
#include <mcp_can.h>

#include <algorithm>
#include <functional>
#include <vector>

#include "dbc_signal.hpp"
#include "../can.hpp"
#include "../counter.hpp"
#include "../logger.hpp"

class DBCMessage {
public:
    std::string name;

    uint32_t id;
    uint8_t dlc;
    
    std::array<uint8_t, 8> default_payload{};
    std::vector<DBCSignal> signals;

    std::function<uint8_t(const uint8_t*, size_t, uint8_t)> crc_func = nullptr;

    Counter *counter;
    uint8_t counter_start_bit = 0;

    DBCMessage() = default;
    DBCMessage(
        std::string name,
        uint32_t id,
        uint8_t dlc,
        Counter *counter = nullptr
    ) : name(std::move(name)),
        id(id),
        dlc(dlc),
        counter(counter) {}

    std::array<uint8_t, 8> getBytes() {
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

        if (this->counter) {
            for (uint8_t i = 0; i < this->counter->bit_length; ++i) {
                const uint16_t bit_position = this->counter_start_bit + i;
                const uint16_t byte_idx = bit_position / 8;
                const uint8_t bit_idx = bit_position % 8;

                if (byte_idx >= dlc || byte_idx >= bytes.size()) {
                    break;
                }

                uint8_t mask = 1u << bit_idx;

                bytes[byte_idx] &= static_cast<uint8_t>(~mask);

                if ((this->counter->value >> i) & 1u) {
                    bytes[byte_idx] |= mask;
                }
            }
        }
        
        return bytes;
    }

    void sendMessage(uint8_t final_xor = 0) {
        auto bytes = this->getBytes();
        uint8_t len = this->dlc;

        if (this->crc_func) {
            uint8_t crc = this->crc_func(bytes.data(), this->dlc, final_xor);
            std::move_backward(bytes.begin(), bytes.end() - 1, bytes.end());
            bytes[0] = crc;
            len++;
        }

        // Log message
        std::string message_str;
        for (size_t i = 0; i < len; i++) {
            char buf[3];
            std::sprintf(buf, "%02x ", bytes[i]);
            message_str += buf;
        }
        Logger::print(DEBUG, "DBC_MESSAGE", "Data sent to ID %x - %s", this->id, message_str.c_str());

        CAN.sendMsgBuf(this->id, 0, len, bytes.data());
    }
};