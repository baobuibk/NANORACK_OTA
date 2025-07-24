/*
 * tick.c
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */
#include "tick.h"
#include "stm32h7xx.h"
#include "stm32h7xx_ll_tim.h"
#include "CLI_Terminal/CLI_Auth/simple_shield.h"

extern ShieldInstance_t auth_usb;

volatile uint32_t LL_Tick = 0;

void TickTimer_IRQHandler(void) {
    if (LL_TIM_IsActiveFlag_UPDATE(TIM1)) {
        LL_TIM_ClearFlag_UPDATE(TIM1);
        LL_Tick++;
        Shield_UpdateTimer(&auth_usb);
    }
}

uint32_t Utils_GetTick(void) {
    return LL_Tick;
}
