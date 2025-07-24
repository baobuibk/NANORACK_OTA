#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#ifndef TRUE
#define TRUE                            1U
#endif
#ifndef FALSE
#define FALSE                           0U
#endif
#ifndef NULL
#define NULL 						    ((void *)0)
#endif

/* User define ---------------------------------------------------------------*/
#define MAX_TASK                        16
#define MAX_TIMERS                      8
#define NUM_TASK_PENDING_ARRAYS         ((MAX_TASK + 31) / 32)
#define NUM_TIMER_PENDING_ARRAYS        ((MAX_TIMERS + 31) / 32)

#define USE_SCH_SEMAPHORE

#ifdef USE_SCH_SEMAPHORE
#define MAX_SEMAPHORES                  4
#endif

/* Exported types ------------------------------------------------------------*/
typedef enum {
  SCH_DONE,
  SCH_ERROR
} SCH_Status;

typedef void (*SCH_TASK) (void);
typedef void (*SCH_TimerCallback) (void);

#define SCH_TASK_HANDLE                 uint8_t
#define SCH_DEFAULT_TASK_HANDLE         0xFF
#define SCH_TIMER_HANDLE                uint8_t
#define SCH_DEFAULT_TIMER_HANDLE        0xFF

#ifdef USE_SCH_SEMAPHORE
#define SCH_SEMAPHORE_HANDLE            uint8_t
#define SCH_DEFAULT_SEMAPHORE_HANDLE    0xFF
#endif

/**
 * @brief Enumeration of task types.
 */
typedef enum SCH_TASK_TYPE
{
  SCH_TASK_NONE,    ///< No task type specified.
  SCH_TASK_SYNC,    ///< Synchronous task.
  SCH_TASK_ASYNC    ///< Asynchronous task.
} SCH_TASK_TYPE;

/**
 * @brief Enumeration of timer types.
 */
typedef enum SCH_TIMER_TYPE
{
  SCH_TIMER_NONE = 0,   ///< No timer type.
  SCH_TIMER_MONO,       ///< Monostable timer.
  SCH_TIMER_PERIODIC    ///< Periodic timer.
} SCH_TIMER_TYPE;

/**
 * @brief Enumeration of task priority levels.
 */
typedef enum SCH_TASK_PRIORITY
{
  SCH_TASK_PRIO_0 = 0,  ///< Lowest priority.
  SCH_TASK_PRIO_1,
  SCH_TASK_PRIO_2,
  SCH_TASK_PRIO_3,     ///< Highest priority.
} SCH_TASK_PRIORITY;

/**
 * @brief Structure defining task configuration properties.
 */
typedef struct SCH_TaskPropertyTypedef
{
  SCH_TASK_TYPE       taskType;           ///< Type of the task (sync/async).
  SCH_TASK_PRIORITY   taskPriority;       ///< Task priority.
  uint32_t            taskPeriodInMS;     ///< Task period in milliseconds.
  SCH_TASK            taskFunction;       ///< Pointer to the task function.
  uint32_t            taskTick;           ///< Initial tick value (used as period).
} SCH_TaskPropertyTypedef;

/**
 * @brief Structure defining timer configuration properties.
 */
typedef struct SCH_TimerPropertyTypedef
{
  SCH_TIMER_TYPE      timerType;          ///< Timer type (mono/periodic).
  uint32_t            timerPeriodInMS;    ///< Timer period in milliseconds.
  SCH_TimerCallback   timerCallbackFunction; ///< Callback function to be called when timer expires.
} SCH_TimerPropertyTypedef;

#ifdef USE_SCH_SEMAPHORE
/**
 * @brief Structure defining semaphore properties.
 */
typedef struct SCH_SemaphoreTypedef
{
  uint8_t value;  ///< Semaphore value (0 or 1 for binary semaphore)
} SCH_SemaphoreTypedef;
#endif

/**
 * @brief Enumeration of task states.
 */
typedef enum SCH_TaskStateTypedef
{
  TASK_StateHold = 0,   ///< Task is suspended or held.
  TASK_StateReady       ///< Task is ready to run.
} SCH_TaskStateTypedef;

/**
 * @brief Enumeration of timer states.
 */
typedef enum SCH_TimerStateTypedef
{
  TIM_StateStop = 0,    ///< Timer is stopped.
  TIM_StateRun          ///< Timer is running.
} SCH_TimerStateTypedef;

/**
 * @brief Enumeration of software timer identifiers.
 */
typedef enum SCH_SoftTimerTypedef
{
  SCH_TIM_FIRST = 0,
  SCH_TIM_LED = SCH_TIM_FIRST,  ///< Soft timer for LED control.
  SCH_TIM_WDT,                  ///<
  SCH_TIM_LAST                  ///< End marker for soft timers.
} SCH_SoftTimerTypedef;

/* Exported functions ------------------------------------------------------- */
void SCH_Initialize(void);

void SCH_TIM_Start(const SCH_SoftTimerTypedef timer, const uint32_t timeInMs);
uint8_t SCH_TIM_isCompleted(const SCH_SoftTimerTypedef timer);
SCH_Status SCH_TASK_ResumeTask(SCH_TASK_HANDLE taskIndex);
SCH_Status SCH_TASK_StopTask(SCH_TASK_HANDLE taskIndex);
SCH_Status SCH_TASK_CreateTask(SCH_TASK_HANDLE* pHandle, SCH_TaskPropertyTypedef* pTaskProperty);
SCH_Status SCH_TASK_TriggerAsync(SCH_TASK_HANDLE taskIndex);
void SCH_RunSystemTickTimer(void);
void SCH_StartScheduler(void);
void SCH_StopScheduler(void);
void SCH_HandleScheduledTask(void);
uint32_t SCH_SystemTick(void);

SCH_Status SCH_TIM_CreateTimer(SCH_TIMER_HANDLE* pHandle, SCH_TimerPropertyTypedef* pTimerProperty);
SCH_Status SCH_TIM_StopTimer(SCH_TIMER_HANDLE timerIndex);
SCH_Status SCH_TIM_RestartTimer(SCH_TIMER_HANDLE timerIndex);

#ifdef USE_SCH_SEMAPHORE
SCH_Status SCH_Semaphore_Create(SCH_SEMAPHORE_HANDLE* pHandle, uint8_t initialValue);
SCH_Status SCH_Semaphore_Take(SCH_SEMAPHORE_HANDLE semHandle);
SCH_Status SCH_Semaphore_Give(SCH_SEMAPHORE_HANDLE semHandle);
#endif

#ifdef __cplusplus
}
#endif


#endif /* SCHEDULER_H_ */
