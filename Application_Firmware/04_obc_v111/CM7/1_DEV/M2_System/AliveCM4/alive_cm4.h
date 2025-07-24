/************************************************
 *  @file     : alive_cm4.h
 *  @date     : Jul 23, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/

#ifndef M2_SYSTEM_ALIVECM4_ALIVE_CM4_H_
#define M2_SYSTEM_ALIVECM4_ALIVE_CM4_H_

#include <stdint.h>

#define MAX_RETRY_COUNT 120
#define CHECK_INTERVAL_MS 5000
#define RESPONSE_TIMEOUT_MS 500

void CM4_KeepAliveTask(void *pvParameters);
uint16_t CM4_GetMissCount(void);

#endif /* M2_SYSTEM_ALIVECM4_ALIVE_CM4_H_ */
