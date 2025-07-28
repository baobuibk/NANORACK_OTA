/*
 * min_process.c
 *
 *  Created on: Apr 18, 2025
 *      Author: CAO HIEU
 */
#include "main.h"
#include "board.h"
#include "uart_driver_dma.h"
#include "SysLog/syslog.h"
#include "min_proto.h"
#include "min_app/min_command.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "min_process.h"
#include "stdio.h"

#include "DateTime/date_time.h"

MIN_Context_t OBC_MinCtx;

static SemaphoreHandle_t responseSemaphore;
static QueueHandle_t pendingCommandsQueue;

typedef struct {
    uint8_t cmdId;
    uint8_t expectedResponseId;
} CommandInfo_t;

void MIN_ResponseCallback(uint8_t min_id, const uint8_t *payload, uint8_t len) {
    CommandInfo_t cmdInfo;
    if (xQueuePeek(pendingCommandsQueue, &cmdInfo, 0) == pdTRUE) {
        if (min_id == cmdInfo.expectedResponseId) {
            xQueueReceive(pendingCommandsQueue, &cmdInfo, 0);
            xSemaphoreGive(responseSemaphore);
        }
    }
}

static void ClearPendingCommand(void) {
    CommandInfo_t cmdInfo;
    if (xQueueReceive(pendingCommandsQueue, &cmdInfo, 0) == pdTRUE) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Cleared pending command: ID 0x%02X, Expected 0x%02X\r\n",
                 cmdInfo.cmdId, cmdInfo.expectedResponseId);
        UART_Driver_SendString(UART_DEBUG, buffer);
    }
}

void MIN_Timeout_Handler(MIN_Context_t *ctx) {
    SYSLOG_ERROR_POLL("MIN-Timeout!");
}

void MIN_Process_Init(void){
	MIN_Context_Init(&OBC_MinCtx, EXP_PORT);
	MIN_RegisterTimeoutCallback(&OBC_MinCtx, MIN_Timeout_Handler);

	responseSemaphore = xSemaphoreCreateBinary();
	pendingCommandsQueue = xQueueCreate(10, sizeof(CommandInfo_t));

	MIN_Handler_Init();
	MIN_RegisterResponseHandler(MIN_ResponseCallback);

	Sys_Boardcast(E_OK, LOG_INFOR, "MIN Process Init!");
}

void MIN_Processing(void){
    while (UART_DMA_Driver_IsDataAvailable(UART_EXP)) {
        int data = UART_DMA_Driver_Read(UART_EXP);
        if (data >= 0) {
            uint8_t byte = (uint8_t)data;
            MIN_App_Poll(&OBC_MinCtx, &byte, 1);
        }
    }
	MIN_App_Poll(&OBC_MinCtx, NULL, 0);
}

