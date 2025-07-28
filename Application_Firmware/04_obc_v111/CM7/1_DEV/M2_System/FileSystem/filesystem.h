/*
 * filesystem.h - Simplified Version
 *
 *  Created on: Mar 2, 2025
 *      Author: CAO HIEU
 */

#ifndef M1_DRIVERS_FILESYSTEM_FILESYSTEM_H_
#define M1_DRIVERS_FILESYSTEM_FILESYSTEM_H_

#include "main.h"
#include "CLI_Terminal/CLI_Command/cli_command.h"
#include "utils.h"


typedef enum{
    SDFS_READY = 0,
    SDFS_RELEASE,
    SDFS_BUSY,
    SDFS_ERROR
}SDFS_StateTypedef;
extern SDFS_StateTypedef SDFS_State;

/*************************************************
 *              Core API Functions               *
 *************************************************/

/**
 * @brief Initialize filesystem subsystem
 */
void FS_Init(void);

/**
 * @brief Create and start async filesystem task
 * @note Call this only if you want to use FS_Write_Async()
 */
void FS_Async_Task(void *pvParameters);

/**
 * @brief Write data to file directly (BLOCKING - Recommended)
 * @param filename Target filename
 * @param buffer Data buffer to write
 * @param size Size of data in bytes
 * @return E_OK: Success, E_BUSY: Filesystem busy, E_ERROR: Failed
 * @note Fast operation with SDMMC DMA (~1-5ms for 8KB)
 */
Std_ReturnType FS_Write_Direct(const char* filename, uint8_t* buffer, uint32_t size);

/**
 * @brief Write data to file asynchronously (NON-BLOCKING)
 * @param filename Target filename
 * @param buffer Data buffer to write (must remain valid until completion)
 * @param size Size of data in bytes
 * @param result_ptr Pointer to store result (optional, can be NULL)
 * @return E_PENDING: Queued successfully, E_BUSY: Queue full, E_ERROR: Failed
 * @note Requires FS_Async_Task() to be running
 */
Std_ReturnType FS_Write_Async(const char* filename, uint8_t* buffer, uint32_t size,
                              volatile Std_ReturnType* result_ptr);

/**
 * @brief Legacy write function for backward compatibility
 * @note Maps to FS_Write_Direct()
 */
Std_ReturnType FS_Request_Write(const char* filename, uint8_t* buffer, uint32_t size);

/*************************************************
 *              Hardware Control                 *
 *************************************************/
void SD_Lockin(void);
void SD_Release(void);
Std_ReturnType Link_SDFS_Driver(void);

/*************************************************
 *              CLI Commands                     *
 *************************************************/
void FS_ListFiles_path(EmbeddedCli *cli);
int Vim_SDFS(EmbeddedCli *cli, const char *filename, const char *content);
int Cat_SDFS(EmbeddedCli *cli, const char *filename);

/*************************************************
 *              Return Codes                     *
 *************************************************/
/* Additional return codes for filesystem */
#ifndef E_BUSY
#define E_BUSY      2   /* Filesystem busy, try again */
#endif

#ifndef E_PENDING
#define E_PENDING   3   /* Operation queued successfully */
#endif

_Bool NeedCleanup(void);
Std_ReturnType CreateOrUpdateDateFile(void);
Std_ReturnType PerformCleanup(void);

#endif /* M1_DRIVERS_FILESYSTEM_FILESYSTEM_H_ */
