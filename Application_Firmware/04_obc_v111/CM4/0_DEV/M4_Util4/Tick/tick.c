/*
 * tick.c
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */
#include "tick.h"
#include "stm32h7xx.h"
#include "stm32h7xx_ll_tim.h"

volatile uint32_t LL_Tick = 0;

void TickTimer_IRQHandler(void) {
    if (LL_TIM_IsActiveFlag_UPDATE(TIM2)) {
        LL_TIM_ClearFlag_UPDATE(TIM2);
        LL_Tick++;
    }
}

uint32_t Utils_GetTick(void) {
    return LL_Tick;
}
