/*
 * rtos_task.c
 *
 *  Created on: Feb 21, 2025
 *      Author: CAO HIEU
 */

/************************************************************************************************************/
/* @CaoHieu:------------------------------------------------------------------------------------------------*/
/* - Todo: Add Return function  for Task, (not NULL anymore)               								v   */
/* - Todo: Add UART Debug									                  							v	*/
/* - Todo: Syslog																						v	*/
/* - Todo: Change Ringbuffer_Uart -> Ringbuffer Utils (Init Size/Maxsize) + -> init to Driver UART  	v	*/
/* - Todo: UART-> UART DMA using FREERTOS QUEUE								  							v	*/
/* - Todo: New Commandline								  												v	*/
/* - Todo: Add Protocol Link board to board								 									*/
/* - Todo: Build TCP FreeRTOS							  													*/
/* - Todo: MMC DRIVER - FATFS							  												v	*/
/* - Todo: Bootloader to EXP							  												?	*/
/* - Todo: Bootloader for OBC							  												?	*/
/* - Todo: Tiny USB + RNDIS 							  												?	*/
/* - Todo: Thread Safe Printf							  												v	*/
/* - Todo: Reason why rtc time jump ramdomly			  												v	*/
/************************************************************************************************************/
#include "stdio.h"
#include "main.h"

#include "rtos_tasks.h"
#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "management.h"
#include "common_state.h"
#include "utils.h"
#include "uart_driver_dma.h"
//#include "cdc_driver.h"
//#include "usbd_cdc_if.h"
/*************************************************
 *                     TEST                      *
 *************************************************/
#include "system.h"
#include "mode.h"

#include "SysLog_Queue/syslog_queue.h"
#include "DateTime/date_time.h"
#include "CLI_Terminal/CLI_Src/embedded_cli.h"
#include "CLI_Terminal/CLI_Setup/cli_setup.h"
#include "SPI_FRAM/fram_spi.h"
#include "IO_ExWD-TPL5010/wd_tpl5010.h"
#include "FileSystem/filesystem.h"
#include "SPI_SlaveOfCM4/spi_slave.h"
#include "SPI_MasterOfEXP/spi_master.h"
#include "min_proto.h"
#include "MIN_Process/min_process.h"
//#include "inter_cpu_comm.h"
#include "CLI_Terminal/CLI_Auth/simple_shield.h"
#include "Dmesg/dmesg.h"

#include "modfsp.h"
#include "ScriptManager/script_manager.h"

#include "log_manager.h"
#include "AliveCM4/alive_cm4.h"

/*************************************************
 *                    Header                     *
 *************************************************/
ShieldInstance_t auth_usb;

MODFSP_Data_t cm4_protocol;

static void writeChar_auth_USB(char c) {
    uint8_t c_to_send = c;
    UART_Driver_Write(UART_USB, c_to_send);
}

Std_ReturnType OBC_AppInit(void);

#define MIN_STACK_SIZE	configMINIMAL_STACK_SIZE
#define ROOT_PRIORITY   5
#define ROOT_STACK_SIZE (configMINIMAL_STACK_SIZE * 10)

static StackType_t root_stack[ROOT_STACK_SIZE];
static StaticTask_t root_tcb;

#define CREATE_TASK(task_func, task_name, stack, param, priority, handle) \
    if (xTaskCreate(task_func, task_name, stack, param, priority, handle) != pdPASS) { \
        return E_ERROR; \
    }

/*************************************************
 *               TASK DEFINE                     *
 *************************************************/
void vSoft_RTC_Task(void *pvParameters);
void UART_DEBUG_DMA_RX_Task(void *pvParameters);
void UART_EXP_DMA_RX_Task(void *pvParameters);
void MIN_Process_Task(void *pvParameters);
void MODFSP_Process_Task(void *pvParameters);
void CLI_Handle_Task(void *pvParameters);
void vTask1_handler(void *pvParameters);
void vTask2_handler(void *pvParameters);
void vTask3_handler(void *pvParameters);
void vUART_bufferTest(void *pvParameters);
//void vUSBCheck_Task(void *argument);
//void CDC_TX_Task(void *pvParameters);
void UART_USB_DMA_RX_TASK(void *pvParameters);
void LogManager_Task(void *pvParameters);

