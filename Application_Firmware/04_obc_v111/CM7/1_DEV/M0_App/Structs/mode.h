/*
 * mode.h
 *
 *  Created on: Mar 3, 2025
 *      Author: CAO HIEU
 */

#ifndef M0_APP_STRUCTS_MODE_H_
#define M0_APP_STRUCTS_MODE_H_

#include "stdint.h"

typedef enum {
    FORWARD_MODE_NORMAL = 0,
    FORWARD_MODE_UART,
    FORWARD_MODE_USB,
    FORWARD_MODE_LISTEN_CM4,
    FORWARD_MODE_LISTEN_USB
} ForwardMode_t;

void ForwardMode_Set(ForwardMode_t mode);

ForwardMode_t ForwardMode_Get(void);

_Bool ForwardMode_ProcessReceivedByte(uint8_t byte);


#endif /* M0_APP_STRUCTS_MODE_H_ */
