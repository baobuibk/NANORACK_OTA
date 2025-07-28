/*
 * min_command.c
 *
 *  Created on: Apr 22, 2025
 *      Author: CAO HIEU
 */

#include "min_command.h"
#include <string.h>
#include "stdio.h"
#include "uart_driver_dma.h"
#include "board.h"

#include "logger/bscript_logger.h"

static MIN_ResponseData_t g_last_response_data;
static SemaphoreHandle_t g_response_data_semaphore;

// =================================================================
// Command Handlers
// =================================================================
void MIN_Handler_Init(void){
    g_response_data_semaphore = xSemaphoreCreateBinary();
    memset(&g_last_response_data, 0, sizeof(MIN_ResponseData_t));
}

_Bool MIN_GetLastResponseData(uint8_t* data, uint8_t* length, uint32_t timeout_ms) {
    if (xSemaphoreTake(g_response_data_semaphore, pdMS_TO_TICKS(timeout_ms)) == pdTRUE) {
        if (g_last_response_data.valid) {
            if (data && length) {
                memcpy(data, g_last_response_data.data, g_last_response_data.length);
                *length = g_last_response_data.length;
            }
            g_last_response_data.valid = 0; // Reset
            return true;
        }
    }
    return false;
}

static void MIN_Handler_PLEASE_RESET_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload PLEASE_RESET_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log(buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log(buffer);

    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_TEST_CONNECTION_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload TEST_CONNECTION_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log(buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log(buffer);

    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_SET_WORKING_RTC_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_WORKING_RTC_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log(buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log(buffer);

    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_SET_NTC_CONTROL_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_NTC_CONTROL_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log(buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log(buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }

}

static void MIN_Handler_SET_TEMP_PROFILE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_TEMP_PROFILE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log(buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log(buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }

}

static void MIN_Handler_START_TEMP_PROFILE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload START_TEMP_PROFILE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_STOP_TEMP_PROFILE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload STOP_TEMP_PROFILE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_SET_OVERRIDE_TEC_PROFILE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_OVERRIDE_TEC_PROFILE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_START_OVERRIDE_TEC_PROFILE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload START_OVERRIDE_TEC_PROFILE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_STOP_OVERRIDE_TEC_PROFILE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload STOP_OVERRIDE_TEC_PROFILE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_SET_PDA_PROFILE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_SAMPLING_PROFILE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_SET_LASER_INTENSITY_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_LASER_INTENSITY_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_SET_POSITION_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_POSITION_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_START_SAMPLE_CYCLE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload START_SAMPLING_CYCLE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_GET_INFO_SAMPLE_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload GET_INFO_SAMPLE_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_GET_CHUNK_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload GET_CHUNK_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_GET_CHUNK_CRC_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload GET_CHUNK_CRC_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_GET_LASER_CURRENT_CRC_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload GET_LASER_CURRENT_CRC_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}
static void MIN_Handler_SET_EXT_LASER_INTENSITY_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_EXT_LASER_INTENSITY_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_TURN_ON_EXT_LASER_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload TURN_ON_EXT_LASER_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_TURN_OFF_EXT_LASER_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload TURN_OFF_EXT_LASER_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_SET_LASER_INT_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_LASER_INT_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_GET_LASER_CURRENT_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload GET_LASER_CURRENT_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_SET_LASER_EXT_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload SET_LASER_EXT_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_GET_LOG_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload GET_LOG_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_CUSTOM_COMMAND_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload CUSTOM_COMMAND_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
    if (len <= sizeof(g_last_response_data.data)) {
        memcpy(g_last_response_data.data, payload, len);
        g_last_response_data.length = len;
        g_last_response_data.valid = 1;
        xSemaphoreGive(g_response_data_semaphore);
    }
}

static void MIN_Handler_PING_CMD(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload PING_CMD (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
}

static void MIN_Handler_PONG_CMD(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload PONG_CMD (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
}

static void MIN_Handler_MIN_RESP_NAK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload MIN_RESP_NAK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
}

static void MIN_Handler_MIN_RESP_ACK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload MIN_RESP_ACK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
}

static void MIN_Handler_MIN_RESP_WRONG(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload MIN_RESP_WRONG (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
}

static void MIN_Handler_MIN_RESP_DONE(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload MIN_RESP_DONE (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
}

static void MIN_Handler_MIN_RESP_FAIL(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload MIN_RESP_FAIL (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
}

static void MIN_Handler_MIN_RESP_OK(MIN_Context_t *ctx, const uint8_t *payload, uint8_t len) {
    char buffer[256];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Payload MIN_RESP_OK (%u bytes):", len);
    for (uint8_t i = 0; i < len && offset < sizeof(buffer) - 4; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %02X", payload[i]);
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    BScript_Log("%s", buffer);
    snprintf(buffer, sizeof(buffer), "Message: \"%s\"\r\n", payload);
    BScript_Log("%s", buffer);
}

// =================================================================
// Command Table
// =================================================================
#define GET_LOG_ACK									0x35
#define GET_LASER_CURRENT_ACK						0x33
#define SET_LASER_EXT_ACK							0x31
#define SET_LASER_INT_ACK							0x2F

static const MIN_Command_t command_table[] = {
	{ PLEASE_RESET_ACK,                    MIN_Handler_PLEASE_RESET_ACK 				},
    { TEST_CONNECTION_ACK,                 MIN_Handler_TEST_CONNECTION_ACK 				},
    { SET_WORKING_RTC_ACK,                 MIN_Handler_SET_WORKING_RTC_ACK				},
	{ SET_NTC_CONTROL_ACK,				   MIN_Handler_SET_NTC_CONTROL_ACK				},
    { SET_TEMP_PROFILE_ACK,                MIN_Handler_SET_TEMP_PROFILE_ACK 			},
    { START_TEMP_PROFILE_ACK,              MIN_Handler_START_TEMP_PROFILE_ACK 			},
    { STOP_TEMP_PROFILE_ACK,               MIN_Handler_STOP_TEMP_PROFILE_ACK 			},
    { SET_OVERRIDE_TEC_PROFILE_ACK,        MIN_Handler_SET_OVERRIDE_TEC_PROFILE_ACK 	},
    { START_OVERRIDE_TEC_PROFILE_ACK,      MIN_Handler_START_OVERRIDE_TEC_PROFILE_ACK 	},
    { STOP_OVERRIDE_TEC_PROFILE_ACK,       MIN_Handler_STOP_OVERRIDE_TEC_PROFILE_ACK 	},
    { SET_PDA_PROFILE_ACK,            	   MIN_Handler_SET_PDA_PROFILE_ACK 				},
    { SET_LASER_INTENSITY_ACK,             MIN_Handler_SET_LASER_INTENSITY_ACK 			},
    { SET_POSITION_ACK,                    MIN_Handler_SET_POSITION_ACK 				},
    { START_SAMPLE_CYCLE_ACK,              MIN_Handler_START_SAMPLE_CYCLE_ACK 			},
    { GET_INFO_SAMPLE_ACK,                 MIN_Handler_GET_INFO_SAMPLE_ACK 				},
    { GET_CHUNK_ACK,                       MIN_Handler_GET_CHUNK_ACK 					},
    { GET_CHUNK_CRC_ACK,                   MIN_Handler_GET_CHUNK_CRC_ACK 				},
    { GET_LASER_CURRENT_CRC_ACK,           MIN_Handler_GET_LASER_CURRENT_CRC_ACK 		},
    { SET_EXT_LASER_INTENSITY_ACK,         MIN_Handler_SET_EXT_LASER_INTENSITY_ACK 		},
    { TURN_ON_EXT_LASER_ACK,               MIN_Handler_TURN_ON_EXT_LASER_ACK 			},
    { TURN_OFF_EXT_LASER_ACK,              MIN_Handler_TURN_OFF_EXT_LASER_ACK 			},
    { SET_LASER_INT_ACK,              	   MIN_Handler_SET_LASER_INT_ACK			    },
    { GET_LASER_CURRENT_ACK,               MIN_Handler_GET_LASER_CURRENT_ACK			},
    { SET_LASER_EXT_ACK,              	   MIN_Handler_SET_LASER_EXT_ACK 			    },
    { GET_LOG_ACK,              		   MIN_Handler_GET_LOG_ACK						},
    { CUSTOM_COMMAND_ACK,                  MIN_Handler_CUSTOM_COMMAND_ACK 				},
    { PING_CMD,                            MIN_Handler_PING_CMD 						},
    { PONG_CMD,                            MIN_Handler_PONG_CMD 						},
    { MIN_RESP_NAK,                        MIN_Handler_MIN_RESP_NAK 					},
    { MIN_RESP_ACK,                        MIN_Handler_MIN_RESP_ACK 					},
    { MIN_RESP_WRONG,                      MIN_Handler_MIN_RESP_WRONG 					},
    { MIN_RESP_DONE,                       MIN_Handler_MIN_RESP_DONE 					},
    { MIN_RESP_FAIL,                       MIN_Handler_MIN_RESP_FAIL 					},
    { MIN_RESP_OK,                         MIN_Handler_MIN_RESP_OK 						},
};

static const int command_table_size = sizeof(command_table) / sizeof(command_table[0]);

// =================================================================
// Helper Functions
// =================================================================

const MIN_Command_t *MIN_GetCommandTable(void) {
    return command_table;
}

int MIN_GetCommandTableSize(void) {
    return command_table_size;
}
