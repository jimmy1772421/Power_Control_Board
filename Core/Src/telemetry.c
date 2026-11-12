//
// Created by Jimmy on 2026/11/12.
//

#include "telemetry.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

#include <stdio.h>
#include <string.h>

extern USBD_HandleTypeDef hUsbDeviceFS;

#define TELEMETRY_BUFFER_SIZE 96U

// Must remain valid until the USB transfer finishes.
static uint8_t tx_buffer[TELEMETRY_BUFFER_SIZE];

bool Telemetry_Send(uint32_t timestamp_ms,
                    const Measurements *measurements)
{
    if (measurements == NULL) {
        return false;
    }

    // Format separately so we never overwrite an active USB buffer.
    char line[TELEMETRY_BUFFER_SIZE];

    int length = snprintf(
        line,
        sizeof(line),
        "%lu,%lu,%lu,%lu\r\n",
        (unsigned long)timestamp_ms,
        (unsigned long)measurements->vin_mv,
        (unsigned long)measurements->vout_mv,
        (unsigned long)measurements->current_ma
    );

    if (length <= 0 || (size_t)length >= sizeof(line)) {
        return false;
    }

    bool accepted = false;

    // Briefly protect the state check and submission from USB
    // reset/disconnect interrupts changing the class data midway.
    uint32_t saved_primask = __get_PRIMASK();
    __disable_irq();

    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
        USBD_CDC_HandleTypeDef *cdc =
            (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;

        if (cdc != NULL && cdc->TxState == 0U) {
            memcpy(tx_buffer, line, (size_t)length);

            accepted = (
                CDC_Transmit_FS(tx_buffer, (uint16_t)length)
                == USBD_OK
            );
        }
    }

    __set_PRIMASK(saved_primask);

    return accepted;
}