#ifndef INC_LOG_MANAGER_H_
#define INC_LOG_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Configuration
#define LOG_BUFFER_SIZE_OBC         (32 * 1024)
#define LOG_TRIGGER_THRESHOLD_OBC   (31 * 1024)
#define LOG_BUFFER_SIZE_EXP         (2 * 1024) // Can be different if needed
#define LOG_TRIGGER_THRESHOLD_EXP   (1 * 1024) // Can be different if needed

// Enum to identify the log source
typedef enum {
    LOG_SOURCE_OBC,
    LOG_SOURCE_EXP,
    LOG_SOURCE_COUNT // Helper to count the number of sources
} LogSource_TypeDef;

// Enum for buffer side
typedef enum {
    LOG_BUFFER_LEFT,
    LOG_BUFFER_RIGHT
} LogBufferSide_TypeDef;

// Structure to manage a single log channel (e.g., OBC or EXP)
typedef struct {
    uint8_t *buffer_left;
    uint8_t *buffer_right;
    uint32_t buffer_size;
    uint32_t trigger_threshold;
    uint32_t current_index;
    LogBufferSide_TypeDef active_buffer;
    volatile bool transfer_ready_flag;
} LogChannel_TypeDef;

// Main Log Manager structure holding all channels
typedef struct {
    LogChannel_TypeDef channels[LOG_SOURCE_COUNT];
} LogManager_TypeDef;


// --- Public API ---

void LogManager_Init(void);
void LogManager_Write_OBC(uint8_t *data, uint32_t length);
void LogManager_Write_EXP(uint8_t *data, uint32_t length);
void LogManager_Process(void);

/**
 * @brief Weak function to be implemented by the user for sending data.
 * @param source The source of the log (OBC or EXP).
 * @param buffer_to_send Pointer to the data buffer to send.
 * @param data_length The length of the data to send.
 */
_Bool LogManager_SendLogData(LogSource_TypeDef source, uint8_t *buffer_to_send, uint32_t data_length);


// --- Debugging API (Optional) ---

void LogManager_DebugInfo(LogSource_TypeDef source);
void LogManager_DumpBuffer(LogSource_TypeDef source, LogBufferSide_TypeDef buffer_side);


#endif /* INC_LOG_MANAGER_H_ */
