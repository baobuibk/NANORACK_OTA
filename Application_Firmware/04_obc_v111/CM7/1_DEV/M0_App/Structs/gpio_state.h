/*
 * gpio.h
 *
 *  Created on: Mar 5, 2025
 *      Author: CAO HIEU
 */

#ifndef M0_APP_STRUCTS_GPIO_STATE_H_
#define M0_APP_STRUCTS_GPIO_STATE_H_

typedef enum {
    TOCM4_ERROR = 0,      // BUSY = 0, READYSEND = 0
    TOCM4_READYSEND = 1,  // BUSY = 0, READYSEND = 1
    TOCM4_BUSY = 2,       // BUSY = 1, READYSEND = 0
    TOCM4_IDLE = 3        // BUSY = 1, READYSEND = 1
} toCM4_State_t;

void toCM4_Init(void);

void toCM4_SetState(toCM4_State_t state);

toCM4_State_t toCM4_GetState(void);

#endif /* M0_APP_STRUCTS_GPIO_STATE_H_ */
