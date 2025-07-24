/*
 * management.h
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */
#include "utils.h"

Std_ReturnType Mgmt_HardwareSystemPreparing(void);
void Mgmt_SystemStart(void);

Std_ReturnType Mgmt_SystemInitStepZero(void);
Std_ReturnType Mgmt_SystemInitStepOne(void);
Std_ReturnType Mgmt_SystemInitStepTwo(void);
Std_ReturnType Mgmt_SystemInitFinal(void);

