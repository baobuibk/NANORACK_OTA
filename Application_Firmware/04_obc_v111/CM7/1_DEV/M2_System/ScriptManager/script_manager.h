/************************************************
 *  @file     : script_manager.h
 *  @date     : Jul 19, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    Script Manager with Daily Time Points scheduling
 *    Uses daily timestamps (0-86399) for robust timing
 ************************************************/

#ifndef SCRIPT_MANAGER_H
#define SCRIPT_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "parser/bscript_parser.h"

// MODFSP Frame IDs
#define FRAME_ID_INIT           0xF0
#define FRAME_ID_DLS_ROUTINE    0xF1
#define FRAME_ID_CAM_ROUTINE    0xF2
#define FRAME_ID_HALT           0xFA
#define FRAME_HALT_ACK          0xAA

#define SEND_TIME_CMD       	0x01
#define SEND_TIME_ACK       	0x11

#define RUN_EXPERIMENT_CMD  	0x02
#define RUN_EXPERIMENT_ACK  	0x12

#define UPDATE_OBC_CMD      	0x03
#define UPDATE_OBC_ACK      	0x13

#define UPDATE_EXP_CMD      	0x04
#define UPDATE_EXP_ACK      	0x14

#define SCRIPT_ACK_INIT        	0xA0
#define SCRIPT_ACK_DLS        	0xA1
#define SCRIPT_ACK_CAM        	0xA2


// Time Point Configuration
#define MAX_TIME_POINTS         288    // Maximum points for 5-minute intervals in 24 hours
#define MIN_INTERVAL_SEC        300    // Minimum interval: 5 minutes
#define MAX_INTERVAL_SEC        86400  // Maximum interval: 24 hours (1 day)
#define SECONDS_PER_DAY         86400  // Seconds in a day

// Script Types
typedef enum {
    SCRIPT_TYPE_INIT = 0,
    SCRIPT_TYPE_DLS_ROUTINE = 1,
    SCRIPT_TYPE_CAM_ROUTINE = 2,
    SCRIPT_TYPE_COUNT = 3
} ScriptType_t;

// Script Execution States
typedef enum {
    SCRIPT_EXEC_IDLE = 0,
    SCRIPT_EXEC_RUNNING,
    SCRIPT_EXEC_WAITING,
    SCRIPT_EXEC_COMPLETED,
    SCRIPT_EXEC_ERROR,
    SCRIPT_EXEC_FAILED_MAX_RETRIES
} ScriptExecState_t;

// Time Point Structure - Uses daily timestamp (0-86399)
typedef struct {
    uint32_t daily_timestamp;  // Daily timestamp in seconds (0-86399)
    uint8_t hour;              // Hour (0-23)
    uint8_t minute;            // Minute (0-59)
    uint8_t second;            // Second (0-59)
} TimePoint_t;

// Time Points Schedule
typedef struct {
    TimePoint_t points[MAX_TIME_POINTS];  // Array of time points
    uint16_t count;                       // Number of valid time points
    uint16_t current_index;               // Current time point index
    uint32_t interval_sec;                // Interval in seconds
    uint32_t start_daily_time;            // Original start time in daily seconds
    _Bool is_configured;                  // True when time points are configured
} TimePointSchedule_t;

// Script Storage Structure
typedef struct {
    bool is_loaded;
    uint32_t binary_size;
    uint8_t binary_data[4096];  // Maximum binary size per script
    Script parsed_script;
} ScriptStorage_t;

// Script Execution Context
typedef struct {
    ScriptExecState_t state;
    uint16_t current_step;
    uint8_t retry_count;
    uint8_t max_retries;
    bool first_run;
} ScriptExecContext_t;

// Script Manager Structure
typedef struct {
    /* Script storage and contexts */
    ScriptStorage_t scripts[SCRIPT_TYPE_COUNT];         /**< Storage for all script types */
    ScriptExecContext_t contexts[SCRIPT_TYPE_COUNT];    /**< Execution contexts for all script types */

    /* Time Point Schedules */
    TimePointSchedule_t dls_schedule;                   /**< DLS routine time points */
    TimePointSchedule_t cam_schedule;                   /**< CAM routine time points */

    /* FreeRTOS synchronization objects */
    SemaphoreHandle_t execution_mutex;                  /**< Mutex for protecting script execution */
    SemaphoreHandle_t dls_semaphore;                    /**< Semaphore for triggering DLS routine */
    SemaphoreHandle_t cam_semaphore;                    /**< Semaphore for triggering CAM routine */

    /* State tracking */
    _Bool init_completed;                               /**< True when INIT script has completed */
    _Bool manager_running;                              /**< True when script manager is running */

    /* System timing configuration */
    _Bool system_time_configured;                       /**< True when system time has been configured */
    uint32_t system_start_daily_time;                   /**< System start daily time in seconds (0-86399) */

    /* Execution statistics */
    uint32_t dls_run_count;                            /**< Number of DLS routine completions */
    uint32_t cam_run_count;                            /**< Number of CAM routine completions */
    uint32_t total_errors;                             /**< Total number of execution errors */
} ScriptManager_t;

// Function prototypes

/* Initialization and Control */
void ScriptManager_Init(void);
bool ScriptManager_LoadScript(ScriptType_t type, const uint8_t* binary_data, uint32_t size);
void ScriptManager_StartExecution(void);
void ScriptManager_StopExecution(void);
void ScriptManager_HandleMODFSPFrame(uint8_t frame_id, const uint8_t* data, uint32_t length);

/* Task functions */
void ScriptManager_Task(void *pvParameters);
void ScriptDLS_Task(void *pvParameters);
void ScriptCAM_Task(void *pvParameters);

/* Status and Information */
ScriptExecState_t ScriptManager_GetScriptState(ScriptType_t type);
uint32_t ScriptManager_GetNextRunTime(ScriptType_t type);
uint32_t ScriptManager_GetTimeUntilNextRun(ScriptType_t type);
void ScriptManager_GetStatistics(uint32_t* dls_runs, uint32_t* cam_runs, uint32_t* errors);
void ScriptManager_PrintStatus(void);

/* MODFSP Frame Handlers */
void ScriptManager_HandleUpdateOBC(const uint8_t* data, uint32_t length);
void ScriptManager_HandleUpdateEXP(const uint8_t* data, uint32_t length);
void ScriptManager_HandleRunExperiment(const uint8_t* data, uint32_t length);
void ScriptManager_HandleSyncTime(const uint8_t* data, uint32_t length);
void ScriptManager_HandleInitFrame(const uint8_t* data, uint32_t length);
void ScriptManager_HandleDLSFrame(const uint8_t* data, uint32_t length);
void ScriptManager_HandleCAMFrame(const uint8_t* data, uint32_t length);
void ScriptManager_HandleHaltFrame(const uint8_t* data, uint32_t length);

/* Time Point Functions */
_Bool ScriptManager_GenerateTimePoints(TimePointSchedule_t* schedule, uint32_t start_daily_time, uint32_t interval_sec);
_Bool ScriptManager_IsTimeToRunSchedule(TimePointSchedule_t* schedule);
void ScriptManager_PrintTimePoints(TimePointSchedule_t* schedule, const char* routine_name);
void ScriptManager_AdvanceSchedule(TimePointSchedule_t* schedule, const char* routine_name);


_Bool ScriptManager_EraseAllScriptsFromFRAM(void);
uint8_t UserActivityDetected(void);
void UserActivityTrigger(void);
uint8_t ExpMonitor_IsEnabled(void);
uint8_t ExpMonitor_SetEnabled(uint8_t enable);
#endif // SCRIPT_MANAGER_H
