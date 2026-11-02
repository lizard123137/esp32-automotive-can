#pragma once

#include <cstdint>
#include <vector>

#include "counter.hpp"
#include "dbc/dbc_message.hpp"

class Timer {
public:
    uint64_t interval;

    std::vector<DBCMessage*> messages;
    std::vector<Counter*> counters;

    Timer(uint64_t interval)
        : interval(interval) {}

    int run();
private:
    uint64_t last_timestamp;
};