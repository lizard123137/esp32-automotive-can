#pragma once

struct DBCSignal {
    std::string name;

    uint8_t start_bit;
    uint8_t bit_length;

    double factor;
    double offset;
    double value;

    bool is_signed;
    bool is_little_endian;
};