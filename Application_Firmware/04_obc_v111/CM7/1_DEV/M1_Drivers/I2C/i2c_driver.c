/*
 * i2c_driver.c
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#include "i2c_driver.h"
#include "reinit.h"

#define I2C_TIMEOUT     10      /* 10ms */

Std_ReturnType I2C_Write(LL_I2C_HandleTypeDef *i2c, uint8_t Slave_address, uint8_t Reg_address, uint8_t *pData, uint8_t Length)
{
    uint32_t tickstart;

    tickstart = Utils_GetTick();
    while(LL_I2C_IsActiveFlag_BUSY(i2c->Instance))
    {
        if(Utils_GetTick() - tickstart > I2C_TIMEOUT){
        	I2C_ReInit(i2c->Instance);
            return E_BUSY;
        }
    }

    LL_I2C_HandleTransfer(i2c->Instance, (Slave_address << 1), LL_I2C_ADDRSLAVE_7BIT, Length + 1,
                          LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

    tickstart = Utils_GetTick();
    while(!LL_I2C_IsActiveFlag_TXIS(i2c->Instance))
    {
        if(LL_I2C_IsActiveFlag_NACK(i2c->Instance))
            return E_ERROR;
        if(Utils_GetTick() - tickstart > I2C_TIMEOUT){
        	I2C_ReInit(i2c->Instance);
            return E_TIMEOUT;
        }
    }
    LL_I2C_TransmitData8(i2c->Instance, Reg_address);

    for(uint8_t i = 0; i < Length; i++)
    {
        tickstart = Utils_GetTick();
        while(!LL_I2C_IsActiveFlag_TXIS(i2c->Instance))
        {
            if(LL_I2C_IsActiveFlag_NACK(i2c->Instance))
                return E_ERROR;
            if(Utils_GetTick() - tickstart > I2C_TIMEOUT){
            	I2C_ReInit(i2c->Instance);
                return E_TIMEOUT;
            }
        }
        LL_I2C_TransmitData8(i2c->Instance, pData[i]);
    }

    tickstart = Utils_GetTick();
    while(!LL_I2C_IsActiveFlag_TXE(i2c->Instance))
    {
        if(Utils_GetTick() - tickstart > I2C_TIMEOUT){
        	I2C_ReInit(i2c->Instance);
            return E_TIMEOUT;
        }
    }
    return E_OK;
}

Std_ReturnType I2C_Read(LL_I2C_HandleTypeDef *i2c, uint8_t Slave_address, uint8_t Reg_address,
                                  uint8_t *pData, uint8_t Length)
{
    uint32_t tickstart;

    tickstart = Utils_GetTick();
    while(LL_I2C_IsActiveFlag_BUSY(i2c->Instance))
    {
        if(Utils_GetTick() - tickstart > I2C_TIMEOUT){
        	I2C_ReInit(i2c->Instance);
            return E_BUSY;
        }
    }

    LL_I2C_HandleTransfer(i2c->Instance, (Slave_address << 1), LL_I2C_ADDRSLAVE_7BIT, 1,
                          LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

    tickstart = Utils_GetTick();
    while(!LL_I2C_IsActiveFlag_TXIS(i2c->Instance))
    {
        if(LL_I2C_IsActiveFlag_NACK(i2c->Instance))
            return E_ERROR;
        if(Utils_GetTick() - tickstart > I2C_TIMEOUT){
        	I2C_ReInit(i2c->Instance);
            return E_TIMEOUT;
        }
    }
    LL_I2C_TransmitData8(i2c->Instance, Reg_address);

    tickstart = Utils_GetTick();
    while(!LL_I2C_IsActiveFlag_TXE(i2c->Instance))
    {
        if(LL_I2C_IsActiveFlag_NACK(i2c->Instance))
            return E_ERROR;
        if(Utils_GetTick() - tickstart > I2C_TIMEOUT){
        	I2C_ReInit(i2c->Instance);
            return E_TIMEOUT;
        }
    }

    LL_I2C_AcknowledgeNextData(i2c->Instance, LL_I2C_ACK);

    LL_I2C_HandleTransfer(i2c->Instance, (Slave_address << 1), ((Slave_address << 1) | 1),
                          Length, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

    tickstart = Utils_GetTick();
    for(uint8_t i = 0; i < Length; i++)
    {
        tickstart = Utils_GetTick();
        while(!LL_I2C_IsActiveFlag_RXNE(i2c->Instance))
        {
            if(LL_I2C_IsActiveFlag_NACK(i2c->Instance))
                return E_ERROR;
            if(Utils_GetTick() - tickstart > I2C_TIMEOUT){
            	I2C_ReInit(i2c->Instance);
                return E_TIMEOUT;
            }
        }
        pData[i] = LL_I2C_ReceiveData8(i2c->Instance);
    }
    return E_OK;
}


