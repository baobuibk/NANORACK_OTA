/************************************************
 *  @file     : alive_cm4.c
 *  @date     : Jul 23, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/

#include "alive_cm4.h"
#include "main.h"
#include "Macro/macro.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SysLog/syslog.h"

#define KEEPALIVE_OUT_PORT    STMOUT_CM4IN_SDA_GPIO_Port
#define KEEPALIVE_OUT_PIN     STMOUT_CM4IN_SDA_Pin

#define KEEPALIVE_IN_PORT     CM4OUT_STMIN_D0_GPIO_Port
#define KEEPALIVE_IN_PIN      CM4OUT_STMIN_D0_Pin

#define CM4_RST_PORT          MCU_IO_RESET_CM4_GPIO_Port
#define CM4_RST_PIN           MCU_IO_RESET_CM4_Pin

static uint16_t cm4_miss_count = 0;

void CM4_KeepAliveTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        GPIO_SetLow(KEEPALIVE_OUT_PORT, KEEPALIVE_OUT_PIN);

        uint8_t respondedLow = 0;
        TickType_t start = xTaskGetTickCount();
        while (xTaskGetTickCount() - start < pdMS_TO_TICKS(RESPONSE_TIMEOUT_MS)) {
            if (GPIO_IsInLow(KEEPALIVE_IN_PORT, KEEPALIVE_IN_PIN)) {
                respondedLow = 1;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        GPIO_SetHigh(KEEPALIVE_OUT_PORT, KEEPALIVE_OUT_PIN);

        uint8_t respondedHigh = 0;
        start = xTaskGetTickCount();
        while (xTaskGetTickCount() - start < pdMS_TO_TICKS(RESPONSE_TIMEOUT_MS)) {
            if (GPIO_IsInHigh(KEEPALIVE_IN_PORT, KEEPALIVE_IN_PIN)) {
                respondedHigh = 1;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        if (respondedLow && respondedHigh) {
            cm4_miss_count = 0;
        } else {
            cm4_miss_count++;
        }

        if (cm4_miss_count >= MAX_RETRY_COUNT) {
            SYSLOG_ERROR("CM4 unresponsive. Triggering reset.");
            GPIO_SetLow(CM4_RST_PORT, CM4_RST_PIN);
            vTaskDelay(pdMS_TO_TICKS(100));
            GPIO_SetHigh(CM4_RST_PORT, CM4_RST_PIN);
            cm4_miss_count = 0;
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CHECK_INTERVAL_MS));
    }
}

uint16_t CM4_GetMissCount(void)
{
    return cm4_miss_count;
}


