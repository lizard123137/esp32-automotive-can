#include "scheduler.hpp"
#include "timer.hpp"

void Scheduler::step() {
    for (const auto& timer : this->timers) {
        int executed = timer->run(); // Returns 0 if the timer ran
        if (!executed) {
            // TODO somehow keep track of counters
            // Maybe add a `bool increments_counter` to timer and if it has that and returned 0 increment it.
        }
    }
}

int Scheduler::updateSignalValue(std::string signal_name, double value) {
    for (const auto& timer : this->timers) {
        for (const auto& message : timer->messages) {
            for (auto& signal : message->signals) {
                if (signal.name == signal_name) {
                    signal.value = value;
                    return 0; // Success
                }
            }
        }
    }
    return -1; // Error: Signal not found
}