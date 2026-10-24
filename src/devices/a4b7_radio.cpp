#include "devices.hpp"

//
// Scheduler
//
static Scheduler scheduler_a4b7_radio;

Scheduler& init_scheduler_a4b7_radio() {
    // TODO set up communication with radio
    return scheduler_a4b7_radio;    
}