#pragma once

#include <type_traits>

class Counter {
public:
    uint8_t value;
    uint8_t min_value;
    uint8_t max_value;
    uint8_t bit_length;

    Counter(uint8_t min_value, uint8_t max_value, size_t bit_length)
        : value(min_value),
        min_value(min_value),
        max_value(max_value),
        bit_length(bit_length) {}

    void increment() {
        if (value < max_value) {
            ++value;
        } else {
            value = min_value;
        }
    }
};