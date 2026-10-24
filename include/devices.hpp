#pragma once

/**
 *
 * This file contains function prototypes for creating device schedulers.
 * A device scheduler is used for communicating with an automotive peripheral.
 *
 * To add support for a new device you will need to know the CAN messages it receives.
 *
 * In the future the goal will be to create a GUI for creating schedulers dynamically,
 * but for now this will be more than enough to showcase interacting with devices.
 *
 */

#include "scheduler.hpp"

// Device scheduler function prototypes:
Scheduler& init_scheduler_f30_cluster(void);
Scheduler& init_scheduler_a4b7_radio(void);