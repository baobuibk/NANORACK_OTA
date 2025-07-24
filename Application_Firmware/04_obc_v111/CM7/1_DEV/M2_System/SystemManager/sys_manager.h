/************************************************
 *  @file     : sys_manager.h
 *  @date     : May 10, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/

#ifndef M2_SYSTEM_SYSTEMMANAGER_SYS_MANAGER_H_
#define M2_SYSTEM_SYSTEMMANAGER_SYS_MANAGER_H_

#include "Define/define.h"

#define SYS_MANAGER

typedef enum
{
    Sys_OK       = 0x00U,  /* Operation successful */
    Sys_ERROR    = 0x01U,  /* Operation failed */
    Sys_BUSY     = 0x02U,  /* Resource is busy */
    Sys_TIMEOUT  = 0x03U   /* Operation timed out */
} ErrorCode_t;

typedef enum {
    PERIPH_STATE_UNINIT,
    PERIPH_STATE_INIT,
    PERIPH_STATE_ERROR
} PeriphState_t;

typedef enum {
    PERIPH_TYPE_GPIO,
    PERIPH_TYPE_SDMMC,
    PERIPH_TYPE_USB,
} PeriphType_t;

typedef struct {
    uint8_t id;
    const char* name;
    PeriphState_t state;
    uint8_t retry_count;
    ErrorCode_t errorCode;
    void (*init_func)(void);
    void (*recover_func)(void);
} PeriphDescriptor_t;



void PeriphManager_InitAll(void);
Std_ReturnType PeriphManager_InitById(uint32_t id);
void PeriphManager_MonitorTask(void *pvParameters);
PeriphState_t PeriphManager_GetState(uint32_t id);


#endif /* M2_SYSTEM_SYSTEMMANAGER_SYS_MANAGER_H_ */
