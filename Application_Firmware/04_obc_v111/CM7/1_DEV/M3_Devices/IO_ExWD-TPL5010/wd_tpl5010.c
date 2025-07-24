/*
 * wd_tpl5010.c
 *
 *  Created on: Mar 6, 2025
 *      Author: CAO HIEU
 */

#include "main.h"
#include "board.h"
#include "wd_tpl5010.h"

static Watchdog_StateEnum currentState = WATCHDOG_STATE_LOW;

Std_ReturnType Watchdog_Device_Init(void)
{
    currentState = WATCHDOG_STATE_LOW;
    LL_GPIO_ResetOutputPin(WD_Done_Port, WD_Done_Pin);
    return E_OK;
}

void Watchdog_Device_Update(void)
{
    if(currentState == WATCHDOG_STATE_LOW)
    {
        currentState = WATCHDOG_STATE_HIGH;
        LL_GPIO_SetOutputPin(WD_Done_Port, WD_Done_Pin);
    }
    else
    {
        currentState = WATCHDOG_STATE_LOW;
        LL_GPIO_ResetOutputPin(WD_Done_Port, WD_Done_Pin);
    }
}

Watchdog_StateEnum Watchdog_Device_GetState(void)
{
    return currentState;
}
