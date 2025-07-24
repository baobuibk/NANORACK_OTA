/*
 * rtc_rv3129.c
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#include "rtc_rv3129.h"
/************************************************
 *                  Register                    *
 ************************************************/
#define RV3129_ADDR         0x56
#define RV3129_SECONDS      0x08
#define RV3129_MINUTES      0x09
#define RV3129_HOURS        0x0A
#define RV3129_DATE         0x0B
#define RV3129_WEEKDAYS     0x0C
#define RV3129_MONTHS       0x0D
#define RV3129_YEARS        0x0E
#define RV3129_TEMP			0x20
#define OFFSET_TEMP			60

static RV3129_HandleTypeDef hrtc_local;

/************************************************
 *                   Helper                     *
 ************************************************/

static inline uint8_t bcd2dec(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static inline uint8_t dec2bcd(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}


RV3129_HandleTypeDef* RV3129_Driver_Init(I2C_TypeDef *i2c_instance)
{
    static LL_I2C_HandleTypeDef ll_i2c_handle;
    ll_i2c_handle.Instance = i2c_instance;
    ll_i2c_handle.State = I2C_STATE_READY;
    ll_i2c_handle.Process = I2C_DONE;

    hrtc_local.rtc_i2c = &ll_i2c_handle;
    hrtc_local.address = RV3129_ADDR;

    return &hrtc_local;
}

RV3129_HandleTypeDef* RV3129_GetHandle(void)
{
    return &hrtc_local;
}

Std_ReturnType RV3129_GetTime(RV3129_HandleTypeDef *hrtc, s_DateTime *datetime)
{
    uint8_t buffer[7];
    Std_ReturnType status;

    status = I2C_Read(hrtc->rtc_i2c, hrtc->address, RV3129_SECONDS, buffer, 7);
    if (status != E_OK)
    {
        return status;
    }

    datetime->second = bcd2dec(buffer[0]);
    datetime->minute = bcd2dec(buffer[1]);
    datetime->hour   = bcd2dec(buffer[2]);
    datetime->day    = bcd2dec(buffer[3]);
    datetime->month  = bcd2dec(buffer[5]);
    datetime->year   = bcd2dec(buffer[6]);

    return E_OK;
}

Std_ReturnType RV3129_SetTime(RV3129_HandleTypeDef *hrtc, s_DateTime *datetime)
{
    uint8_t buffer[7];

    buffer[0] = dec2bcd(datetime->second);
    buffer[1] = dec2bcd(datetime->minute);
    buffer[2] = dec2bcd(datetime->hour);
    buffer[3] = dec2bcd(datetime->day);
    buffer[4] = dec2bcd(1);
    buffer[5] = dec2bcd(datetime->month);
    buffer[6] = dec2bcd(datetime->year);

    return I2C_Write(hrtc->rtc_i2c, hrtc->address, RV3129_SECONDS, buffer, 7);
}

Std_ReturnType RV3129_GetTemp(RV3129_HandleTypeDef *hrtc, int16_t *pTemp)
{
    uint8_t temp;
    Std_ReturnType status;

    status = I2C_Read(hrtc->rtc_i2c, hrtc->address, RV3129_TEMP, &temp, 1);
    if (status != E_OK)
    {
        return status;
    }
    *pTemp = (int8_t)(temp - OFFSET_TEMP);
    return E_OK;
}
