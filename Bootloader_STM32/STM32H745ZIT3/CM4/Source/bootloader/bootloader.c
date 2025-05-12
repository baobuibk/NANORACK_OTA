/************************************             Includes               ************************************/
#include "bootloader.h"
#include <stdio.h>
#include <stm32h7xx_hal.h>
#include <stm32h7xx_ll_bus.h>

/************************************    Static Functions Declarations  ************************************/

bool Jump_To_App(void)
{
	 uint32_t app_address = FIRMWARE_BANK2_BASE;
	__disable_irq();

	for (uint8_t i = 0; i < 8; i++)
	{
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}

	HAL_DeInit();

	SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;
	__DSB();
	__ISB();

	__set_MSP(*((volatile uint32_t*)app_address));
	SCB->VTOR = app_address;

	__DSB();
	__ISB();

	uint32_t MainAppAddr = *((volatile uint32_t*)(app_address + 4));
	void (*reset_handler)(void) = (void(*)(void))MainAppAddr;

	__enable_irq();
	reset_handler();

	return true;
}