void WatchdogPulseTask(void *pvParameters);
void WatchdogMonitorTask(void *pvParameters);
void ExpMonitorTask(void *pvParameters);

//Task-kick

typedef struct {
    const char *name;
    uint8_t alive;
} TaskHeartbeat_t;

#define TASK_COUNT 3
TaskHeartbeat_t taskHeartbeats[TASK_COUNT] = {
    {"MIN", 0},
    {"MODFSP", 0},
    {"LOG", 0}
};

volatile uint8_t watchdog_allow_pulse = 1;

void Task_Kick(const char *taskName) {
    for (int i = 0; i < TASK_COUNT; i++) {
        if (strcmp(taskHeartbeats[i].name, taskName) == 0) {
            taskHeartbeats[i].alive = 1;
            break;
        }
    }
}

#define CM4_PIN_PORT            CM4OUT_STMIN_D1_GPIO_Port
#define CM4_PIN                 CM4OUT_STMIN_D1_Pin
#define MONITOR_DEBOUNCE_MS     100

/*************************************************
 *               	Root Task	                 *
 *************************************************/
void OBC_RootTask(void *pvParameters);

void OBC_RootTask(void *pvParameters)
{
	Sys_Debugcast(E_OK, LOG_NOTICE, "Root task started");
    if (OBC_AppInit() != E_OK)
    {
        Sys_Boardcast(E_ERROR, LOG_ERROR, "!!! Application Initialization Failed");
        system_status.program_state = PROGRAM_STATE_ERROR;
    }

    vTaskDelete(NULL);
    while(1){
    	// Should not go here
    }
}

void OBC_RootGrowUp(void)
{
    TaskHandle_t task_handle = xTaskCreateStatic(
        OBC_RootTask,
        "OBC_RootTask",
        ROOT_STACK_SIZE,
        NULL,
        ROOT_PRIORITY,
        root_stack,
        &root_tcb
    );

    if (task_handle == NULL)
    {
        Sys_Boardcast(E_ERROR, LOG_FATAL, "Cannot Start Root-Task!!!");
        while (1);
    }

    vTaskStartScheduler();
}

/*************************************************
 *               	TASK INIT	                 *
 *************************************************/

