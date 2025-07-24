/*
 * gpio_state.c
 *
 *  Created on: Mar 5, 2025
 *      Author: CAO HIEU
 */

#include "utils.h"
#include "main.h"
#include "board.h"
#include "gpio_state.h"

void toCM4_Init(void) {
//	GPIO_SetHigh(STATE_toCM4_BUSY_Port, STATE_toCM4_BUSY);
//	GPIO_SetHigh(STATE_toCM4_READYSEND_Port, STATE_toCM4_READYSEND);
    toCM4_SetState(TOCM4_IDLE);
}

void toCM4_SetState(toCM4_State_t state) {
    switch (state) {
        case TOCM4_ERROR:
//            GPIO_SetLow(STATE_toCM4_BUSY_Port, STATE_toCM4_BUSY);
//            GPIO_SetLow(STATE_toCM4_READYSEND_Port, STATE_toCM4_READYSEND);
            break;
        case TOCM4_READYSEND:
//            GPIO_SetLow(STATE_toCM4_BUSY_Port, STATE_toCM4_BUSY);
//            GPIO_SetHigh(STATE_toCM4_READYSEND_Port, STATE_toCM4_READYSEND);
            break;
        case TOCM4_BUSY:
//            GPIO_SetHigh(STATE_toCM4_BUSY_Port, STATE_toCM4_BUSY);
//            GPIO_SetLow(STATE_toCM4_READYSEND_Port, STATE_toCM4_READYSEND);
            break;
        case TOCM4_IDLE:
//            GPIO_SetHigh(STATE_toCM4_BUSY_Port, STATE_toCM4_BUSY);
//            GPIO_SetHigh(STATE_toCM4_READYSEND_Port, STATE_toCM4_READYSEND);
            break;
        default:
//            GPIO_SetLow(STATE_toCM4_BUSY_Port, STATE_toCM4_BUSY);
//            GPIO_SetLow(STATE_toCM4_READYSEND_Port, STATE_toCM4_READYSEND);
            break;
    }
}

toCM4_State_t toCM4_GetState(void) {
//    uint8_t busy_state = GPIO_IsOutHigh(STATE_toCM4_BUSY_Port, STATE_toCM4_BUSY);
//    uint8_t readysend_state = GPIO_IsOutHigh(STATE_toCM4_READYSEND_Port, STATE_toCM4_READYSEND);

//
//    if (busy_state == 0 && readysend_state == 0) {
//        return TOCM4_ERROR;
//    } else if (busy_state == 0 && readysend_state == 1) {
//        return TOCM4_READYSEND;
//    } else if (busy_state == 1 && readysend_state == 0) {
//        return TOCM4_BUSY;
//    } else { // busy_state == 1 && readysend_state == 1
//        return TOCM4_IDLE;
//    }
//
    return TOCM4_IDLE;
}