// =================================================================
// Command Sending Functions
// =================================================================
uint8_t MIN_Send_PLEASE_RESET_CMD(void) {
    uint8_t payload[1] = {0xFF};

    MIN_Send(&OBC_MinCtx, PLEASE_RESET_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {PLEASE_RESET_CMD, PLEASE_RESET_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - PLEASE_RESET_CMD");
        return 1;
    } else {
    	return 0;
        SYSLOG_ERROR("Timeout PLEASE_RESET_CMD");
        ClearPendingCommand();
    }

    return 1;
}

void MIN_Send_TEST_CONNECTION_CMD(uint32_t value) {
    uint8_t payload[4] = {0};
    // Big-endian packing [3210]
    payload[3] = (uint8_t)((value >> 24) & 0xFF);
    payload[2] = (uint8_t)((value >> 16) & 0xFF);
    payload[1] = (uint8_t)((value >> 8) & 0xFF);
    payload[0] = (uint8_t)((value & 0xFF));

    MIN_Send(&OBC_MinCtx, TEST_CONNECTION_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {TEST_CONNECTION_CMD, TEST_CONNECTION_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - TEST_CONNECTION_CMD");
    } else {
        SYSLOG_ERROR("Timeout TEST_CONNECTION_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_WORKING_RTC_CMD(void) {
    s_DateTime now;
    Utils_GetRTC(&now);

    uint8_t payload[5] = {0};
    payload[0] = now.day;
    payload[1] = now.hour;
    payload[2] = now.minute;
    payload[3] = now.second;
    payload[4] = 0xFF;

    MIN_Send(&OBC_MinCtx, SET_WORKING_RTC_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_WORKING_RTC_CMD, SET_WORKING_RTC_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_WORKING_RTC_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_WORKING_RTC_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_NTC_CONTROL_CMD(uint8_t ntc_bitControl) {
    uint8_t payload[1] = {ntc_bitControl}; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_NTC_CONTROL_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_NTC_CONTROL_CMD, SET_NTC_CONTROL_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_NTC_CONTROL_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_NTC_CONTROL_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_TEMP_PROFILE_CMD(uint16_t target_temp, uint16_t min_temp, uint16_t max_temp,
        uint8_t ntc_pri, uint8_t ntc_sec, uint8_t auto_recover,
        uint8_t tec_positions, uint8_t heater_positions,
        uint16_t tec_vol, uint8_t heater_duty_cycle)
	{
	uint8_t payload[15] = {0};

	payload[0] = (uint8_t)((target_temp >> 8) & 0xFF);
	payload[1] = (uint8_t)(target_temp & 0xFF);

	payload[2] = (uint8_t)((min_temp >> 8) & 0xFF);
	payload[3] = (uint8_t)(min_temp & 0xFF);

	payload[4] = (uint8_t)((max_temp >> 8) & 0xFF);
	payload[5] = (uint8_t)(max_temp & 0xFF);

	payload[6] = ntc_pri;
	payload[7] = ntc_sec;

	payload[8] = auto_recover;

	payload[9] = tec_positions;
	payload[10] = heater_positions;

	payload[11] = (uint8_t)((tec_vol >> 8) & 0xFF);
	payload[12] = (uint8_t)(tec_vol & 0xFF);

	payload[13] = heater_duty_cycle;

	payload[14] = 0xFF; // RESERVED


    MIN_Send(&OBC_MinCtx, SET_TEMP_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_TEMP_PROFILE_CMD, SET_TEMP_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_TEMP_PROFILE_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_TEMP_PROFILE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_START_TEMP_PROFILE_CMD(void) {
    uint8_t payload[1] = {0xFF}; // RESERVED
    MIN_Send(&OBC_MinCtx, START_TEMP_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {START_TEMP_PROFILE_CMD, START_TEMP_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - START_TEMP_PROFILE_CMD");
    } else {
        SYSLOG_ERROR("Timeout START_TEMP_PROFILE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_STOP_TEMP_PROFILE_CMD(void) {
    uint8_t payload[1] = {0xFF}; // RESERVED
    MIN_Send(&OBC_MinCtx, STOP_TEMP_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {STOP_TEMP_PROFILE_CMD, STOP_TEMP_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - STOP_TEMP_PROFILE_CMD");
    } else {
        SYSLOG_ERROR("Timeout STOP_TEMP_PROFILE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_OVERRIDE_TEC_PROFILE_CMD(uint16_t interval, uint8_t ovr_tec_index, uint16_t ovr_tec_vol) {
    uint8_t payload[6] = {0};

    payload[0] = (uint8_t)((interval >> 8) & 0xFF);  // MSB
    payload[1] = (uint8_t)(interval & 0xFF);         // LSB
    payload[2] = ovr_tec_index;
    payload[3] = (uint8_t)((ovr_tec_vol >> 8) & 0xFF);  // MSB
    payload[4] = (uint8_t)(ovr_tec_vol & 0xFF);         // LSB
    payload[5] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_OVERRIDE_TEC_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_OVERRIDE_TEC_PROFILE_CMD, SET_OVERRIDE_TEC_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_OVERRIDE_TEC_PROFILE_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_OVERRIDE_TEC_PROFILE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_START_OVERRIDE_TEC_PROFILE_CMD(void) {
    uint8_t payload[1] = {0xFF}; // RESERVED
    MIN_Send(&OBC_MinCtx, START_OVERRIDE_TEC_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {START_OVERRIDE_TEC_PROFILE_CMD, START_OVERRIDE_TEC_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - START_OVERRIDE_TEC_PROFILE_CMD");
    } else {
        SYSLOG_ERROR("Timeout START_OVERRIDE_TEC_PROFILE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_STOP_OVERRIDE_TEC_PROFILE_CMD(void) {
    uint8_t payload[1] = {0xFF}; // RESERVED
    MIN_Send(&OBC_MinCtx, STOP_OVERRIDE_TEC_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {STOP_OVERRIDE_TEC_PROFILE_CMD, STOP_OVERRIDE_TEC_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - STOP_OVERRIDE_TEC_PROFILE_CMD");
    } else {
        SYSLOG_ERROR("Timeout STOP_OVERRIDE_TEC_PROFILE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_PDA_PROFILE_CMD(uint32_t sample_rate, uint16_t pre, uint16_t in, uint16_t post) {
    uint8_t payload[11] = {0};

    // Big-endian packing for 32-bit values
    // Big-endian packing for 16-bit value
    payload[0] = (uint8_t)((sample_rate >> 24) & 0xFF);
    payload[1] = (uint8_t)((sample_rate >> 16) & 0xFF);
    payload[2] = (uint8_t)((sample_rate >> 8) & 0xFF);
    payload[3] = (uint8_t)(sample_rate & 0xFF);

    payload[4] = (uint8_t)((pre >> 8) & 0xFF);
    payload[5] = (uint8_t)(pre & 0xFF);


    payload[6] = (uint8_t)((in >> 8) & 0xFF);
    payload[7] = (uint8_t)(in & 0xFF);


    payload[8] = (uint8_t)((post >> 8) & 0xFF);
    payload[9] = (uint8_t)(post & 0xFF);

    payload[10] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_PDA_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_PDA_PROFILE_CMD, SET_PDA_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_SAMPLING_PROFILE_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_SAMPLING_PROFILE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_LASER_INTENSITY_CMD(uint8_t intensity) {
    uint8_t payload[2] = {0};

    payload[0] = intensity;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_LASER_INTENSITY_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_LASER_INTENSITY_CMD, SET_LASER_INTENSITY_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_LASER_INTENSITY_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_LASER_INTENSITY_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_POSITION_CMD(uint8_t position) {
    uint8_t payload[2] = {0};

    payload[0] = position;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_POSITION_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_POSITION_CMD, SET_POSITION_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_POSITION_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_POSITION_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_START_SAMPLE_CYCLE_CMD(void) {
    uint8_t payload[1] = {0xFF}; // RESERVED
    MIN_Send(&OBC_MinCtx, START_SAMPLE_CYCLE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {START_SAMPLE_CYCLE_CMD, START_SAMPLE_CYCLE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - START_SAMPLING_CYCLE_CMD");
    } else {
        SYSLOG_ERROR("Timeout START_SAMPLING_CYCLE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_GET_INFO_SAMPLE_CMD(void) {
    uint8_t payload[1] = {0xFF}; // RESERVED
    MIN_Send(&OBC_MinCtx, GET_INFO_SAMPLE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_INFO_SAMPLE_CMD, GET_INFO_SAMPLE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_INFO_SAMPLE_CMD");
    } else {
        SYSLOG_ERROR("Timeout GET_INFO_SAMPLE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_GET_CHUNK_CMD(uint8_t noChunk) {
    uint8_t payload[2] = {0};

    payload[0] = noChunk;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, GET_CHUNK_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_CHUNK_CMD, GET_CHUNK_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_CHUNK_CMD");
    } else {
        SYSLOG_ERROR("Timeout GET_CHUNK_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_GET_CHUNK_CRC_CMD(uint8_t noChunk) {
    uint8_t payload[2] = {0};

    payload[0] = noChunk;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, GET_CHUNK_CRC_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_CHUNK_CRC_CMD, GET_CHUNK_CRC_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_CHUNK_CRC_CMD");
    } else {
        SYSLOG_ERROR("Timeout GET_CHUNK_CRC_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_EXT_LASER_INTENSITY_CMD(uint8_t intensity) {
    uint8_t payload[2] = {0};

    payload[0] = intensity;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_EXT_LASER_INTENSITY_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_EXT_LASER_INTENSITY_CMD, SET_EXT_LASER_INTENSITY_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_EXT_LASER_PROFILE_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_EXT_LASER_PROFILE_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_TURN_ON_EXT_LASER_CMD(uint8_t position) {
    uint8_t payload[2] = {0};

    payload[0] = position;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, TURN_ON_EXT_LASER_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {TURN_ON_EXT_LASER_CMD, TURN_ON_EXT_LASER_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - TURN_ON_EXT_LASER_CMD");
    } else {
        SYSLOG_ERROR("Timeout TURN_ON_EXT_LASER_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_TURN_OFF_EXT_LASER_CMD(void) { // all
    uint8_t payload[1] = {0xFF}; // RESERVED
    MIN_Send(&OBC_MinCtx, TURN_OFF_EXT_LASER_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {TURN_OFF_EXT_LASER_CMD, TURN_OFF_EXT_LASER_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - TURN_OFF_EXT_LASER_CMD");
    } else {
        SYSLOG_ERROR("Timeout TURN_OFF_EXT_LASER_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_LASER_INT_CMD(uint8_t pos, uint8_t percent) {
    uint8_t payload[3] = {0};

    payload[0] = pos;
    payload[1] = percent;
    payload[2] = 0xFF;

    MIN_Send(&OBC_MinCtx, SET_LASER_INT_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_LASER_INT_CMD, SET_LASER_INT_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_LASER_INT_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_LASER_INT_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_SET_LASER_EXT_CMD(uint8_t pos, uint8_t percent) {
    uint8_t payload[3] = {0};

    payload[0] = pos;
    payload[1] = percent;
    payload[2] = 0xFF;

    MIN_Send(&OBC_MinCtx, SET_LASER_EXT_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_LASER_EXT_CMD, SET_LASER_EXT_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_LASER_EXT_CMD");
    } else {
        SYSLOG_ERROR("Timeout SET_LASER_EXT_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_GET_CURRENT_CMD(void) {
    uint8_t payload[1] = {0};

    payload[0] = 0xFF;

    MIN_Send(&OBC_MinCtx, GET_LASER_CURRENT_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_LASER_CURRENT_CMD, GET_LASER_CURRENT_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_CURRENT_CMD");
    } else {
        SYSLOG_ERROR("Timeout GET_CURRENT_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_GET_LOG_CMD(void) {
    uint8_t payload[1] = {0};

    payload[0] = 0xFF;

    MIN_Send(&OBC_MinCtx, GET_LOG_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_LOG_CMD, GET_LOG_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_LOG_CMD");
    } else {
        SYSLOG_ERROR("Timeout GET_LOG_CMD");
        ClearPendingCommand();
    }
}

void MIN_Send_CUSTOM_COMMAND_CMD(const char *cmdStr, uint8_t len) {
    MIN_Send(&OBC_MinCtx, CUSTOM_COMMAND_CMD, (const uint8_t *)cmdStr, len);

    CommandInfo_t cmdInfo = {CUSTOM_COMMAND_CMD, CUSTOM_COMMAND_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - CUSTOM_COMMAND_CMD");
    } else {
        SYSLOG_ERROR("Timeout CUSTOM_COMMAND_CMD");
        ClearPendingCommand();
    }
}

//-------------------------------------------------------------------
_Bool MIN_Send_SET_WORKING_RTC_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    s_DateTime now;
    Utils_GetRTC(&now);

    uint8_t payload[5] = {0};
    payload[0] = now.day;
    payload[1] = now.hour;
    payload[2] = now.minute;
    payload[3] = now.second;
    payload[4] = 0xFF;

    MIN_Send(&OBC_MinCtx, SET_WORKING_RTC_CMD, payload, sizeof(payload));
    CommandInfo_t cmdInfo = { SET_WORKING_RTC_CMD, SET_WORKING_RTC_ACK };
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);


    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_WORKING_RTC_CMD");
        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout SET_WORKING_RTC_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_CLEAN_PROFILE_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0xFF}; // RESERVED

    MIN_Send(&OBC_MinCtx, CLEAN_PROFILE_CMD, payload, sizeof(payload));
    CommandInfo_t cmdInfo = { CLEAN_PROFILE_CMD, CLEAN_PROFILE_ACK };
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);


    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - CLEAN_PROFILE_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout CLEAN_PROFILE_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_SET_NTC_CONTROL_CMD_WithData(uint8_t ntc_bitControl, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {ntc_bitControl}; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_NTC_CONTROL_CMD, payload, sizeof(payload));
    CommandInfo_t cmdInfo = { SET_NTC_CONTROL_CMD, SET_NTC_CONTROL_ACK };
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);


    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_NTC_CONTROL_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout SET_NTC_CONTROL_CMD");
        ClearPendingCommand();
        return false;
    }
}


_Bool MIN_Send_SET_TEMP_PROFILE_CMD_WithData(uint16_t target_temp, uint16_t min_temp, uint16_t max_temp,
											uint8_t ntc_pri, uint8_t ntc_sec, uint8_t auto_recover,
											uint8_t tec_positions, uint8_t heater_positions,
											uint16_t tec_vol, uint8_t heater_duty_cycle,
											uint8_t* response_data, uint8_t* response_len ) {
    uint8_t payload[15] = {0};

    payload[0] = (uint8_t)((target_temp >> 8) & 0xFF);
    payload[1] = (uint8_t)(target_temp & 0xFF);

    payload[2] = (uint8_t)((min_temp >> 8) & 0xFF);
    payload[3] = (uint8_t)(min_temp & 0xFF);

    payload[4] = (uint8_t)((max_temp >> 8) & 0xFF);
    payload[5] = (uint8_t)(max_temp & 0xFF);

    payload[6] = ntc_pri;
    payload[7] = ntc_sec;

    payload[8] = auto_recover;

    payload[9] = tec_positions;
    payload[10] = heater_positions;

    payload[11] = (uint8_t)((tec_vol >> 8) & 0xFF);      // MSB of tec_vol
    payload[12] = (uint8_t)(tec_vol & 0xFF);             // LSB of tec_vol
    payload[13] = heater_duty_cycle;

    payload[14] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_TEMP_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_TEMP_PROFILE_CMD, SET_TEMP_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_TEMP_PROFILE_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout SET_TEMP_PROFILE_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_START_TEMP_PROFILE_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0xFF}; // RESERVED

    MIN_Send(&OBC_MinCtx, START_TEMP_PROFILE_CMD, payload, sizeof(payload));
    CommandInfo_t cmdInfo = { START_TEMP_PROFILE_CMD, START_TEMP_PROFILE_ACK };
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);


    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - START_TEMP_PROFILE_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout START_TEMP_PROFILE_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_SET_OVERRIDE_TEC_PROFILE_CMD_WithData(uint16_t interval, uint8_t ovr_tec_index, uint16_t ovr_tec_vol, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[6] = {0};

    payload[0] = (uint8_t)((interval >> 8) & 0xFF);
    payload[1] = (uint8_t)(interval & 0xFF);

    payload[2] = ovr_tec_index;
    payload[3] = (uint8_t)((ovr_tec_vol >> 8) & 0xFF);  // MSB
    payload[4] = (uint8_t)(ovr_tec_vol & 0xFF);         // LSB
    payload[5] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_OVERRIDE_TEC_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_OVERRIDE_TEC_PROFILE_CMD, SET_OVERRIDE_TEC_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_OVERRIDE_TEC_PROFILE_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout SET_OVERRIDE_TEC_PROFILE_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_START_OVERRIDE_TEC_PROFILE_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0xFF}; // RESERVED

    MIN_Send(&OBC_MinCtx, START_OVERRIDE_TEC_PROFILE_CMD, payload, sizeof(payload));
    CommandInfo_t cmdInfo = { START_OVERRIDE_TEC_PROFILE_CMD, START_OVERRIDE_TEC_PROFILE_ACK };
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);


    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - START_OVERRIDE_TEC_PROFILE_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout START_OVERRIDE_TEC_PROFILE_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_SET_PDA_PROFILE_CMD_WithData(uint32_t sample_rate, uint16_t pre, uint16_t in, uint16_t post, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[11] = {0};
    // Big-endian packing for 16-bit value

    payload[0] = (uint8_t)((sample_rate >> 24) & 0xFF);
    payload[1] = (uint8_t)((sample_rate >> 16) & 0xFF);
    payload[2] = (uint8_t)((sample_rate >> 8) & 0xFF);
    payload[3] = (uint8_t)(sample_rate & 0xFF);

    payload[4] = (uint8_t)((pre >> 8) & 0xFF);
    payload[5] = (uint8_t)(pre & 0xFF);

    payload[6] = (uint8_t)((in >> 8) & 0xFF);
    payload[7] = (uint8_t)(in & 0xFF);

    payload[8] = (uint8_t)((post >> 8) & 0xFF);
    payload[9] = (uint8_t)(post & 0xFF);

    payload[10] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_PDA_PROFILE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_PDA_PROFILE_CMD, SET_PDA_PROFILE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_PDA_PROFILE_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout SET_PDA_PROFILE_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_SET_LASER_INTENSITY_CMD_WithData(uint8_t intensity, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[2] = {0};
    // Big-endian packing for 16-bit value

    payload[0] = intensity;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_LASER_INTENSITY_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_LASER_INTENSITY_CMD, SET_LASER_INTENSITY_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_LASER_INTENSITY_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout SET_LASER_INTENSITY_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_SET_POSITION_CMD_WithData(uint8_t position, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[2] = {0};
    // Big-endian packing for 16-bit value

    payload[0] = position;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_POSITION_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_POSITION_CMD, SET_POSITION_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_POSITION_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout SET_POSITION_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_START_SAMPLE_CYCLE_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0};
    // Big-endian packing for 16-bit value
    payload[0] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, START_SAMPLE_CYCLE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {START_SAMPLE_CYCLE_CMD, START_SAMPLE_CYCLE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - START_SAMPLE_CYCLE_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout START_SAMPLE_CYCLE_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_GET_INFO_SAMPLE_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0};
    // Big-endian packing for 16-bit value
    payload[0] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, GET_INFO_SAMPLE_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_INFO_SAMPLE_CMD, GET_INFO_SAMPLE_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_INFO_SAMPLE_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout GET_INFO_SAMPLE_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_GET_CHUNK_CMD_WithData(uint8_t nochunk, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[2] = {0};
    // Big-endian packing for 16-bit value
    payload[0] = nochunk;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, GET_CHUNK_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_CHUNK_CMD, GET_CHUNK_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_CHUNK_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout GET_CHUNK_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_GET_CHUNK_CRC_CMD_WithData(uint8_t nochunk, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[2] = {0};
    // Big-endian packing for 16-bit value
    payload[0] = nochunk;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, GET_CHUNK_CRC_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_CHUNK_CRC_CMD, GET_CHUNK_CRC_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_CHUNK_CRC_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout GET_CHUNK_CRC_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_GET_LASER_CURRENT_DATA_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0};
    // Big-endian packing for 16-bit value
    payload[0] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, GET_LASER_CURRENT_DATA_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_LASER_CURRENT_DATA_CMD, GET_LASER_CURRENT_DATA_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_LASER_CURRENT_DATA_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout GET_LASER_CURRENT_DATA_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_GET_LASER_CURRENT_CRC_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0};
    // Big-endian packing for 16-bit value
    payload[0] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, GET_LASER_CURRENT_CRC_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_LASER_CURRENT_CRC_CMD, GET_LASER_CURRENT_CRC_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_LASER_CURRENT_CRC_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout GET_LASER_CURRENT_CRC_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_SET_EXT_LASER_INTENSITY_CMD_WithData(uint8_t intensity, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[2] = {0};
    // Big-endian packing for 16-bit value

    payload[0] = intensity;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, SET_EXT_LASER_INTENSITY_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {SET_EXT_LASER_INTENSITY_CMD, SET_EXT_LASER_INTENSITY_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - SET_EXT_LASER_INTENSITY_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout SET_EXT_LASER_INTENSITY_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_TURN_ON_EXT_LASER_CMD_WithData(uint8_t position, uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[2] = {0};
    // Big-endian packing for 16-bit value

    payload[0] = position;
    payload[1] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, TURN_ON_EXT_LASER_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {TURN_ON_EXT_LASER_CMD, TURN_ON_EXT_LASER_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - TURN_ON_EXT_LASER_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout TURN_ON_EXT_LASER_CMD");
        ClearPendingCommand();
        return false;
    }
}


_Bool MIN_Send_TURN_OFF_EXT_LASER_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0};
    // Big-endian packing for 16-bit value
    payload[0] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, TURN_OFF_EXT_LASER_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {TURN_OFF_EXT_LASER_CMD, TURN_OFF_EXT_LASER_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - TURN_OFF_EXT_LASER_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout TURN_OFF_EXT_LASER_CMD");
        ClearPendingCommand();
        return false;
    }
}

_Bool MIN_Send_GET_LOG_CMD_WithData(uint8_t* response_data, uint8_t* response_len) {
    uint8_t payload[1] = {0};
    // Big-endian packing for 16-bit value
    payload[0] = 0xFF; // RESERVED

    MIN_Send(&OBC_MinCtx, GET_LOG_CMD, payload, sizeof(payload));

    CommandInfo_t cmdInfo = {GET_LOG_CMD, GET_LOG_ACK};
    xQueueSend(pendingCommandsQueue, &cmdInfo, portMAX_DELAY);

    if (xSemaphoreTake(responseSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SYSLOG_NOTICE("Response OK - GET_LOG_CMD");

        if (response_data && response_len) {
            return MIN_GetLastResponseData(response_data, response_len, 100);
        }
        return true;
    } else {
        SYSLOG_ERROR("Timeout GET_LOG_CMD");
        ClearPendingCommand();
        return false;
    }
}