Std_ReturnType OBC_AppInit(void)
{

	Std_ReturnType ret = E_ERROR;

	ret = Mgmt_SystemInitStepZero();
	if (ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_FATAL, "System Init Step Zero Failed!!!");
	}else{
    	Sys_Boardcast(E_OK, LOG_INFOR, "Step Zero: PASS!");
	}

	ret = Mgmt_SystemInitStepOne();
	if (ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_FATAL, "System Init Step One Failed!!!");
	}else{
    	Sys_Boardcast(E_OK, LOG_INFOR, "Step One: PASS!");
	}

	ret = Mgmt_SystemInitStepTwo();
	if (ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_FATAL, "System Init Step Two Failed!!!");
	}else{
    	Sys_Boardcast(E_OK, LOG_INFOR, "Step Two: PASS!");
	}

	ret = Mgmt_SystemInitFinal();
	if (ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_FATAL, "System Init Final Failed!!!");
	}else{
    	Sys_Boardcast(E_OK, LOG_INFOR, "Step Final: PASS!");
	}

	FS_Init();

	MIN_Process_Init();

	if (ret != E_OK){
		Sys_Boardcast(E_ERROR, LOG_FATAL, "System Failed!!!");
	}else{
    	Sys_Boardcast(E_OK, LOG_INFOR, "[WELCOME]");
	}

	char boot_log[256] = {0};
    int offset = 0;
    s_DateTime rtc;
    Utils_GetRTC(&rtc);
    offset += snprintf(boot_log + offset, sizeof(boot_log) - offset,
                "\r\nHardtime: 20%02d-%02d-%02d %02d:%02d:%02d ",
                rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
    uint8_t hours = 0, minutes = 0, seconds = 0;
    Utils_GetWorkingTime(NULL, &hours, &minutes, &seconds);
    offset += snprintf(boot_log + offset, sizeof(boot_log) - offset,
                    "\r\nUptime: %02u:%02u:%02u",
                    hours, minutes, seconds);
    offset += snprintf(boot_log + offset, sizeof(boot_log) - offset,
                    "\r\nWelcome to SpaceLiinTech - SLT BEE-PC1 OBC \r\n\r\n");

	for (uint32_t i = 0; boot_log[i] != '\0'; i++)
	{
	    while (!LL_USART_IsActiveFlag_TXE(USART1));
	    LL_USART_TransmitData8(USART1, (uint8_t)boot_log[i]);
	}
	while (!LL_USART_IsActiveFlag_TC(USART1));

	for (uint32_t i = 0; boot_log[i] != '\0'; i++)
	{
	    while (!LL_USART_IsActiveFlag_TXE(USART2));
	    LL_USART_TransmitData8(USART2, (uint8_t)boot_log[i]);
	}
	while (!LL_USART_IsActiveFlag_TC(USART2));


	ScriptManager_Init();

	MODFSP_Init(&cm4_protocol);


    CREATE_TASK(FS_Gatekeeper_Task, 	"FS_Gatekeeper", 	MIN_STACK_SIZE * 20, 	NULL, 									2, NULL);

    CREATE_TASK(MIN_Process_Task, 		"MIN_Process", 		MIN_STACK_SIZE * 20, 	NULL, 									1, NULL);

    CREATE_TASK(MODFSP_Process_Task, 	"MODFSP_Process", 	MIN_STACK_SIZE * 20, 	NULL, 									1, NULL);

    CREATE_TASK(SysLog_Task, 			"SysLog_Task", 		MIN_STACK_SIZE * 10, 	NULL, 									1, NULL);	// Syslog Queue from syslog_queue.c

    CREATE_TASK(vSoft_RTC_Task, 		"Soft_RTC_Task", 	MIN_STACK_SIZE * 5, 	NULL, 									1, NULL);

    CREATE_TASK(UART_DEBUG_DMA_RX_Task, "DEBUG_RX_Task", 	MIN_STACK_SIZE * 20, 	(void*)UART_DMA_Driver_Get(UART_DEBUG), 1, NULL);

    CREATE_TASK(UART_EXP_DMA_RX_Task, 	"EXP_RX_Task",	 	MIN_STACK_SIZE * 20, 	(void*)UART_DMA_Driver_Get(UART_EXP), 	1, NULL);

    CREATE_TASK(CLI_Handle_Task, 		"CLI_Handle_Task", 	MIN_STACK_SIZE * 10, 	NULL, 									1, NULL);

    CREATE_TASK(vTask1_handler, 		"vTask1", 			MIN_STACK_SIZE, 		NULL, 									1, NULL);

    CREATE_TASK(vTask2_handler, 		"vTask2", 			MIN_STACK_SIZE, 		NULL, 									1, NULL);

    		CREATE_TASK(ScriptManager_Task, 		"vTaskx", 			MIN_STACK_SIZE * 20, 	NULL, 									1, NULL);
    		CREATE_TASK(ScriptDLS_Task, 			"vTasky", 			MIN_STACK_SIZE * 20, 	NULL, 									1, NULL);
    		CREATE_TASK(ScriptCAM_Task, 			"vTaskz", 			MIN_STACK_SIZE * 20, 	NULL, 									1, NULL);

    CREATE_TASK(UART_USB_DMA_RX_TASK, 	"UART_USB_RX_Task", MIN_STACK_SIZE * 20, 	(void*)UART_DMA_Driver_Get(UART_USB),	1, NULL);

    		CREATE_TASK(LogManager_Task, 			"LogManager", 		MIN_STACK_SIZE * 5, 		NULL, 								1, NULL);

    CREATE_TASK(WatchdogMonitorTask, 	"WatchdogMonitorTask", 	MIN_STACK_SIZE * 2, 		NULL, 									1, NULL);
    CREATE_TASK(WatchdogPulseTask, 		"WatchdogPulseTask", 	MIN_STACK_SIZE * 2, 		NULL, 									1, NULL);

    CREATE_TASK(CM4_KeepAliveTask, 		"CM4_KeepAlive", 		MIN_STACK_SIZE * 5, 		NULL, 									1, NULL);

    CREATE_TASK(ExpMonitorTask, 		"ExpMonitorTask", 		MIN_STACK_SIZE * 2, 		NULL, 									1, NULL);

    vTaskDelay(pdMS_TO_TICKS(1));

	Shield_Init(&auth_usb, writeChar_auth_USB);

	Dmesg_Init();

    return E_OK;
}

