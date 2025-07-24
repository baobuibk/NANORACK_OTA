// bscript_port_freertos.c
#include "bscript_port.h"
#include "FreeRTOS.h"
#include "task.h"

uint32_t BScript_GetTick(void) {
    return xTaskGetTickCount();
}

void BScript_Delayms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}
