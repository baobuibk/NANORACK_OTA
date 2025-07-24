#ifndef _LWL_H_
#define _LWL_H_

/*
 * @brief Lightweight Logging (LWL) for OBC STM32
 * 
 * This module provides efficient binary logging for OBC STM32.
 * Integrates with Log Manager for data transmission.
 */

#include <stdbool.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Configuration
////////////////////////////////////////////////////////////////////////////////

#define LWL_START_BYTE 0xAA // Start byte for each log record
#define LWL_MAX_PACKAGE_SIZE 64 // Maximum size for a single log package

////////////////////////////////////////////////////////////////////////////////
// Macros for argument packing
////////////////////////////////////////////////////////////////////////////////

#define LWL_1(a) (uint32_t)(a)
#define LWL_2(a) (uint32_t)(a), (uint32_t)(a) >> 8
#define LWL_3(a) (uint32_t)(a), (uint32_t)(a) >> 8, (uint32_t)(a) >> 16
#define LWL_4(a) (uint32_t)(a), (uint32_t)(a) >> 8, (uint32_t)(a) >> 16, (uint32_t)(a) >> 24

////////////////////////////////////////////////////////////////////////////////
// Log Message IDs for OBC STM32
////////////////////////////////////////////////////////////////////////////////

enum {
    INVALID = 0,                        // ID 0: Reserved
    // EXP Block
    TIMESTAMP = 1,                      // ID 1
    TEMPERATURE_NTC,                    // ID 2
    TEMPERATURE_SINGLE_NTC,             // ID 3
    TEMPERATURE_ERROR,                  // ID 4
    TEMPERATURE_AUTOMMODE_ON,           // ID 5
    TEMPERATURE_AUTOMMODE_TEC_ON,       // ID 6
    TEMPERATURE_AUTOMMODE_TEC_OFF,      // ID 7
    TEMPERATURE_TEC_ON,                 // ID 8
    TEMPERATURE_TEC_OFF,                // ID 9
    TEMPERATURE_TEC_STATUS,             // ID 10
    TEMPERATURE_HEATER_ON,              // ID 11
    TEMPERATURE_HEATER_OFF,             // ID 12
    TEMPERATURE_HEATER_STATUS,          // ID 13
    TEMPERATURE_INTERNAL_LASER_ON,      // ID 14
    TEMPERATURE_INTERNAL_LASER_OFF,     // ID 15
    TEMPERATURE_EXTERNAL_LASER_ON,      // ID 16
    TEMPERATURE_EXTERNAL_LASER_OFF,     // ID 17
    PHOTODIODE_START_SAMPLING,          // ID 18
    PHOTODIODE_FINISH_SAMPLING,         // ID 19
    SYSTEM_RESET,                       // ID 20
    SYSTEM_INITIALIZED,                 // ID 21
    SYSTEM_STARTED,                     // ID 22

    // OBC STM32 block
    OBC_STM32_________________LOG,      // ID 23
    OBC_STM32_TEST_LOG,                 // ID 24
    OBC_STM32_STARTUP,                  // ID 25
	OBC_STM32_BOOTING,					// 26
	OBC_STM32_LOGTEST,

    // OBC CM4 block
    OBC_CM4___________________LOG,      // ID 26
    OBC_CM4_TEST_LOG,                   // ID 27
    OBC_CM4_STARTUP                     // ID 28

};

////////////////////////////////////////////////////////////////////////////////
// Public Function Declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize the LWL module
 */
void LWL_Init(void);

/**
 * @brief Main logging function with variable arguments
 * @param id Log message ID
 * @param ... Variable arguments (each converted to bytes)
 */
void LWL_Log(uint8_t id, ...);

/**
 * @brief Enable/disable LWL logging
 * @param enable True to enable, false to disable
 */
void LWL_Enable(bool enable);

/**
 * @brief Test function to generate sample logs
 * @return 0 on success
 */
int32_t LWL_TestLogs(void);

////////////////////////////////////////////////////////////////////////////////
// Convenience Macros
////////////////////////////////////////////////////////////////////////////////

// Macro for easier logging (optional backward compatibility)
#define LWL(id, ...) LWL_Log(id, ##__VA_ARGS__)

#endif // _LWL_H_
