//
// Created by Jimmy on 2026/11/12.
//

#ifndef POWER_CONTROL_BOARD_MEASUREMENTS_H
#define POWER_CONTROL_BOARD_MEASUREMENTS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t vin_raw;
    uint16_t vout_raw;
    uint16_t current_raw;

    uint32_t vdda_mv;
    uint32_t vin_mv;
    uint32_t vout_mv;
    uint32_t current_ma;
} Measurements;

bool Measurements_Init(void);
bool Measurements_Read(Measurements *result);

#endif //POWER_CONTROL_BOARD_MEASUREMENTS_H