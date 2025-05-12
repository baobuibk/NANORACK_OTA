/*
 * bootmode.c
 *
 *  Created on: May 2, 2025
 *      Author: DELL
 */

#include "bootmode.h"

/*static void clear_all_ram(void)
{
    // Xóa RAMD1
    uint32_t *ram_start = (uint32_t *)0x24000000; // Địa chỉ bắt đầu RAMD1
    uint32_t ram_size = 0xC000; // Tổng kích thước RAMD1 (3KB)
    for (uint32_t i = 0; i < ram_size / 4; i++)
    {
        ram_start[i] = 0;
    }

}*/


void full_system_reset(void)
{
    //clear_all_ram(); // Xóa RAM
//    __DSB();
    __disable_irq();
    __HAL_RCC_CLEAR_RESET_FLAGS();
    NVIC_SystemReset();
//    while (1);
}
