/*
 * tick.h
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#ifndef M4_UTILS_TICK_TICK_H_
#define M4_UTILS_TICK_TICK_H_

#include "stdint.h"
extern volatile uint32_t LL_Tick;

void TickTimer_IRQHandler(void);
uint32_t Utils_GetTick(void);

#endif /* M4_UTILS_TICK_TICK_H_ */
