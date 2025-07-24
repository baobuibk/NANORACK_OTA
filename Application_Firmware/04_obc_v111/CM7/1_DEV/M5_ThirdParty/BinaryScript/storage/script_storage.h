/*************************************************
 * @file     : script_storage.h
 * @date     : Jul 21, 2025
 * @author   : CAO HIEU
 *-----------------------------------------------
 * Description :
 *   Script Storage Manager for FRAM persistence
 *   Header file with types and function declarations
 ************************************************/

#ifndef SCRIPT_STORAGE_H
#define SCRIPT_STORAGE_H

#include "SPI_FRAM/fram_spi.h"
#include <stdint.h>
#include <stdbool.h>
#include "ScriptManager/script_manager.h"
/*************************************************
 *                TYPE DEFINITIONS               *
 *************************************************/

/**
 * @brief Script storage result codes
 */
typedef enum {
    STORAGE_SUCCESS = 0,
    STORAGE_ERROR_NOT_INITIALIZED,
    STORAGE_ERROR_INVALID_PARAMS,
    STORAGE_ERROR_INVALID_SIZE,
    STORAGE_ERROR_INVALID_MAGIC,
    STORAGE_ERROR_CRC_MISMATCH,
    STORAGE_ERROR_FRAM_READ,
    STORAGE_ERROR_FRAM_WRITE,
    STORAGE_ERROR_BUFFER_TOO_SMALL,
    STORAGE_ERROR_SCRIPT_NOT_FOUND
} ScriptStorageResult_t;

/**
 * @brief Script storage status structure
 */
typedef struct {
    _Bool is_initialized;                    //!< Storage manager initialized
    _Bool auto_load_enabled;                 //!< Auto-load feature enabled
    uint32_t auto_load_delay_sec;           //!< Auto-load delay in seconds
    _Bool script_exists[SCRIPT_TYPE_COUNT]; //!< Script existence flags
    uint32_t fram_base_address;             //!< FRAM base address for scripts
    uint32_t script_slot_size;              //!< Size of each script slot
    uint32_t max_script_size;               //!< Maximum script data size
} ScriptStorageStatus_t;

/**
 * @brief Script storage manager structure
 */
typedef struct {
    _Bool is_initialized;                   //!< Initialization status
    FRAM_SPI_HandleTypeDef* fram_handle;    //!< FRAM handle
    _Bool auto_load_enabled;                //!< Auto-load enabled flag
    uint32_t auto_load_delay_sec;          //!< Auto-load delay in seconds
} ScriptStorageManager_t;

/*************************************************
 *              FUNCTION DECLARATIONS            *
 *************************************************/

/**
 * @brief Initialize script storage manager
 * @return true if successful, false otherwise
 */
_Bool ScriptStorage_Init(void);

/**
 * @brief Save script to FRAM
 * @param type Script type (INIT, DLS, CAM)
 * @param binary_data Pointer to binary script data
 * @param size Size of binary data in bytes
 * @return Storage result code
 * 
 * @note Script is saved with format: [Magic Code][Length][Data][CRC16]
 *       Magic codes: INIT=0x494E, DLS=0x444C, CAM=0x4341
 *       Each script slot is 5KB in FRAM
 */
ScriptStorageResult_t ScriptStorage_SaveScript(ScriptType_t type, const uint8_t* binary_data, uint32_t size);

/**
 * @brief Load script from FRAM
 * @param type Script type to load
 * @param binary_data Output buffer for binary data
 * @param max_size Maximum size of output buffer
 * @param actual_size Pointer to store actual loaded size
 * @return Storage result code
 * 
 * @note Validates magic code, size, and CRC16 before returning data
 */
ScriptStorageResult_t ScriptStorage_LoadScript(ScriptType_t type, uint8_t* binary_data, 
                                             uint32_t max_size, uint32_t* actual_size);

/**
 * @brief Check if script exists and is valid in FRAM
 * @param type Script type to check
 * @return true if script exists and passes basic validation, false otherwise
 * 
 * @note Only checks magic code and size, does not verify CRC
 */
_Bool ScriptStorage_ScriptExists(ScriptType_t type);

/**
 * @brief Erase script from FRAM
 * @param type Script type to erase
 * @return Storage result code
 * 
 * @note Invalidates script by clearing magic code
 */
ScriptStorageResult_t ScriptStorage_EraseScript(ScriptType_t type);

/**
 * @brief Auto-load all scripts from FRAM with delay
 * @param script_manager Pointer to script manager (for loading scripts)
 * @return Number of scripts successfully loaded (0-3)
 * 
 * @note Waits for specified delay before loading to allow user input
 *       If all scripts loaded successfully, starts execution automatically
 *       Otherwise requests missing scripts from master
 */
uint8_t ScriptStorage_AutoLoadScripts(void* script_manager);

/**
 * @brief Get storage status information
 * @param status Pointer to status structure to fill
 */
void ScriptStorage_GetStatus(ScriptStorageStatus_t* status);

/**
 * @brief Print detailed storage status to log
 */
void ScriptStorage_PrintStatus(void);

/**
 * @brief Enable or disable auto-load feature
 * @param enabled true to enable auto-load, false to disable
 */
void ScriptStorage_SetAutoLoadEnabled(_Bool enabled);

/**
 * @brief Set auto-load delay duration
 * @param delay_sec Delay in seconds before auto-loading starts
 */
void ScriptStorage_SetAutoLoadDelay(uint32_t delay_sec);

/*************************************************
 *              UTILITY FUNCTIONS                *
 *************************************************/

/**
 * @brief Get storage result description string
 * @param result Storage result code
 * @return Human-readable description string
 */
static inline const char* ScriptStorage_GetResultString(ScriptStorageResult_t result)
{
    switch (result) {
        case STORAGE_SUCCESS:
            return "Success";
        case STORAGE_ERROR_NOT_INITIALIZED:
            return "Storage not initialized";
        case STORAGE_ERROR_INVALID_PARAMS:
            return "Invalid parameters";
        case STORAGE_ERROR_INVALID_SIZE:
            return "Invalid script size";
        case STORAGE_ERROR_INVALID_MAGIC:
            return "Invalid magic code";
        case STORAGE_ERROR_CRC_MISMATCH:
            return "CRC mismatch";
        case STORAGE_ERROR_FRAM_READ:
            return "FRAM read error";
        case STORAGE_ERROR_FRAM_WRITE:
            return "FRAM write error";
        case STORAGE_ERROR_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case STORAGE_ERROR_SCRIPT_NOT_FOUND:
            return "Script not found";
        default:
            return "Unknown error";
    }
}

/*************************************************
 *                  CONSTANTS                    *
 *************************************************/

// FRAM memory layout constants (for reference)
#define SCRIPT_STORAGE_FRAM_SIZE        (256 * 1024)    // 256KB total FRAM
#define SCRIPT_STORAGE_SECTION_SIZE     (16 * 1024)     // 16KB for scripts  
#define SCRIPT_STORAGE_SLOT_SIZE        (5 * 1024)      // 5KB per script
#define SCRIPT_STORAGE_MIN_SIZE         10               // Minimum script size
#define SCRIPT_STORAGE_MAX_SIZE         (SCRIPT_STORAGE_SLOT_SIZE - 8)  // Max data size (5KB - header - CRC)

// Default timing constants
#define SCRIPT_STORAGE_DEFAULT_AUTOLOAD_DELAY   15      // 15 seconds default delay

#endif /* SCRIPT_STORAGE_H */
