/*
 * i2c_driver.h
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#ifndef M1_DRIVERS_I2C_I2C_DRIVER_H_
#define M1_DRIVERS_I2C_I2C_DRIVER_H_
#include "utils.h"
#include "stm32h7xx_ll_i2c.h"

typedef enum
{
  I2C_STATE_RESET             = 0x00U,   /*!< Peripheral is not yet Initialized         */
  I2C_STATE_READY             = 0x01U,   /*!< Peripheral Initialized and ready for use  */
  I2C_STATE_BUSY              = 0x02U,   /*!< An internal process is ongoing            */
  I2C_STATE_BUSY_TX           = 0x03U,   /*!< Data Transmission process is ongoing      */
  I2C_STATE_BUSY_RX           = 0x04U,   /*!< Data Reception process is ongoing         */
  I2C_STATE_LISTEN            = 0x05U,   /*!< Address Listen Mode is ongoing            */
  I2C_STATE_BUSY_TX_LISTEN    = 0x06U,   /*!< Address Listen Mode and Data Transmission
                                                 process is ongoing                         */
  I2C_STATE_BUSY_RX_LISTEN    = 0x07U,   /*!< Address Listen Mode and Data Reception
                                                 process is ongoing                         */
  I2C_STATE_ABORT             = 0x08U,   /*!< Abort user request ongoing                */

} LL_I2C_StateTypeDef;

typedef enum
{
  I2C_DONE             		  = 0x10U,
  I2C_START_SB                = 0x11U,
  I2C_ADDRSENT_ADDR           = 0x12U,
  I2C_REGSENT_TXIS            = 0x13U,
  I2C_TRANSMDATA_TXIS         = 0x14U,
  I2C_REREADDATA_RERX         = 0x15U,
  I2C_READWAITING    		  = 0x16U,
  I2C_READPROCESSING          = 0x17U,
  I2C_TRANSFIN_TCR            = 0x18U,
  I2C_STOP_SF				  = 0x19U,
} I2C_Process_StateTypeDef;


typedef struct LL_I2C_HandleTypeDef
{
  I2C_TypeDef               			 	*Instance;      		/*!< I2C registers base address                */

  uint8_t                    				*pBuffer;      			/*!< Pointer to I2C transfer buffer            */

  volatile uint8_t              			vSize;     				/*!< I2C transfer size counter                 */

  volatile LL_I2C_StateTypeDef  			PreviousState;  		/*!< I2C communication Previous state          */

  volatile LL_I2C_StateTypeDef  			State;          		/*!< I2C communication state                   */

  volatile I2C_Process_StateTypeDef         Process; 				/*!< I2C Processing Event counter              */

  volatile uint8_t              			SlaveAddress;     		/*!< I2C Target device address                 */

  volatile uint8_t              			RegAddress;     		/*!< I2C Target register address               */

  Std_ReturnType							ErrorCode;				/*!< I2C Error code when running               */

} LL_I2C_HandleTypeDef;

Std_ReturnType I2C_Write(LL_I2C_HandleTypeDef *i2c, uint8_t Slave_address, uint8_t Reg_address, uint8_t *pData, uint8_t Length);
Std_ReturnType I2C_Read(LL_I2C_HandleTypeDef *i2c, uint8_t Slave_address, uint8_t Reg_address,uint8_t *pData, uint8_t Length);

#endif /* M1_DRIVERS_I2C_I2C_DRIVER_H_ */
