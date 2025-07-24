/* Standard Includes ---------------------------------------------------------*/
#include "scheduler.h"
#include "string.h"

#ifndef SET
#define SET 						    1U
#endif
#ifndef RESET
#define RESET 						    0U
#endif

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief Task context structure.
 *
 * This structure holds the runtime context for a task, including its state,
 * a separate tick counter (currentTick), and a copy of its configuration properties.
 */
typedef struct SCH_TaskContextTypedef
{
    SCH_TaskStateTypedef taskState;       ///< Current state of the task.
    uint32_t currentTick;                 ///< Runtime tick counter for the task.
    SCH_TaskPropertyTypedef taskProperty; ///< Copy of the task configuration.
} SCH_TaskContextTypedef;

/**
 * @brief Timer context structure.
 *
 * This structure holds the runtime context for a timer, including its state,
 * a separate tick counter (currentTick), and a copy of its configuration properties.
 */
typedef struct SCH_TimerContextTypedef
{
    SCH_TimerStateTypedef timerState;       ///< Current state of the timer.
    uint32_t currentTick;                   ///< Runtime tick counter for the timer.
    SCH_TimerPropertyTypedef timerProperty; ///< Copy of the timer configuration.
} SCH_TimerContextTypedef;

/* Private global variable -----------------------------------------------*/
static uint8_t s_SchedulerRunning = FALSE;

static SCH_TaskContextTypedef s_TaskContext[MAX_TASK];
static uint8_t s_NumOfTaskScheduled;
static SCH_TimerContextTypedef s_TimerContext[MAX_TIMERS];
static uint8_t s_NumOfTimers;

#ifdef USE_SCH_SEMAPHORE
static SCH_SemaphoreTypedef s_SemaphoreContext[MAX_SEMAPHORES];
static uint8_t s_NumOfSemaphores;
#endif

static volatile uint32_t s_TaskPending[NUM_TASK_PENDING_ARRAYS] = {0};
static volatile uint32_t s_TimerPending[NUM_TIMER_PENDING_ARRAYS] = {0};

volatile uint32_t s_SystemTick;
volatile uint32_t s_SoftTimers[SCH_TIM_LAST];

/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
SCH_TaskContextTypedef *SCH_TASK_GetTaskContext(SCH_TASK_HANDLE taskHandle)
{
    if (taskHandle >= 0 && taskHandle < MAX_TASK)
    {
        return &s_TaskContext[taskHandle];
    }
    return NULL;
}

/*******************************************************************************
 * @name   SCH_Initialize
 * @brief  Function initializes scheduler
 * @param  None
 * @retval None
 *****************************************************************************/
void SCH_Initialize(void)
{
    s_SystemTick = RESET;
    s_NumOfTaskScheduled = RESET;
    s_NumOfTimers = RESET;
    
#ifdef USE_SCH_SEMAPHORE
    s_NumOfSemaphores = RESET;
#endif

    s_SchedulerRunning = FALSE;

    // Initial Scheduler Context
    memset((uint8_t *)&s_TaskContext[0], RESET, (sizeof(SCH_TaskContextTypedef) * MAX_TASK));
    memset((uint8_t *)&s_TimerContext[0], RESET, (sizeof(SCH_TimerContextTypedef) * MAX_TIMERS));
    memset((uint8_t *)&s_SoftTimers[0], RESET, (sizeof(uint32_t) * SCH_TIM_LAST));
    memset((uint8_t *)&s_TaskPending[0], RESET, (sizeof(uint32_t) * NUM_TASK_PENDING_ARRAYS));
    memset((uint8_t *)&s_TimerPending[0], RESET, (sizeof(uint32_t) * NUM_TIMER_PENDING_ARRAYS));
#ifdef USE_SCH_SEMAPHORE
    memset((uint8_t *)&s_SemaphoreContext[0], RESET, (sizeof(SCH_SemaphoreTypedef) * MAX_SEMAPHORES));
#endif
}

/*******************************************************************************
 * @name   SCH_TIM_Start
 * @brief  Start Soft timer
 * @param  const SCH_SoftTimerTypedef timer - type of soft timer
 *         const uint32_t timeInMs - time in mSec
 * @retval None
 *****************************************************************************/
void SCH_TIM_Start(const SCH_SoftTimerTypedef timer, const uint32_t timeInMs)
{
    if (timer < SCH_TIM_LAST)
    {
        s_SoftTimers[timer] = timeInMs;
    }
}

/*******************************************************************************
 * @name   SCH_TIM_HasCompleted
 * @brief  Function checks the completion of soft timer
 * @param  const SCH_SoftTimerTypedef timer - type of soft timer
 * @retval TRUE / FALSE
 *****************************************************************************/
uint8_t SCH_TIM_isCompleted(const SCH_SoftTimerTypedef timer)
{
    return (s_SoftTimers[timer] == 0);
}

