#ifndef M4_UTILS_MACRO_MACRO_H_
#define M4_UTILS_MACRO_MACRO_H_

//#include "main.h"

#define GPIO_SetHigh(PORT, PIN)  LL_GPIO_SetOutputPin(PORT, PIN)
#define GPIO_SetLow(PORT, PIN)   LL_GPIO_ResetOutputPin(PORT, PIN)
#define GPIO_IsOutHigh(PORT, PIN)  (LL_GPIO_IsOutputPinSet(PORT, PIN) != 0)
#define GPIO_IsOutLow(PORT, PIN)   (LL_GPIO_IsOutputPinSet(PORT, PIN) == 0)
#define GPIO_IsInHigh(PORT, PIN)   (LL_GPIO_IsInputPinSet(PORT, PIN) != 0)
#define GPIO_IsInLow(PORT, PIN)    (LL_GPIO_IsInputPinSet(PORT, PIN) == 0)


#endif /* M4_UTILS_MACRO_MACRO_H_ */
