#include "scheduler_tasks.h"

typedef struct
{
    SCH_TASK_HANDLE taskHandle;
    SCH_TaskPropertyTypedef taskProperty;
} TaskEntry;

static SCH_TASK_HANDLE asyncTaskHandle;

/*************************************************
 *                 User Include                  *
 *************************************************/
#include "main.h"
#include "../../_Target4/board4.h"
#include "usbd_cdc_if.h"
#include "UART_DMA/uart_dma_driver.h"
#include "USB_CDC/cdc_driver.h"
#include "Shared_REG/shared_reg.h"
/*************************************************
 *                  Task Define                  *
 *************************************************/
static void BlinkLed1_Task(void);
static void BlinkLed2_Task(void);
static void USB_Check_Task(void);
static void CDC_TxPoll_Task(void);
static void CDC_RxPoll_Task(void);
static void UART_Port_RxPoll_Task(void);

/*************************************************
 *                   Task Infor                  *
 *************************************************/
/**
 * @brief Example task definitions. Users can modify or define their own tasks.
 * e.g BlinkLed 1000ms cycle, HelloWorld as Async trigger by BlinkLed->On
 */
static TaskEntry schedulerTasks[] = {
    {SCH_DEFAULT_TASK_HANDLE, {SCH_TASK_SYNC,  SCH_TASK_PRIO_0, 1000,    BlinkLed1_Task,     		0}},
    {SCH_DEFAULT_TASK_HANDLE, {SCH_TASK_SYNC,  SCH_TASK_PRIO_0, 1000,    BlinkLed2_Task,     		0}},
    {SCH_DEFAULT_TASK_HANDLE, {SCH_TASK_SYNC,  SCH_TASK_PRIO_0, 5,       CDC_RxPoll_Task,    		0}},
    {SCH_DEFAULT_TASK_HANDLE, {SCH_TASK_SYNC,  SCH_TASK_PRIO_0, 5,       CDC_TxPoll_Task,    		0}},
    {SCH_DEFAULT_TASK_HANDLE, {SCH_TASK_SYNC,  SCH_TASK_PRIO_0, 1,       UART_Port_RxPoll_Task,     0}},
    {SCH_DEFAULT_TASK_HANDLE, {SCH_TASK_SYNC,  SCH_TASK_PRIO_1, 100,     USB_Check_Task,     		0}},
};

/*************************************************
 *                    Task List                  *
 *************************************************/
static void BlinkLed1_Task(void)
{
    static uint8_t ledState = 0;
    if (ledState == 0)
    {
        ledState = 1;
        LL_GPIO_SetOutputPin(MCU_IO_DEBUG_LED2_GPIO_Port, MCU_IO_DEBUG_LED2_Pin);
    }
    else
    {
        LL_GPIO_ResetOutputPin(MCU_IO_DEBUG_LED2_GPIO_Port, MCU_IO_DEBUG_LED2_Pin);
        ledState = 0;
    }
}

static void BlinkLed2_Task(void)
{
//    static uint8_t ledState = 0;
//    if (ledState == 0)
//    {
//        ledState = 1;
//        LL_GPIO_ResetOutputPin(MCU_IO_DEBUG_LED3_GPIO_Port, MCU_IO_DEBUG_LED3_Pin);
//    }
//    else
//    {
//        LL_GPIO_SetOutputPin(MCU_IO_DEBUG_LED3_GPIO_Port, MCU_IO_DEBUG_LED3_Pin);
//        ledState = 0;
//    }
}

static void CDC_RxPoll_Task(void)
{
    if (!CDC_getRxReady()) return;
    CDC_setRxReady(0);

    uint8_t ch;
    while (CDC_RX_RingBuffer_Get(&ch))
    {
        UART_Driver_Write(UART_PORT, ch);
    }
}

static void CDC_TxPoll_Task(void)
{
    if (!CDC_IsTxReady())
    {
        return;
    }

    uint8_t pkt[CDC_USB_MAX_PKT];
    uint16_t cnt = 0;
    while (cnt < CDC_USB_MAX_PKT && CDC_TX_RingBuffer_Get(&pkt[cnt]))
    {
        ++cnt;
    }

    if (cnt == 0)
    {
        return;
    }

    if (CDC_Transmit_FS(pkt, cnt) != USBD_OK)
    {
        return;
    }
}

static void USB_Check_Task(void)
{
    if (USB_checkUSB())
    {
    	USB_setCheckFlag(0);
        uint8_t comPortState = CDC_ComPort_IsOpen() ? 1 : 0;
        if(comPortState){
        	LL_GPIO_ResetOutputPin(MCU_IO_DEBUG_LED3_GPIO_Port, MCU_IO_DEBUG_LED3_Pin);
        }else{
        	LL_GPIO_SetOutputPin(MCU_IO_DEBUG_LED3_GPIO_Port, MCU_IO_DEBUG_LED3_Pin);
        }
        SharedREG_Write(DIR_M4_TO_M7, 0, comPortState);
    }
}
static void UART_Port_RxPoll_Task(void)
{

        int c;
        while ((c = UART_DMA_Driver_Read(UART_PORT)) != -1)
        {
            CDC_SendChar((char)c);
        }

}

/*************************************************
 *                  Helper Define                *
 *************************************************/

#define NUM_SCHEDULER_TASKS (sizeof(schedulerTasks) / sizeof(schedulerTasks[0]))

void SchedulerTasks_Create(void)
{
    for (uint8_t i = 0; i < NUM_SCHEDULER_TASKS; i++)
    {
        SCH_TASK_CreateTask(&schedulerTasks[i].taskHandle, &schedulerTasks[i].taskProperty);
        if (schedulerTasks[i].taskProperty.taskType == SCH_TASK_ASYNC)
        {
            asyncTaskHandle = schedulerTasks[i].taskHandle;
        }
    }
}

SCH_Status SchedulerTasks_RegisterTask(SCH_TASK_HANDLE *pHandle, SCH_TaskPropertyTypedef *pTaskProperty)
{
    SCH_Status status = SCH_TASK_CreateTask(pHandle, pTaskProperty);
    if (status == SCH_DONE && pTaskProperty->taskType == SCH_TASK_ASYNC)
    {
        asyncTaskHandle = *pHandle;
    }
    return status;
}
