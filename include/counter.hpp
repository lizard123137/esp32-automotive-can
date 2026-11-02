#pragma once

#include <cstdint>
#include <type_traits>

class Counter {
public:
    uint8_t value;
    uint8_t min_value;
    uint8_t max_value;
    uint8_t bit_length;
    uint8_t step;

    Counter(uint8_t min_value, uint8_t max_value, size_t bit_length, uint8_t increment = 1)
        : value(min_value),
        min_value(min_value),
        max_value(max_value),
        bit_length(bit_length),
        step(increment) {}

    void increment() {
        const uint16_t range = static_cast<uint16_t>(max_value) - min_value + 1;
        value = min_value + (static_cast<uint16_t>(value - min_value) + step) % range;
    }
};