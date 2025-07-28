#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "main.h"
#include <stm32h7xx.h>
#include <stm32h745xx.h>
#include "core_cm4.h"
#include <stdbool.h>

#define FIRMWARE1_CORE2_MEM_BASE	0x08140000U  	// Sector 2, 3, 4 (firmware1)
#define FIRMWARE2_CORE2_MEM_BASE	0x081A0000U  	// Sector 5, 6, 7 (firmware2)

typedef void (*pMainApp)(void);
typedef void (*Jump_Ptr)(void);

bool Jump_To_App(void);

#endif /* BOOTLOADER_H_ */
