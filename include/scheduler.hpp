#pragma once

#include <vector>

#include "timer.hpp"

class Scheduler {
public:
    std::vector<Timer*> timers;

    void step();
};

extern Scheduler current_scheduler;