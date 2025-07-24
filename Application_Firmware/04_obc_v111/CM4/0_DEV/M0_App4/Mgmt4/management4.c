/************************************************
 *  @file     : management4.c
 *  @date     : May 13, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/
#include "management4.h"

/*************************************************
 *                 User Include                  *
 *************************************************/
#include "USB_CDC/cdc_driver.h"
#include "UART_DMA/uart_dma_driver.h"
#include "OS4/scheduler.h"
#include "Task4/scheduler_tasks.h"
#include "Shared_REG/shared_reg.h"

//#include "../../_Target4/board4.h"
//#include "UART_DMA/uart_dma_driver.h"
/*************************************************
 *                   Function                    *
 *************************************************/

void Mgmt_HardwareSystemPreparing(void){
	UART_DMA_Driver_Init();
	CDC_RingBuffer_Init();
	SharedREG_Init(DIR_M4_TO_M7);
//	UART_Driver_SendString(UART_PORT, "Hello");
}

void Mgmt_SystemStart(void){

	  SCH_Initialize();

	  SchedulerTasks_Create();

	  SCH_StartScheduler();

	  while(1){
			SCH_HandleScheduledTask();
	  }
}