/*******************************************************************************
 * @name   SCH_TASK_ResumeTask
 * @brief  Function resumes suspended task
 * @param  SCH_TASK_HANDLE taskIndex - task handle
 * @retval status
 *****************************************************************************/
SCH_Status SCH_TASK_ResumeTask(SCH_TASK_HANDLE taskIndex)
{
    SCH_Status status = SCH_ERROR;

    if (taskIndex < s_NumOfTaskScheduled)
    {
        // Get Task Context
        SCH_TaskContextTypedef *pTaskContext = &s_TaskContext[taskIndex];
        pTaskContext->taskState = TASK_StateReady;
        status = SCH_DONE;
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_TASK_StopTask
 * @brief  Function stops running task
 * @param  SCH_TASK_HANDLE taskIndex - task handle
 * @retval status
 *****************************************************************************/
SCH_Status SCH_TASK_StopTask(SCH_TASK_HANDLE taskIndex)
{
    SCH_Status status = SCH_ERROR;

    if (taskIndex < s_NumOfTaskScheduled)
    {
        // Get Task Context
        SCH_TaskContextTypedef *pTaskContext = &s_TaskContext[taskIndex];
        pTaskContext->taskState = TASK_StateHold;
        status = SCH_DONE;
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_TASK_CreateTask
 * @brief  Function creates task
 * @param  SCH_TASK_HANDLE* pHandle - pointer to task handle
 *         SCH_TaskPropertyTypedef TaskProperty
 * @retval status
 *****************************************************************************/
SCH_Status SCH_TASK_CreateTask(SCH_TASK_HANDLE *pHandle, SCH_TaskPropertyTypedef *pTaskProperty)
{
    SCH_Status status = SCH_ERROR;
    if (pHandle && pTaskProperty)
    {
        if (s_NumOfTaskScheduled < MAX_TASK)
        {
            SCH_TaskContextTypedef *pTaskContext = &s_TaskContext[s_NumOfTaskScheduled];
            memcpy(&pTaskContext->taskProperty, pTaskProperty, sizeof(SCH_TaskPropertyTypedef));
            pTaskContext->currentTick = 0;
            pTaskContext->taskState = (pTaskProperty->taskType == SCH_TASK_ASYNC) 
                                                        ? TASK_StateHold : TASK_StateReady;

            *pHandle = s_NumOfTaskScheduled;
            s_NumOfTaskScheduled++;
            status = SCH_DONE;
        }
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_TASK_TriggerAsync
 * @brief  Function triggers an async task
 * @param  SCH_TASK_HANDLE taskIndex - task handle
 * @retval status
 *****************************************************************************/
SCH_Status SCH_TASK_TriggerAsync(SCH_TASK_HANDLE taskIndex)
{
    SCH_Status status = SCH_ERROR;

    if (taskIndex < s_NumOfTaskScheduled)
    {
        SCH_TaskContextTypedef *pTaskContext = &s_TaskContext[taskIndex];
        if (pTaskContext->taskProperty.taskType == SCH_TASK_ASYNC)
        {
            uint8_t index = taskIndex / 32;
            uint8_t bit = taskIndex % 32;
            s_TaskPending[index] |= (1 << bit);  // Set bit for async task
            pTaskContext->taskState = TASK_StateReady;
            status = SCH_DONE;
        }
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_TIM_CreateTimer
 * @brief  Function creates event timer
 * @param  SCH_TIMER_HANDLE* pHandle - timer handle
 *         SCH_TimerPropertyTypedef TimerProperty
 * @retval status
 *****************************************************************************/
SCH_Status SCH_TIM_CreateTimer(SCH_TIMER_HANDLE *pHandle, SCH_TimerPropertyTypedef *pTimerProperty)
{
    SCH_Status status = SCH_ERROR;
    if (pHandle && pTimerProperty)
    {
        if (s_NumOfTimers < MAX_TIMERS)
        {
            SCH_TimerContextTypedef *pTimerContext = &s_TimerContext[s_NumOfTimers];
            memcpy(&pTimerContext->timerProperty, pTimerProperty, sizeof(SCH_TimerPropertyTypedef));
            pTimerContext->currentTick = 0;
            pTimerContext->timerState = TIM_StateStop;

            *pHandle = s_NumOfTimers;
            s_NumOfTimers++;
            status = SCH_DONE;
        }
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_TIM_RestartTimer
 * @brief  Function starts / restarts event timer
 * @param  SCH_TIMER_HANDLE timerIndex - event timer index
 * @retval status
 *****************************************************************************/
SCH_Status SCH_TIM_RestartTimer(SCH_TIMER_HANDLE timerIndex)
{
    SCH_Status status = SCH_ERROR;

    if (timerIndex < s_NumOfTimers)
    {
        SCH_TimerContextTypedef *pTimerContext = &s_TimerContext[timerIndex];
        pTimerContext->currentTick = RESET;
        pTimerContext->timerState = TIM_StateRun;
        status = SCH_DONE;
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_TIM_StopTimer
 * @brief  Function stops event timer
 * @param  SCH_TIMER_HANDLE timerIndex - event timer handle
 * @retval status
 *****************************************************************************/
SCH_Status SCH_TIM_StopTimer(SCH_TIMER_HANDLE timerIndex)
{
    SCH_Status status = SCH_ERROR;

    if (timerIndex < s_NumOfTimers)
    {
        SCH_TimerContextTypedef *pTimerContext = &s_TimerContext[timerIndex];
        pTimerContext->timerState = TIM_StateStop;
        status = SCH_DONE;
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_RunSystemTickTimer
 * @brief  Function handles system tick timer
 * @param  None
 * @retval None
 *****************************************************************************/
void SCH_RunSystemTickTimer(void)
{
    if (s_SchedulerRunning)
    {
        uint8_t taskIndex;
        SCH_TaskContextTypedef *pTaskContext;
        uint8_t timerIndex;
        SCH_TimerContextTypedef *pTimerContext;
        s_SystemTick++;
        // Task
        for (taskIndex = 0; taskIndex < s_NumOfTaskScheduled; taskIndex++)
        {
            pTaskContext = &s_TaskContext[taskIndex];
            if (pTaskContext->taskProperty.taskType == SCH_TASK_SYNC && 
                pTaskContext->taskState == TASK_StateReady)
            {
                pTaskContext->currentTick += 1;
                if (pTaskContext->currentTick >= pTaskContext->taskProperty.taskPeriodInMS)
                {
                    pTaskContext->currentTick = 0;
                    uint8_t index = taskIndex / 32;
                    uint8_t bit = taskIndex % 32;
                    s_TaskPending[index] |= (1 << bit);  // Set bit for task
                }
            }
        }

        // Timer
        for (timerIndex = 0; timerIndex < s_NumOfTimers; timerIndex++)
        {
            pTimerContext = &s_TimerContext[timerIndex];
            if (TIM_StateRun == pTimerContext->timerState)
            {
                pTimerContext->currentTick += 1;
                if (pTimerContext->currentTick >= pTimerContext->timerProperty.timerPeriodInMS)
                {
                    uint8_t index = timerIndex / 32;
                    uint8_t bit = timerIndex % 32;
                    s_TimerPending[index] |= (1 << bit);  // Set bit for timer
                    pTimerContext->currentTick = 0;
                    pTimerContext->timerState = (SCH_TIMER_PERIODIC == pTimerContext->timerProperty.timerType) ? TIM_StateRun : TIM_StateStop;
                }
            }
        }

        // Soft timer
        for (timerIndex = 0; timerIndex < SCH_TIM_LAST; timerIndex++)
        {
            if (s_SoftTimers[timerIndex] > 0)
                s_SoftTimers[timerIndex]--;
        }
    }
}

/*******************************************************************************
 * @name   SCH_StartScheduler
 * @brief  Start scheduler
 * @param  None
 * @retval None
 *****************************************************************************/
void SCH_StartScheduler(void)
{
    s_SchedulerRunning = TRUE;
}

/*******************************************************************************
 * @name   SCH_StopScheduler
 * @brief  Stop scheduler
 * @param  None
 * @retval None
 *****************************************************************************/
void SCH_StopScheduler(void)
{
    // Stop Scheduler..i.e. stop system tick timer
    s_SchedulerRunning = FALSE;
    // Initialize Scheduler Context
    memset((uint8_t *)&s_TaskContext[0], RESET, (sizeof(SCH_TaskContextTypedef) * MAX_TASK));
    memset((uint8_t *)&s_TimerContext[0], RESET, (sizeof(SCH_TimerContextTypedef) * MAX_TIMERS));
    memset((uint8_t *)&s_SoftTimers[0], RESET, (sizeof(uint32_t) * SCH_TIM_LAST));
    memset((uint8_t *)&s_TaskPending[0], RESET, (sizeof(uint32_t) * NUM_TASK_PENDING_ARRAYS));
    memset((uint8_t *)&s_TimerPending[0], RESET, (sizeof(uint32_t) * NUM_TIMER_PENDING_ARRAYS));
#ifdef USE_SCH_SEMAPHORE
    memset((uint8_t *)&s_SemaphoreContext[0], RESET, (sizeof(SCH_SemaphoreTypedef) * MAX_SEMAPHORES));
#endif
}

/*******************************************************************************
 * @name   SCH_HandleScheduledTask
 * @brief  Function handles scheduled task and timer events
 * @param  None
 * @retval None
 *****************************************************************************/
void SCH_HandleScheduledTask(void)
{
    if (s_SchedulerRunning)
    {
        // Task - Process by priority from highest (SCH_TASK_PRIO_3) to lowest (SCH_TASK_PRIO_0)
        for (int8_t priority = SCH_TASK_PRIO_3; priority >= SCH_TASK_PRIO_0; priority--)
        {
            for (uint8_t index = 0; index < NUM_TASK_PENDING_ARRAYS; index++)
            {
                uint32_t pending = s_TaskPending[index];
                for (uint8_t bit = 0; bit < 32; bit++)
                {
                    if (pending & (1 << bit))
                    {
                        uint8_t taskIndex = index * 32 + bit;
                        if (taskIndex < s_NumOfTaskScheduled)
                        {
                            SCH_TaskContextTypedef *pTaskContext = &s_TaskContext[taskIndex];
                            if (pTaskContext->taskProperty.taskPriority == priority &&
                                pTaskContext->taskState == TASK_StateReady)
                            {
                                s_TaskPending[index] &= ~(1 << bit);  // Clear bit
                                pTaskContext->taskProperty.taskFunction();
                                // Async tasks, set state to Hold after execution
                                if (pTaskContext->taskProperty.taskType == SCH_TASK_ASYNC)
                                {
                                    pTaskContext->taskState = TASK_StateHold;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Timer
        for (uint8_t index = 0; index < NUM_TIMER_PENDING_ARRAYS; index++)
        {
            uint32_t pending = s_TimerPending[index];
            for (uint8_t bit = 0; bit < 32; bit++)
            {
                if (pending & (1 << bit))
                {
                    uint8_t timerIndex = index * 32 + bit;
                    if (timerIndex < s_NumOfTimers)
                    {
                        s_TimerPending[index] &= ~(1 << bit);  // Clear bit
                        SCH_TimerContextTypedef *pTimerContext = &s_TimerContext[timerIndex];
                        if (pTimerContext->timerProperty.timerCallbackFunction)
                        {
                            pTimerContext->timerProperty.timerCallbackFunction();
                        }
                    }
                }
            }
        }
    }
}

/*******************************************************************************
 * @name   SCH_SystemTick
 * @brief  Function returns system ticks
 * @param  None
 * @retval system ticks
 *****************************************************************************/
uint32_t SCH_SystemTick(void)
{
    return s_SystemTick;
}

#ifdef USE_SCH_SEMAPHORE
/*******************************************************************************
 * @name   SCH_Semaphore_Create
 * @brief  Function creates a semaphore with an initial value
 * @param  SCH_SEMAPHORE_HANDLE* pHandle - pointer to semaphore handle
 *         uint8_t initialValue - initial value of the semaphore (0 or 1 for binary)
 * @retval status
 *****************************************************************************/
SCH_Status SCH_Semaphore_Create(SCH_SEMAPHORE_HANDLE* pHandle, uint8_t initialValue)
{
    SCH_Status status = SCH_ERROR;
    if (pHandle && s_NumOfSemaphores < MAX_SEMAPHORES)
    {
        SCH_SemaphoreTypedef *pSemaphore = &s_SemaphoreContext[s_NumOfSemaphores];
        pSemaphore->value = initialValue;
        *pHandle = s_NumOfSemaphores;
        s_NumOfSemaphores++;
        status = SCH_DONE;
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_Semaphore_Take
 * @brief  Function attempts to take the semaphore
 * @param  SCH_SEMAPHORE_HANDLE semHandle - semaphore handle
 * @retval SCH_DONE if semaphore is taken, SCH_ERROR if not available
 *****************************************************************************/
SCH_Status SCH_Semaphore_Take(SCH_SEMAPHORE_HANDLE semHandle)
{
    SCH_Status status = SCH_ERROR;
    if (semHandle < s_NumOfSemaphores)
    {
        SCH_SemaphoreTypedef *pSemaphore = &s_SemaphoreContext[semHandle];
        if (pSemaphore->value > 0)
        {
            pSemaphore->value--;
            status = SCH_DONE;
        }
    }
    return status;
}

/*******************************************************************************
 * @name   SCH_Semaphore_Give
 * @brief  Function gives back the semaphore
 * @param  SCH_SEMAPHORE_HANDLE semHandle - semaphore handle
 * @retval status
 *****************************************************************************/
SCH_Status SCH_Semaphore_Give(SCH_SEMAPHORE_HANDLE semHandle)
{
    SCH_Status status = SCH_ERROR;
    if (semHandle < s_NumOfSemaphores)
    {
        SCH_SemaphoreTypedef *pSemaphore = &s_SemaphoreContext[semHandle];
        if (pSemaphore->value < 1)  // For binary semaphore, max value is 1
        {
            pSemaphore->value++;
            status = SCH_DONE;
        }
    }
    return status;
}

#endif