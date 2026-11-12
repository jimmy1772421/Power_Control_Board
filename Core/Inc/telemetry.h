//
// Created by Jimmy on 2026/11/12.
//

#ifndef POWER_CONTROL_BOARD_TELEMETRY_H
#define POWER_CONTROL_BOARD_TELEMETRY_H

#include <stdbool.h>
#include <stdint.h>
#include "measurements.h"

// Returns true if USB accepted the transmission.
// Call from the main loop only.
bool Telemetry_Send(uint32_t timestamp_ms,
                    const Measurements *measurements);


#endif //POWER_CONTROL_BOARD_TELEMETRY_H