#ifndef SCHEDULER_TASKS_H
#define SCHEDULER_TASKS_H

#include "OS4/scheduler.h"

#ifdef SCHEDULER_LOG
#define SCH_LOG(fmt, ...) do { /* User-defined logging, e.g., printf(fmt, ##__VA_ARGS__) */ } while (0)
#endif

/**
 * @brief Initialize and create default tasks (e.g., LED blink, HelloWorld).
 */
void SchedulerTasks_Create(void);

/**
 * @brief Register a new task with the scheduler.
 * @param pHandle Pointer to store the task handle.
 * @param pTaskProperty Task configuration properties.
 * @retval SCH_DONE on success, SCH_ERROR on failure.
 */
SCH_Status SchedulerTasks_RegisterTask(SCH_TASK_HANDLE *pHandle, SCH_TaskPropertyTypedef *pTaskProperty);

#endif
