/*
 * management.c
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#include "management.h"
#include "rtos_tasks.h"
#include "utils.h"
#include "rtos.h"
#include "uart_driver_dma.h"
#include "main.h"
#include "board.h"
#include "common_state.h"

#include "filesystem.h"
#include "CLI_Terminal/CLI_Setup/cli_setup.h"
#include "SysLog_Queue/syslog_queue.h"
#include "DateTime/date_time.h"
#include "devices.h"
#include "SPI_FRAM/fram_spi.h"
#include "IO_ExWD-TPL5010/wd_tpl5010.h"
#include "SPI_SlaveOfCM4/spi_slave.h"
#include "SPI_MasterOfEXP/spi_master.h"
#include "shared_reg.h"

#include "lwl.h"
#include "log_manager.h"

SystemStatus_t Mgmt_GetSystemStatus(void);

Std_ReturnType Mgmt_HardwareSystemPreparing(void)
{
	Std_ReturnType ret = E_ERROR;
	system_status.init_state = INIT_STATE_STEP_PREPARING;

	RV3129_Driver_Init(I2C_RTC);
	FRAM_SPI_Driver_Init(SPI_MEM, FRAM_CS_Port, FRAM_CS);
	ret = UART_DMA_Driver_Init();

    Watchdog_Device_Init();

	Utils_SoftTime_Init();

	SharedREG_Init(DIR_M7_TO_M4);

	Sys_Debugcast(E_OK,	LOG_NOTICE , 	"OBC OS Preparing!");
	Sys_Debugcast(E_OK,	LOG_INFOR ,  	"OBC OS Preparing!");
	Sys_Debugcast(E_OK,	LOG_DEBUG, 		"OBC OS Preparing!");
	Sys_Debugcast(E_OK, LOG_WARN, 		"OBC OS Preparing!");
	Sys_Debugcast(E_OK, LOG_ERROR, 		"OBC OS Preparing!");
	Sys_Debugcast(E_OK, LOG_FATAL, 		"OBC OS Preparing!");
	return ret;
}

void Mgmt_SystemStart(void){
	Sys_Debugcast(E_OK,	LOG_NOTICE , 	"OBC OS Starting!");

	OBC_RTOS_Start();
}

//********************************************** RTOS Applied ****************************************************

/*************************************************
 *                   RTOS Control                *
 *************************************************/
Std_ReturnType Mgmt_SystemInitStepZero(void)
{
	Std_ReturnType ret = E_ERROR;
	system_status.init_state = INIT_STATE_STEP_ZERO;
	Sys_Debugcast(E_OK, LOG_INFOR, "Step Zero: Pending...");

	LWL_Init();

	LogManager_Init();

	LWL_TestLogs();

	ret = Utils_SoftTime_Sync();

	if(Utils_SoftTime_Sync() == E_OK){
		Sys_Boardcast(E_OK,	LOG_NOTICE, "[Sync Time!]");

	    s_DateTime now;
	    Utils_GetRTC(&now);
	    uint16_t full_year = 2000 + now.year;
	    LWL_Log(OBC_STM32_BOOTING, now.day, now.month, full_year, now.hour, now.minute, now.second);

	}else{
		system_status.last_error = ret;
		system_status.init_state = INIT_STATE_FAILED;
	}



	return ret;
}

Std_ReturnType Mgmt_SystemInitStepOne(void)
{
	system_status.init_state = INIT_STATE_STEP_ONE;
	Std_ReturnType ret = E_ERROR;
	Sys_Debugcast(E_OK, LOG_INFOR, "Step One: Pending...");

	ret = SystemCLI_Init();
	if(ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_ERROR, "[CLI-Interface Init Fail]");
		system_status.last_error = ret;
		system_status.init_state = INIT_STATE_FAILED;
	}else{
		Sys_Boardcast(E_OK, LOG_NOTICE, "[CLI-Interface Init Done]");
	}

	ret = Link_SDFS_Driver();
	if(ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_ERROR, "[Link FATFS Fail]");
		system_status.last_error = ret;
		system_status.init_state = INIT_STATE_FAILED;
	}else{
		Sys_Boardcast(E_OK, LOG_NOTICE, "[Link FATFS Success]");
	}

	SysLogQueue_Init();

	return ret;
}

Std_ReturnType Mgmt_SystemInitStepTwo(void)
{
	system_status.init_state = INIT_STATE_STEP_TWO;
	Std_ReturnType ret = E_ERROR;
	Sys_Debugcast(E_OK, LOG_INFOR, "Step Two: Pending...");

	ret = SPI_SlaveDevice_Init();
	if(ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_ERROR, "[SPI Device Init Fail]");
		system_status.last_error = ret;
		system_status.init_state = INIT_STATE_FAILED;
	}else{
		Sys_Boardcast(E_OK, LOG_NOTICE, "[SPI Device Init Done]");
	}

	ret = SPI_MasterDevice_Init(SPI6, SPI6_EXP_CS_GPIO_Port, SPI6_EXP_CS_Pin);
	if(ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_ERROR, "[SPI Master Init Fail]");
		system_status.last_error = ret;
		system_status.init_state = INIT_STATE_FAILED;
	}else{
		Sys_Boardcast(E_OK, LOG_NOTICE, "[SPI Master Init Done]");
	}

	return ret;
}

Std_ReturnType Mgmt_SystemInitFinal(void)
{
	system_status.init_state = INIT_STATE_FINAL;
	system_status.init_state = INIT_STATE_COMPLETED;
	Sys_Debugcast(E_OK, LOG_INFOR, "Step Final: Pending...");
	return E_OK;
}

SystemStatus_t Mgmt_GetSystemStatus(void)
{
    return system_status;
}
