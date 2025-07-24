/*
 * reinit.h
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#ifndef M0_APP_MGMT_REINIT_H_
#define M0_APP_MGMT_REINIT_H_

void I2C_ReInit(I2C_TypeDef *I2Cx);
void SDMMC1_ReInit(void);
void SDMMC1_DeInit(void);
void SDMMC1_Init(void);

#endif /* M0_APP_MGMT_REINIT_H_ */
