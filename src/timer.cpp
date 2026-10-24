#include "timer.hpp"

#include <Arduino.h>

int Timer::run() {
    uint64_t now = millis();
    if (now > this->last_timestamp + this->interval) {
        // TODO maybe name timer
        Logger::print(DEBUG, "Timer", "Running timer with interval %llu", this->interval);

        for (const auto& message : this->messages) {
            message->sendMessage();
        }

        // TODO maybe support multiple counters per timer
        if (this->counter) {
            this->counter->increment();
        }

        this->last_timestamp = now;
        return 0;
    }
    return 1;
}