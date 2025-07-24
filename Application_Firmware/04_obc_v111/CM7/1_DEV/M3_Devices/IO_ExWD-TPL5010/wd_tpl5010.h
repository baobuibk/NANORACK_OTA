/*
 * wd_tpl5010.h
 *
 *  Created on: Mar 6, 2025
 *      Author: CAO HIEU
 */

#ifndef M3_DEVICES_IO_EXWD_TPL5010_WD_TPL5010_H_
#define M3_DEVICES_IO_EXWD_TPL5010_WD_TPL5010_H_

#include "utils.h"

#define HIGH_PERIOD   200
#define LOW_PERIOD    600

typedef enum {
    WATCHDOG_STATE_LOW = 0,
    WATCHDOG_STATE_HIGH = 1
} Watchdog_StateEnum;

Std_ReturnType Watchdog_Device_Init(void);

void Watchdog_Device_Update(void);

Watchdog_StateEnum Watchdog_Device_GetState(void);


#endif /* M3_DEVICES_IO_EXWD_TPL5010_WD_TPL5010_H_ */
