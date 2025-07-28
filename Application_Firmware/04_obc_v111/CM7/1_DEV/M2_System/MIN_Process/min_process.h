/*
 * min_process.h
 *
 *  Created on: Apr 18, 2025
 *      Author: CAO HIEU
 */

#ifndef M2_SYSTEM_MIN_PROCESS_MIN_PROCESS_H_
#define M2_SYSTEM_MIN_PROCESS_MIN_PROCESS_H_

void MIN_Process_Init(void);
void MIN_Processing(void);

void MIN_ResponseCallback(uint8_t min_id, const uint8_t *payload, uint8_t len);
// =================================================================
// Command Sending Functions
// =================================================================
uint8_t MIN_Send_PLEASE_RESET_CMD(void);
void MIN_Send_TEST_CONNECTION_CMD(uint32_t value);

void MIN_Send_SET_TEMP_PROFILE_CMD(uint16_t target_temp, uint16_t min_temp, uint16_t max_temp,
        uint8_t ntc_pri, uint8_t ntc_sec, uint8_t auto_recover,
        uint8_t tec_positions, uint8_t heater_positions,
        uint16_t tec_vol, uint8_t heater_duty_cycle);

void MIN_Send_START_TEMP_PROFILE_CMD(void);
void MIN_Send_STOP_TEMP_PROFILE_CMD(void);

void MIN_Send_SET_OVERRIDE_TEC_PROFILE_CMD(uint16_t interval, uint8_t ovr_tec_index, uint16_t ovr_tec_vol);
void MIN_Send_START_OVERRIDE_TEC_PROFILE_CMD(void);
void MIN_Send_STOP_OVERRIDE_TEC_PROFILE_CMD(void);

void MIN_Send_SET_PDA_PROFILE_CMD(uint32_t sample_rate, uint16_t pre,
                                       uint16_t in,
                                       uint16_t post
                                       );

void MIN_Send_SET_LASER_INTENSITY_CMD(uint8_t intensity);
void MIN_Send_SET_POSITION_CMD(uint8_t position);

void MIN_Send_START_SAMPLE_CYCLE_CMD(void);
void MIN_Send_GET_INFO_SAMPLE_CMD(void);

void MIN_Send_GET_CHUNK_CMD(uint8_t noChunk);

void MIN_Send_SET_EXT_LASER_INTENSITY_CMD(uint8_t intensity);
void MIN_Send_TURN_ON_EXT_LASER_CMD(uint8_t position);
void MIN_Send_TURN_OFF_EXT_LASER_CMD(void);

void MIN_Send_CUSTOM_COMMAND_CMD(const char *cmdStr, uint8_t len);
void MIN_Send_SET_NTC_CONTROL_CMD(uint8_t ntc_bitControl) ;
void MIN_Send_SET_WORKING_RTC_CMD(void);
void MIN_Send_GET_CURRENT_CMD(void);
void MIN_Send_SET_LASER_EXT_CMD(uint8_t pos, uint8_t percent);
void MIN_Send_SET_LASER_INT_CMD(uint8_t pos, uint8_t percent);
void MIN_Send_GET_LOG_CMD(void);
void MIN_Send_GET_CHUNK_CRC_CMD(uint8_t noChunk);
//------------------------------------------------------------------
_Bool MIN_Send_GET_LASER_CURRENT_CRC_CMD_WithData(uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_GET_LASER_CURRENT_DATA_CMD_WithData(uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_GET_CHUNK_CRC_CMD_WithData(uint8_t nochunk, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_GET_CHUNK_CMD_WithData(uint8_t nochunk, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_GET_INFO_SAMPLE_CMD_WithData(uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_START_SAMPLE_CYCLE_CMD_WithData(uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_SET_POSITION_CMD_WithData(uint8_t position, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_SET_LASER_INTENSITY_CMD_WithData(uint8_t intensity, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_SET_PDA_PROFILE_CMD_WithData(uint32_t sample_rate, uint16_t pre, uint16_t in, uint16_t post, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_START_OVERRIDE_TEC_PROFILE_CMD_WithData(uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_SET_OVERRIDE_TEC_PROFILE_CMD_WithData(uint16_t interval, uint8_t ovr_tec_index, uint16_t ovr_tec_vol, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_START_TEMP_PROFILE_CMD_WithData(uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_SET_TEMP_PROFILE_CMD_WithData(uint16_t target_temp, uint16_t min_temp, uint16_t max_temp,
											uint8_t ntc_pri, uint8_t ntc_sec, uint8_t auto_recover,
											uint8_t tec_positions, uint8_t heater_positions,
											uint16_t tec_vol, uint8_t heater_duty_cycle,
											uint8_t* response_data, uint8_t* response_len );
_Bool MIN_Send_SET_NTC_CONTROL_CMD_WithData(uint8_t ntc_bitControl, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_CLEAN_PROFILE_CMD_WithData(uint8_t* response_data, uint8_t* response_len) ;

_Bool MIN_Send_SET_EXT_LASER_INTENSITY_CMD_WithData(uint8_t intensity, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_TURN_OFF_EXT_LASER_CMD_WithData(uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_TURN_ON_EXT_LASER_CMD_WithData(uint8_t position, uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_SET_WORKING_RTC_CMD_WithData(uint8_t* response_data, uint8_t* response_len);
_Bool MIN_Send_GET_LOG_CMD_WithData(uint8_t* response_data, uint8_t* response_len);

#endif /* M2_SYSTEM_MIN_PROCESS_MIN_PROCESS_H_ */
