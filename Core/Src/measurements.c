//
// Created by Jimmy on 2026/11/12.
//

#include "measurements.h"
#include "adc.h"

#define VOLTAGE_DIVIDER_RATIO 21.0f

#define CURRENT_SHUNT_OHMS   0.010f
#define CURRENT_MONITOR_GM   0.001f
#define CURRENT_LOAD_OHMS \
    ((43200.0f * 49900.0f) / (43200.0f + 49900.0f))

#define CURRENT_VOLTS_PER_AMP \
    (CURRENT_SHUNT_OHMS * CURRENT_MONITOR_GM * CURRENT_LOAD_OHMS)

static bool ReadChannel(uint32_t channel, uint16_t *raw)
{
    ADC_ChannelConfTypeDef config = {0};

    config.Channel = channel;
    config.Rank = ADC_REGULAR_RANK_1;
    config.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    config.SingleDiff = ADC_SINGLE_ENDED;
    config.OffsetNumber = ADC_OFFSET_NONE;
    config.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &config) != HAL_OK) {
        return false;
    }

    // Allow the internal reference to settle after being enabled.
    if (channel == ADC_CHANNEL_VREFINT) {
        HAL_Delay(1);
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return false;
    }

    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) {
        HAL_ADC_Stop(&hadc1);
        return false;
    }

    *raw = (uint16_t)HAL_ADC_GetValue(&hadc1);

    return HAL_ADC_Stop(&hadc1) == HAL_OK;
}

bool Measurements_Init(void)
{
    return HAL_ADCEx_Calibration_Start(
        &hadc1, ADC_SINGLE_ENDED
    ) == HAL_OK;
}

bool Measurements_Read(Measurements *result)
{
    if (result == NULL) {
        return false;
    }

    Measurements next = {0};
    uint16_t vref_raw = 0;

    if (!ReadChannel(ADC_CHANNEL_VREFINT, &vref_raw) ||
        vref_raw == 0) {
        return false;
    }

    next.vdda_mv = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(
        vref_raw, ADC_RESOLUTION_12B
    );

    if (!ReadChannel(ADC_CHANNEL_1, &next.vin_raw) ||
        !ReadChannel(ADC_CHANNEL_2, &next.vout_raw) ||
        !ReadChannel(ADC_CHANNEL_3, &next.current_raw)) {
        return false;
    }

    const float mv_per_count = (float)next.vdda_mv / 4095.0f;

    next.vin_mv = (uint32_t)(
        next.vin_raw * mv_per_count * VOLTAGE_DIVIDER_RATIO + 0.5f
    );

    next.vout_mv = (uint32_t)(
        next.vout_raw * mv_per_count * VOLTAGE_DIVIDER_RATIO + 0.5f
    );

    next.current_ma = (uint32_t)(
        next.current_raw * mv_per_count / CURRENT_VOLTS_PER_AMP + 0.5f
    );

    *result = next;
    return true;
}