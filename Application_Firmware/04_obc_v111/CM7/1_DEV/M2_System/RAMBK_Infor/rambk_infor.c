
#include "rambk_infor.h"
#include <stm32h7xx_hal.h>
#include <stm32h7xx_ll_bus.h>

wdg_no_init_vars no_init_vars __attribute__((section (".bkpram")));

static void EnableBackupRAM(void);
static void DisableBackupRAM(void);
static void update_no_init_vars(uint32_t reset_cause);


static void update_no_init_vars(uint32_t reset_cause)
{
	EnableBackupRAM();
    memset(&no_init_vars, 0, sizeof(no_init_vars));
    no_init_vars.magic = WDG_NO_INIT_VARS_MAGIC;
    no_init_vars.reset_cause = reset_cause;
    no_init_vars.reset_wdg_id = 0;
    DisableBackupRAM();
}


static void EnableBackupRAM(void)
{
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_BKPRAM_CLK_ENABLE();
    HAL_PWREx_EnableBkUpReg();
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_BRR));
}

static void DisableBackupRAM(void)
{
    HAL_PWREx_DisableBkUpReg();
    __HAL_RCC_BKPRAM_CLK_DISABLE();
    HAL_PWR_DisableBkUpAccess();
}

uint8_t SystemOnBootloader_Reset(void)
{
	update_no_init_vars(RESET_CAUSE_BOOTLOADER);
    __disable_irq();
    __HAL_RCC_CLEAR_RESET_FLAGS();
    HAL_DeInit();
    NVIC_SystemReset();

	return FOTA_SUCCESS;
}
