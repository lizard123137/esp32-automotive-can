#pragma once

#include <cstdint>
#include <vector>

#include "counter.hpp"
#include "dbc/dbc_message.hpp"

class Timer {
public:
    Counter *counter;
    uint64_t interval;

    std::vector<DBCMessage*> messages;

    Timer(uint64_t interval, Counter *counter = nullptr)
        : interval(interval),
          counter(counter) {}

    int run();
private:
    uint64_t last_timestamp;
};