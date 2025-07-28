#include "log_manager.h"
#include <stdio.h>

#include "logger/bscript_logger.h"
#include "DateTime/date_time.h"
#include "SimpleDataTransfer/simple_datatrans.h"

__attribute__((section(".log_data"))) __attribute__((aligned(4))) uint8_t obc_log_left[LOG_BUFFER_SIZE_OBC];
__attribute__((section(".log_data"))) __attribute__((aligned(4))) uint8_t obc_log_right[LOG_BUFFER_SIZE_OBC];

__attribute__((section(".log_data"))) __attribute__((aligned(4))) uint8_t exp_log_left[LOG_BUFFER_SIZE_EXP];
__attribute__((section(".log_data"))) __attribute__((aligned(4))) uint8_t exp_log_right[LOG_BUFFER_SIZE_EXP];

LogManager_TypeDef log_manager;

// --- Internal Helper Functions ---
static void LogManager_InitChannel(LogChannel_TypeDef *channel, uint8_t *b_left, uint8_t *b_right, uint32_t buf_size, uint32_t threshold) {
    channel->buffer_left = b_left;
    channel->buffer_right = b_right;
    channel->buffer_size = buf_size;
    channel->trigger_threshold = threshold;

    memset(channel->buffer_left, 0, channel->buffer_size);
    memset(channel->buffer_right, 0, channel->buffer_size);

    channel->current_index = 0;
    channel->active_buffer = LOG_BUFFER_LEFT;
    channel->transfer_ready_flag = false;
}

static void LogManager_Write_Internal(LogChannel_TypeDef *channel, uint8_t *data, uint32_t length) {
    if (channel->current_index + length > channel->buffer_size) {
        #ifdef LOG_MANAGER_DEBUG
        printf("LogManager_Write: Data too large or buffer full! Dropping data.\n");
        #endif
        return;
    }

    uint8_t *target_buffer = (channel->active_buffer == LOG_BUFFER_LEFT) ? channel->buffer_left : channel->buffer_right;

    memcpy(target_buffer + channel->current_index, data, length);
    channel->current_index += length;

    if (channel->current_index >= channel->trigger_threshold && !channel->transfer_ready_flag) {
        channel->transfer_ready_flag = true;
        channel->active_buffer = (channel->active_buffer == LOG_BUFFER_LEFT) ? LOG_BUFFER_RIGHT : LOG_BUFFER_LEFT;
        channel->current_index = 0;
    }
}


// --- Public API Implementation ---

void LogManager_Init(void) {
    // Initialize OBC Channel
    LogManager_InitChannel(&log_manager.channels[LOG_SOURCE_OBC],
                           obc_log_left, obc_log_right,
                           LOG_BUFFER_SIZE_OBC, LOG_TRIGGER_THRESHOLD_OBC);

    // Initialize EXP Channel
    LogManager_InitChannel(&log_manager.channels[LOG_SOURCE_EXP],
                           exp_log_left, exp_log_right,
                           LOG_BUFFER_SIZE_EXP, LOG_TRIGGER_THRESHOLD_EXP);
}

void LogManager_Write_OBC(uint8_t *data, uint32_t length) {
    LogManager_Write_Internal(&log_manager.channels[LOG_SOURCE_OBC], data, length);
}

void LogManager_Write_EXP(uint8_t *data, uint32_t length) {
    LogManager_Write_Internal(&log_manager.channels[LOG_SOURCE_EXP], data, length);
}

void LogManager_Process(void) {
    for (int i = 0; i < LOG_SOURCE_COUNT; i++) {
        LogChannel_TypeDef *channel = &log_manager.channels[i];

        if (channel->transfer_ready_flag) {
            // The buffer to send is the one that is NOT active
            uint8_t *buffer_to_send = (channel->active_buffer == LOG_BUFFER_RIGHT) ? channel->buffer_left : channel->buffer_right;
            uint32_t length_to_send = channel->trigger_threshold; // Send the threshold amount

//            LogManager_SendLogData((LogSource_TypeDef)i, buffer_to_send, length_to_send);
//
//            // Clear the buffer that was just sent and reset the flag
            if (LogManager_SendLogData((LogSource_TypeDef)i, buffer_to_send, length_to_send)) {
                memset(buffer_to_send, 0, channel->buffer_size);
                channel->transfer_ready_flag = false;
            }
        }
    }
}


bool LogManager_SendLogData(LogSource_TypeDef source, uint8_t *current_buffer_to_send, uint32_t data_length) {

    char base_filename[48];
    s_DateTime rtc;
    Utils_GetRTC(&rtc);

    snprintf(base_filename, sizeof(base_filename), "obc_log_20%02d%02d%02d_%02d%02d%02d",
             rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);


    BScript_Log("[LogManager] Initiating transfer for %s data...", base_filename);

    SimpleTransferResult_t result = SimpleDataTransfer_ExecuteLogTransfer(
        base_filename,
        current_buffer_to_send,
        data_length,
		rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second
    );

    if (result == SIMPLE_TRANSFER_SUCCESS) {
        BScript_Log("[LogManager] Transfer for %s succeeded.", base_filename);
        return true;
    } else {
        BScript_Log("[LogManager] Transfer for %s failed. Result: %s",
                      base_filename, SimpleDataTransfer_GetResultString(result));
        return false;
    }
}

// --- Debugging API Implementation ---

#define LOG_MANAGER_DEBUG

void LogManager_DebugInfo(LogSource_TypeDef source) {
    #ifdef LOG_MANAGER_DEBUG
    if (source >= LOG_SOURCE_COUNT) return;
    LogChannel_TypeDef *channel = &log_manager.channels[source];
    const char* source_name = (source == LOG_SOURCE_OBC) ? "OBC" : "EXP";

    BScript_Log("--- Log Manager Debug Info (%s) ---", source_name);
    BScript_Log("Current Index: %lu bytes", channel->current_index);
    BScript_Log("Active Buffer: %s", (channel->active_buffer == LOG_BUFFER_LEFT) ? "LEFT" : "RIGHT");
    BScript_Log("Transfer Ready Flag: %s", channel->transfer_ready_flag ? "TRUE" : "FALSE");
    BScript_Log("--------------------------------------");
    #endif
}


void LogManager_DumpBuffer(LogSource_TypeDef source, LogBufferSide_TypeDef buffer_side) {
    #ifdef LOG_MANAGER_DEBUG
    if (source >= LOG_SOURCE_COUNT) return;
    LogChannel_TypeDef *channel = &log_manager.channels[source];
    const char* source_name = (source == LOG_SOURCE_OBC) ? "OBC" : "EXP";
    const char* buffer_name = (buffer_side == LOG_BUFFER_LEFT) ? "LEFT" : "RIGHT";
    uint8_t* buffer_ptr = (buffer_side == LOG_BUFFER_LEFT) ? channel->buffer_left : channel->buffer_right;
    uint32_t size = channel->buffer_size;

    BScript_Log("--- Dumping %s Buffer for %s (Size: %lu bytes) ---", buffer_name, source_name, size);

    char line_buf[128];
    for (uint32_t i = 0; i < size; i += 16) {
        int len = snprintf(line_buf, sizeof(line_buf), "%04lX: ", i);
        for (uint32_t j = 0; j < 16 && (i + j) < size; j++) {
            len += snprintf(line_buf + len, sizeof(line_buf) - len, "%02X ", buffer_ptr[i + j]);
        }
        BScript_Log("%s", line_buf);
    }

    BScript_Log("------------------------------------------------");
    #endif
}

