/*
 * rtc_rv3129.h
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#ifndef M3_DEVICES_I2C_RTC_RV3129_RTC_RV3129_H_
#define M3_DEVICES_I2C_RTC_RV3129_RTC_RV3129_H_

#include "DateTime/date_time.h"
#include "i2c_driver.h"

typedef struct {
    LL_I2C_HandleTypeDef *rtc_i2c;
    uint8_t address;
} RV3129_HandleTypeDef;

RV3129_HandleTypeDef* RV3129_GetHandle(void);

/**
 * Initializes the RTC by setting the I2C instance (e.g., I2C2) and assigning the RTC address.
 *
 * @param hrtc      Pointer to the RV3129_HandleTypeDef structure
 * @param rtc_i2c   Pointer to the I2C_Handle (already initialized from the I2C driver)
 */
RV3129_HandleTypeDef* RV3129_Driver_Init(I2C_TypeDef *i2c_instance);

/**
 * Reads the time from the RTC.
 *
 * @param hrtc       Pointer to RV3129_HandleTypeDef
 * @param datetime   Pointer to the s_DateTime structure to store the read time
 * @return Std_ReturnType: E_OK if successful, or an error code
 */
Std_ReturnType RV3129_GetTime(RV3129_HandleTypeDef *hrtc, s_DateTime *datetime);

/**
 * Writes the time to the RTC.
 *
 * @param hrtc       Pointer to RV3129_HandleTypeDef
 * @param datetime   Pointer to the s_DateTime structure containing the time to write
 * @return Std_ReturnType: E_OK if successful, or an error code
 */
Std_ReturnType RV3129_SetTime(RV3129_HandleTypeDef *hrtc, s_DateTime *datetime);

/**
 * Reads the temperature value from the RTC.
 *
 * @param hrtc    Pointer to RV3129_HandleTypeDef
 * @param pTemp   Pointer to a variable to store the temperature value (unit: °C, signed integer)
 * @return Std_ReturnType: E_OK if successful, or an error code
 */
Std_ReturnType RV3129_GetTemp(RV3129_HandleTypeDef *hrtc, int16_t *pTemp);

#endif /* M3_DEVICES_I2C_RTC_RV3129_RTC_RV3129_H_ */