/*************************************************
 *               TASK LIST                       *
 *************************************************/
void vSoft_RTC_Task(void *pvParameters)
{
    static uint32_t countingSyncTime = 1;

    while(1)
    {
        Utils_SoftTime_Update();
        countingSyncTime++;
        if(countingSyncTime > 7200)
        {
            countingSyncTime = 0;
            if(Utils_SoftTime_Sync() == E_OK)
            {
                SYSLOG_NOTICE("[Sync Time!]");
            }
        }

        vTaskDelay(1000);
    }
}

void CLI_Handle_Task(void *pvParameters)
{
	while (1)
	{
    	ShieldAuthState_t auth_state;

    	auth_state = Shield_GetState(&auth_usb);
    	if(auth_state == AUTH_ADMIN || auth_state == AUTH_USER){
        	if(auth_usb.initreset == 1){
                embeddedCliPrint(getUsbCdcCliPointer(), "");
                auth_usb.initreset = 0;
        	}
			embeddedCliProcess(getUsbCdcCliPointer());
//			embeddedCliProcess(getUartCm4CliPointer());
    	}

		vTaskDelay(100);
	}
}

void UART_USB_DMA_RX_TASK(void *pvParameters)
{
    UART_DMA_Driver_t *driver = (UART_DMA_Driver_t *)pvParameters;
    for (;;)
    {
    		// Sửa port max delay
        if (xSemaphoreTake(driver->rxSemaphore, portMAX_DELAY) == pdTRUE)
        {
            int c;
            while ((c = UART_DMA_Driver_Read(driver->uart)) != -1)
            {
                ForwardMode_t mode = ForwardMode_Get();
                if (mode == FORWARD_MODE_USB) {
                    // Forward mode: USART2 (rx) - UART7 (tx)
//                    UART_Driver_Write(UART7, (uint8_t)c);
                    UART_Driver_Write(UART_EXP, (uint8_t)c);

                    if (ForwardMode_ProcessReceivedByte((uint8_t)c)) {
                        embeddedCliPrint(getUsbCdcCliPointer(), "Forward mode disabled due to 10 consecutive '\\'.");
                    }
                } else if (mode == FORWARD_MODE_LISTEN_USB) {
                    if (ForwardMode_ProcessReceivedByte((uint8_t)c)) {
                        embeddedCliPrint(getUsbCdcCliPointer(), "Forward mode disabled due to 10 consecutive '\\'.");
                    }
                    embeddedCliReceiveChar(getUsbCdcCliPointer(), (char)c);
                    embeddedCliProcess(getUsbCdcCliPointer());
                } else {
                    // Mode NORMAL: CLI
                	ShieldAuthState_t auth_state = Shield_GetState(&auth_usb);
                	if(auth_state == AUTH_ADMIN || auth_state == AUTH_USER){
                		Shield_ResetTimer(&auth_usb);
                        embeddedCliReceiveChar(getUsbCdcCliPointer(), (char)c);
                        embeddedCliProcess(getUsbCdcCliPointer());
                	}else{
                		Shield_ReceiveChar(&auth_usb, (char)c);
                	}
                }
            }
        }
        	// Đổi timeout của xtake semapphore -> trigger watchdog task (trigger bit)
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void UART_EXP_DMA_RX_Task(void *pvParameters)
{
    UART_DMA_Driver_t *driver = (UART_DMA_Driver_t *)pvParameters;
    for (;;)
    {
        if (xSemaphoreTake(driver->rxSemaphore, 0) == pdTRUE)
        {
                ForwardMode_t mode = ForwardMode_Get();
                if (mode == FORWARD_MODE_UART) {
                    // Forward mode (CM4): UART7 RX -> UART_DEBUG
                    int c;
                    while ((c = UART_DMA_Driver_Read(driver->uart)) != -1)
                    {
						UART_Driver_Write(UART_DEBUG, (uint8_t)c);
                    }
//                    embeddedCliReceiveChar(getUartCm4CliPointer(), (char)c);
                } else if (mode == FORWARD_MODE_USB) {
                    // Forward mode (USB): UART7 -> to CDC
                    int c;
                    while ((c = UART_DMA_Driver_Read(driver->uart)) != -1)
                    {
						UART_Driver_Write(UART_USB, (uint8_t)c);
                    }
                } else if (mode == FORWARD_MODE_LISTEN_CM4) {
                    // Listen mode (CM4): UART7 RX -> UART_DEBUG
                    int c;
                    while ((c = UART_DMA_Driver_Read(driver->uart)) != -1)
                    {
						UART_Driver_Write(UART_DEBUG, (uint8_t)c);
                    }
                } else if (mode == FORWARD_MODE_LISTEN_USB) {
                    // Listen mode (USB):UART7 RX -> CDC
                    int c;
                    while ((c = UART_DMA_Driver_Read(driver->uart)) != -1)
                    {
						UART_Driver_Write(UART_USB, (uint8_t)c);
                    }
                } else {
                    // NORMAL mode: Default processing (e.g., logging or ignoring data)
                }
//                if (ForwardMode_ProcessReceivedByte((uint8_t)c)) {
//                    if (mode == FORWARD_MODE_UART || mode == FORWARD_MODE_LISTEN_CM4) {
//                        embeddedCliPrint(getUartCm4CliPointer(), "Listen/Forward mode disabled due to 10 consecutive '\\'.");
//                    } else if (mode == FORWARD_MODE_USB || mode == FORWARD_MODE_LISTEN_USB) {
//                        embeddedCliPrint(getUsbCdcCliPointer(), "Listen/Forward mode disabled due to 10 consecutive '\\'.");
//                    }
//                }
            }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

}


void UART_DEBUG_DMA_RX_Task(void *pvParameters)
{
    UART_DMA_Driver_t *driver = (UART_DMA_Driver_t *)pvParameters;
    for (;;)
    {
        if (xSemaphoreTake(driver->rxSemaphore, portMAX_DELAY) == pdTRUE)
        {

                ForwardMode_t mode = ForwardMode_Get();
                if (mode == FORWARD_MODE_UART) {
                    // Forward mode: USART2 (rx) - UART7 (tx)
//                    UART_Driver_Write(UART7, (uint8_t)c);
                    int c;
                    while ((c = UART_DMA_Driver_Read(driver->uart)) != -1)
                    {
						UART_Driver_Write(UART_EXP, (uint8_t)c);
                    }

                } else if (mode == FORWARD_MODE_LISTEN_CM4) {
                    int c;
                    while ((c = UART_DMA_Driver_Read(driver->uart)) != -1)
                    {
                    	UART_Driver_Write(UART_DEBUG, (uint8_t)c);
                    }

                } else {
                    // Mode NORMAL: CLI
//                    embeddedCliReceiveChar(getUartCm4CliPointer(), (char)c);
//                    embeddedCliProcess(getUartCm4CliPointer());
                }
            }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

}

void MODFSP_Process_Task(void *pvParameters)
{
	while(1){
		Task_Kick("MODFSP");
        ForwardMode_t mode = ForwardMode_Get();
        if (mode == FORWARD_MODE_NORMAL) {
            if (UART_DMA_Driver_IsDataAvailable(UART_DEBUG)) {
            	MODFSP_Process(&cm4_protocol);
            }else{
            	MODFSP_Process(&cm4_protocol);
            }
        }
	    vTaskDelay(pdMS_TO_TICKS(1));
	}
}


void MIN_Process_Task(void *pvParameters)
{
	while(1){
		Task_Kick("MIN");
        ForwardMode_t mode = ForwardMode_Get();
        if (mode == FORWARD_MODE_NORMAL) {
        	MIN_Processing();
        }
	    vTaskDelay(pdMS_TO_TICKS(1));
	}
}

void vTask1_handler(void *pvParameters)
{
	while (1)
	{
		GPIO_SetLow(LED0_Port, LED0);
		vTaskDelay(1000);

		GPIO_SetHigh(LED0_Port, LED0);
		vTaskDelay(1000);
	}
}

void vTask2_handler(void *pvParameters)
{
	while (1)
	{
		GPIO_SetHigh(LED1_Port, LED1);
		vTaskDelay(1000);
		GPIO_SetLow(LED1_Port, LED1);
		vTaskDelay(1000);
	}
}

void vTask3_handler(void *pvParameters)
{
	static uint8_t counting = 0;
	while (1)
	{
		char buffer[64];
	    snprintf(buffer, sizeof(buffer), "60s Cycle Heartbeat: %d", counting++);
		SYSLOG_NOTICE(buffer);
		vTaskDelay(60000);
	}
}\

void LogManager_Task(void *pvParameters)
{
    for (;;) {
    	Task_Kick("LOG");
    	LogManager_Process();
        vTaskDelay(100);
    }
}

void WatchdogMonitorTask(void *pvParameters)
{
    for (;;) {
        int allAlive = 1;
        for (int i = 0; i < TASK_COUNT; i++) {
            if (taskHeartbeats[i].alive == 0) {
                allAlive = 0;
                SYSLOG_WARN("Missed heartbeat");
            }
        }

        if (allAlive) {
            watchdog_allow_pulse = 1;
        } else {
            watchdog_allow_pulse = 0;
        }

        for (int i = 0; i < TASK_COUNT; i++) {
            taskHeartbeats[i].alive = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void WatchdogPulseTask(void *pvParameters)
{
    for (;;) {
        if (watchdog_allow_pulse) {
            Watchdog_Device_Update();
        }

        if (Watchdog_Device_GetState() == WATCHDOG_STATE_HIGH) {
            vTaskDelay(pdMS_TO_TICKS(HIGH_PERIOD));
        } else {
            vTaskDelay(pdMS_TO_TICKS(LOW_PERIOD));
        }
    }
}

void ExpMonitorTask(void *pvParameters) {
    uint8_t lastLow = GPIO_IsInLow(CM4_PIN_PORT, CM4_PIN);
    uint32_t lastChangeTime = xTaskGetTickCount();

    for (;;) {
        if (!ExpMonitor_IsEnabled()) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        uint8_t isLow = GPIO_IsInLow(CM4_PIN_PORT, CM4_PIN);
        uint32_t now = xTaskGetTickCount();

        if (isLow != lastLow) {
            lastLow = isLow;
            lastChangeTime = now;
        } else {
            uint32_t elapsedMs = (now - lastChangeTime) * portTICK_PERIOD_MS;
            if (isLow && elapsedMs >= MONITOR_DEBOUNCE_MS) {
                // Low > 100 ms: reset EXP  UART-forward
            	MIN_Send_PLEASE_RESET_CMD();
                ForwardMode_Set(FORWARD_MODE_UART);
                ExpMonitor_SetEnabled(0);
            }
            else if (!isLow && elapsedMs >= MONITOR_DEBOUNCE_MS) {
                // High > 100 ms: NORMAL mode
                ForwardMode_Set(FORWARD_MODE_NORMAL);
                ExpMonitor_SetEnabled(0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


/*************************************************
 *                    END                        *
 *************************************************/


