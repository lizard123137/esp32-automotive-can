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