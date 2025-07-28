/************************************************
 *  @file     : script_manager.c
 *  @date     : Jul 19, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    Script Manager for handling INIT, DLS, and CAM routines
 *    Uses time point scheduling for precise timing
 ************************************************/

#include "script_manager.h"
#include "parser/bscript_parser.h"
#include "runner/bscript_runner.h"
#include "logger/bscript_logger.h"
#include "port/bscript_port.h"
#include "DateTime/date_time.h"
#include "action_id.h"
#include "utils.h"
#include "modfsp.h"
#include <string.h>

#include "storage/script_storage.h"

extern MODFSP_Data_t cm4_protocol;

#include "MIN_Process/min_process.h"

#include "DateTime/date_time.h"

#include "SimpleDataTransfer/simple_datatrans.h"
#include "main.h"
#include "stdio.h"
#include "log_manager.h"

#include "filesystem.h"
#include "reinit.h"

#include "RAMBK_Infor/rambk_infor.h"
/*************************************************
 *               PRIVATE VARIABLES               *
 *************************************************/

#define SCHED_OFFSET_SEC 		5

#define SAMPLING_TIMEOUT_MS		25000

static volatile uint8_t g_user_activity_detected = 0;
static uint8_t frames_received = 0;
static ScriptManager_t g_script_manager;

static volatile uint8_t expMonitorFlag = 0;
static volatile bool g_log_fetching_enabled = false;


#define EXP_LOG_TRIGGER_GPIO_PORT 	EXPOUT_OBCIN_LOGTRIGGER_GPIO_Port
#define EXP_LOG_TRIGGER_GPIO_PIN	EXPOUT_OBCIN_LOGTRIGGER_Pin

#define EXP_MINBUSY_GPIO_PORT		EXPOUT_OBCIN_MINBUSY_GPIO_Port
#define EXP_MINBUSY_GPIO_PIN		EXPOUT_OBCIN_MINBUSY_Pin

typedef struct {
    uint32_t release_time_sec;
    uint32_t lockin_time_sec;
    bool release_done_today;
    bool lockin_done_today;
    bool configured;
} SDScheduler_t;

extern MODFSP_Data_t cm4_protocol;

static SDScheduler_t g_sd_scheduler = {0};

/*************************************************
 *               PRIVATE FUNCTIONS               *
 *************************************************/
static StepExecResult ScriptManager_ExecuteStep(ScriptType_t script_type, Step* step);
static StepExecResult ScriptManager_ExecuteInitStep(Step* step);
static StepExecResult ScriptManager_ExecuteDLSStep(Step* step);
static StepExecResult ScriptManager_ExecuteCAMStep(Step* step);

static void ScriptManager_ResetContext(ScriptType_t type);
static void ScriptManager_HandleStepResult(ScriptType_t type, StepExecResult result);
static uint32_t ScriptManager_GetCurrentTimeSeconds(void);
static uint32_t ScriptManager_ParseStartTime(uint32_t time_value);
static void ScriptManager_TimeToHMS(uint32_t seconds_in_day, uint8_t* hour, uint8_t* minute, uint8_t* second);

static void ScriptManager_RequestScriptsFromMaster(void);
static _Bool ScriptManager_HandleAutoLoad(void);
/*************************************************
 *           TIME MANAGEMENT FUNCTIONS           *
 *************************************************/

void SDScheduler_Configure(uint32_t release_time_raw, uint32_t lockin_time_raw)
{
    uint8_t release_hh = (release_time_raw >> 16) & 0xFF;
    uint8_t release_mm = (release_time_raw >> 8) & 0xFF;
    uint8_t release_ss = release_time_raw & 0xFF;

    uint8_t lockin_hh = (lockin_time_raw >> 16) & 0xFF;
    uint8_t lockin_mm = (lockin_time_raw >> 8) & 0xFF;
    uint8_t lockin_ss = lockin_time_raw & 0xFF;

    g_sd_scheduler.release_time_sec = release_hh * 3600 + release_mm * 60 + release_ss;
    g_sd_scheduler.lockin_time_sec = lockin_hh * 3600 + lockin_mm * 60 + lockin_ss;

    g_sd_scheduler.release_done_today = false;
    g_sd_scheduler.lockin_done_today = false;
    g_sd_scheduler.configured = true;

    // Logging (optional)
    BScript_Log("[SDScheduler] Configured: Release @ %02u:%02u:%02u (%us), Lock-in @ %02u:%02u:%02u (%us)",
        release_hh, release_mm, release_ss, g_sd_scheduler.release_time_sec,
        lockin_hh, lockin_mm, lockin_ss, g_sd_scheduler.lockin_time_sec);
}


/**
 * @brief Get current time in seconds since start of day (0-86399)
 * @return Current time in seconds from 00:00:00
 */
static uint32_t ScriptManager_GetCurrentDailyTimeSeconds(void)
{
    s_DateTime rtc;
    Utils_GetRTC(&rtc);

    uint32_t daily_seconds = (rtc.hour * 3600) + (rtc.minute * 60) + rtc.second;
    return daily_seconds;
}

/**
 * @brief Get current soft time in seconds since system start (for logging only)
 * @return Current time in seconds
 */
static uint32_t ScriptManager_GetCurrentTimeSeconds(void)
{
    s_DateTime rtc;
    Utils_GetRTC(&rtc);

    // Convert to total seconds (simplified calculation)
    // This provides a monotonic increasing value for logging only
    uint32_t total_seconds = (rtc.year * 365 * 24 * 3600) +
                            (rtc.month * 30 * 24 * 3600) +
                            (rtc.day * 24 * 3600) +
                            (rtc.hour * 3600) +
                            (rtc.minute * 60) +
                            rtc.second;

    return total_seconds;
}

/**
 * @brief Parse time from FF HH MM SS format and convert to daily time
 * @param time_value Time value in FF HH MM SS format
 * @return Daily time in seconds (0-86399)
 */
static uint32_t ScriptManager_ParseStartTime(uint32_t time_value)
{
    if (time_value == 0xFFFFFFFF) {
        // "now" - return current daily time
        uint32_t current_daily_time = ScriptManager_GetCurrentDailyTimeSeconds();
        BScript_Log("[ScriptManager] Start time = now (%u seconds from 00:00:00)", current_daily_time);
        return current_daily_time;
    }

    // Extract FF HH MM SS format (skip FF byte)
    uint8_t hours = (time_value >> 16) & 0xFF;
    uint8_t minutes = (time_value >> 8) & 0xFF;
    uint8_t seconds = time_value & 0xFF;

    // Validate time components
    if (hours > 23 || minutes > 59 || seconds > 59) {
        BScript_Log("[ScriptManager] Invalid time format: %02u:%02u:%02u, using now",
                   hours, minutes, seconds);
        return ScriptManager_GetCurrentDailyTimeSeconds();
    }

    // Convert to daily seconds
    uint32_t daily_seconds = (hours * 3600) + (minutes * 60) + seconds;

    BScript_Log("[ScriptManager] Start time set to %02u:%02u:%02u (%u seconds from 00:00:00)",
               hours, minutes, seconds, daily_seconds);

    return daily_seconds;
}

/**
 * @brief Convert seconds in day to hour, minute, second
 * @param seconds_in_day Seconds from start of day (0-86399)
 * @param hour Output hour (0-23)
 * @param minute Output minute (0-59)
 * @param second Output second (0-59)
 */
static void ScriptManager_TimeToHMS(uint32_t seconds_in_day, uint8_t* hour, uint8_t* minute, uint8_t* second)
{
    *hour = (seconds_in_day / 3600) % 24;
    *minute = (seconds_in_day % 3600) / 60;
    *second = seconds_in_day % 60;
}

/*************************************************
 *           TIME POINT FUNCTIONS                *
 *************************************************/

/**
 * @brief Generate next day time points for a schedule
 * @param schedule Pointer to schedule structure
 * @return true if successful, false otherwise
 */
_Bool ScriptManager_GenerateNextDayTimePoints(TimePointSchedule_t* schedule)
{
    if (!schedule || !schedule->is_configured) {
        return false;
    }

    // Get the start time pattern from first point (if exists)
    uint32_t start_daily_time = 0;
    if (schedule->count > 0) {
        start_daily_time = schedule->points[0].daily_timestamp;
    }

    BScript_Log("[ScriptManager] Generating next day time points starting at daily time: %u", start_daily_time);

    // Generate time points for next day using the same daily start time
    return ScriptManager_GenerateTimePoints(schedule, start_daily_time, schedule->interval_sec);
}

/**
 * @brief Generate time points for a routine based on start time and interval
 * @param schedule Pointer to schedule structure
 * @param start_daily_time Start time in daily seconds (0-86399)
 * @param interval_sec Interval in seconds
 * @return true if successful, false otherwise
 */
_Bool ScriptManager_GenerateTimePoints(TimePointSchedule_t* schedule, uint32_t start_daily_time, uint32_t interval_sec)
{
    if (!schedule || interval_sec < MIN_INTERVAL_SEC || interval_sec > MAX_INTERVAL_SEC) {
        BScript_Log("[ScriptManager] Invalid parameters for time point generation");
        return false;
    }

    if (start_daily_time >= SECONDS_PER_DAY) {
        BScript_Log("[ScriptManager] Invalid start_daily_time: %u (must be 0-86399)", start_daily_time);
        return false;
    }

//    if (start_daily_time >= SCHED_OFFSET_SEC) {
//        start_daily_time -= SCHED_OFFSET_SEC;
//        BScript_Log("[ScriptManager] Adjusted start_daily_time by -%u seconds to compensate for delay", SCHED_OFFSET_SEC);
//    } else {
//        start_daily_time = 0;
//        BScript_Log("[ScriptManager] Start time too close to 00:00:00, adjusted to zero");
//    }


//    // Store interval for future regeneration
//    uint32_t stored_interval = schedule->interval_sec;
//    _Bool was_configured = schedule->is_configured;

    // Clear existing schedule but preserve interval if this is a regeneration
    memset(schedule, 0, sizeof(TimePointSchedule_t));
    schedule->interval_sec = interval_sec;

    // Get current daily time for reference
    uint32_t current_daily_time = ScriptManager_GetCurrentDailyTimeSeconds();

    BScript_Log("[ScriptManager] Generating time points:");
    BScript_Log("  - Start daily time: %u seconds", start_daily_time);
    BScript_Log("  - Interval: %u seconds", interval_sec);
    BScript_Log("  - Current daily time: %u seconds", current_daily_time);

    // Generate time points for the day
    uint32_t current_point = start_daily_time;
    uint16_t point_count = 0;

    while (point_count < MAX_TIME_POINTS) {
        // Check if we can fit another complete interval before end of day
        if (current_point + interval_sec > SECONDS_PER_DAY) {
            BScript_Log("[ScriptManager] Cannot fit next interval (%u + %u > %u), stopping",
                       current_point, interval_sec, SECONDS_PER_DAY);
            break;
        }

        // Create time point
        TimePoint_t* point = &schedule->points[point_count];
        point->daily_timestamp = current_point;
        ScriptManager_TimeToHMS(current_point, &point->hour, &point->minute, &point->second);

        BScript_Log("  - Point %u: %02u:%02u:%02u (daily_timestamp: %u)",
                   point_count, point->hour, point->minute, point->second, point->daily_timestamp);

        point_count++;
        current_point += interval_sec;
    }

    schedule->count = point_count;
    schedule->current_index = 0;
    schedule->is_configured = true;

    BScript_Log("[ScriptManager] Generated %u time points", point_count);

    // Find the current or next time point index
    _Bool found_current = false;
    for (uint16_t i = 0; i < point_count; i++) {
        if (schedule->points[i].daily_timestamp >= current_daily_time) {
            schedule->current_index = i;
            found_current = true;
            BScript_Log("[ScriptManager] Next time point index: %u (daily_timestamp: %u)",
                       i, schedule->points[i].daily_timestamp);
            break;
        }
    }

    // If all points are in the past for today, start from first point (will run tomorrow at 00:00+ cycle)
    if (!found_current) {
        BScript_Log("[ScriptManager] All time points are in the past for today, will start from first point tomorrow");
        schedule->current_index = 0; // Will be triggered tomorrow when daily time resets
    }

    return true;
}

/**
 * @brief Check if it's time to run based on schedule
 * @param schedule Pointer to schedule structure
 * @return true if should run, false otherwise
 */
_Bool ScriptManager_IsTimeToRunSchedule(TimePointSchedule_t* schedule)
{
    if (!schedule || !schedule->is_configured || schedule->count == 0) {
        return false;
    }

    uint32_t current_daily_time = ScriptManager_GetCurrentDailyTimeSeconds();

    // Check if current time point is reached
    if (schedule->current_index < schedule->count) {
        TimePoint_t* current_point = &schedule->points[schedule->current_index];

        if (current_daily_time >= current_point->daily_timestamp) {
            return true;
        }
    }

    // Handle day rollover case: if we're at index 0 and current time matches first point
    if (schedule->current_index == 0 && schedule->count > 0) {
        TimePoint_t* first_point = &schedule->points[0];
        if (current_daily_time >= first_point->daily_timestamp &&
            current_daily_time <= (first_point->daily_timestamp + 60)) { // Within 1 minute window
            return true;
        }
    }

    return false;
}

/**
 * @brief Advance schedule to next time point
 * @param schedule Pointer to schedule structure
 * @param routine_name Name for logging
 */
void ScriptManager_AdvanceSchedule(TimePointSchedule_t* schedule, const char* routine_name)
{
    if (!schedule || !schedule->is_configured || schedule->count == 0) {
        return;
    }

    if (schedule->current_index < schedule->count) {
        TimePoint_t* current_point = &schedule->points[schedule->current_index];

        BScript_Log("[ScriptManager] %s: Time point reached: %02u:%02u:%02u (daily_timestamp: %u, index %u)",
                   routine_name, current_point->hour, current_point->minute, current_point->second,
                   current_point->daily_timestamp, schedule->current_index);

        // Move to next time point
        schedule->current_index++;

        // If we've completed all points for today, reset for tomorrow
        if (schedule->current_index >= schedule->count) {
            BScript_Log("[ScriptManager] %s: All time points completed for today, resetting for tomorrow", routine_name);
            schedule->current_index = 0; // Reset to first point for tomorrow
        }
    }
}

/**
 * @brief Print time points for debugging
 * @param schedule Pointer to schedule structure
 * @param routine_name Name of the routine for logging
 */
void ScriptManager_PrintTimePoints(TimePointSchedule_t* schedule, const char* routine_name)
{
    if (!schedule || !routine_name) return;

    uint32_t current_daily_time = ScriptManager_GetCurrentDailyTimeSeconds();
    s_DateTime current_rtc;
    Utils_GetRTC(&current_rtc);

    BScript_Log("[ScriptManager] %s Time Points:", routine_name);
    BScript_Log("  - Configured: %s", schedule->is_configured ? "YES" : "NO");
    BScript_Log("  - Interval: %u seconds (%u minutes)", schedule->interval_sec, schedule->interval_sec / 60);
    BScript_Log("  - Start daily time: %u seconds", schedule->start_daily_time);
    BScript_Log("  - Total points: %u", schedule->count);
    BScript_Log("  - Current index: %u", schedule->current_index);
    BScript_Log("  - Current system time: 20%02d-%02d-%02d %02d:%02d:%02d (daily: %u)",
               current_rtc.year, current_rtc.month, current_rtc.day,
               current_rtc.hour, current_rtc.minute, current_rtc.second, current_daily_time);

    int16_t start = (int16_t)schedule->current_index - 3;
    int16_t end = (int16_t)schedule->current_index + 3;

    if (start < 0) start = 0;
    if (end >= schedule->count) end = schedule->count - 1;

    for (int16_t i = start; i <= end; i++) {
        TimePoint_t* point = &schedule->points[i];
        const char* status = "";

        if (i == schedule->current_index) {
            status = " <-- NEXT";
        } else if (i < schedule->current_index) {
            status = " (completed)";
        } else if (point->daily_timestamp <= current_daily_time) {
            status = " (past due)";
        }

        BScript_Log("    [%u] %02u:%02u:%02u (daily_timestamp: %u)%s",
                   i, point->hour, point->minute, point->second, point->daily_timestamp, status);
    }


    // Show next run time info
    if (schedule->current_index < schedule->count) {
        uint32_t next_run_daily = schedule->points[schedule->current_index].daily_timestamp;
        uint32_t time_until = 0;

        if (current_daily_time <= next_run_daily) {
            time_until = next_run_daily - current_daily_time;
            BScript_Log("  - Next run in: %u seconds (%u minutes) - today", time_until, time_until / 60);
        } else {
            time_until = SECONDS_PER_DAY - current_daily_time + next_run_daily;
            BScript_Log("  - Next run in: %u seconds (%u minutes) - tomorrow", time_until, time_until / 60);
        }
    } else {
        BScript_Log("  - All points completed for today, will reset at midnight");
    }
}

/*************************************************
 *              CONTEXT MANAGEMENT               *
 *************************************************/

/**
 * @brief Reset script execution context
 * @param type Script type to reset
 */
static void ScriptManager_ResetContext(ScriptType_t type)
{
    if (type >= SCRIPT_TYPE_COUNT) return;

    ScriptExecContext_t* context = &g_script_manager.contexts[type];
    context->state = SCRIPT_EXEC_IDLE;
    context->current_step = 0;
    context->retry_count = 0;
    SimpleDataTransfer_SetFatfsOk(true);
    context->first_run = true;
}

/**
 * @brief Handle step execution result
 * @param type Script type
 * @param result Execution result
 */
static void ScriptManager_HandleStepResult(ScriptType_t type, StepExecResult result)
{
    if (type >= SCRIPT_TYPE_COUNT) return;

    ScriptExecContext_t* context = &g_script_manager.contexts[type];

    switch (result) {
        case STEP_EXEC_SUCCESS:
            context->current_step++;
            context->retry_count = 0;

            // Check if INIT is completed
            if (type == SCRIPT_TYPE_INIT &&
                context->current_step >= g_script_manager.scripts[type].parsed_script.total_steps) {
                g_script_manager.init_completed = true;
                context->state = SCRIPT_EXEC_COMPLETED;
                BScript_Log("[ScriptManager] INIT script completed, enabling routines");
            }
            break;

        case STEP_EXEC_ERROR:
            context->retry_count++;
            if (context->retry_count >= context->max_retries) {
                context->state = SCRIPT_EXEC_FAILED_MAX_RETRIES;
                g_script_manager.total_errors++;
                BScript_Log("[ScriptManager] Script type %d failed after max retries", type);
            } else {
                BScript_Log("[ScriptManager] Script type %d retrying step %d (attempt %d/%d)",
                           type, context->current_step, context->retry_count, context->max_retries);
            }
            break;

        case STEP_EXEC_WAIT:
            context->state = SCRIPT_EXEC_WAITING;
            break;
    }
}

/*************************************************
 *              PUBLIC FUNCTIONS                 *
 *************************************************/

/**
 * @brief Initialize the Script Manager
 */
void ScriptManager_Init(void)
{
    memset(&g_script_manager, 0, sizeof(ScriptManager_t));



    // Create synchronization objects
    g_script_manager.execution_mutex = xSemaphoreCreateMutex();
    g_script_manager.dls_semaphore = xSemaphoreCreateBinary();
    g_script_manager.cam_semaphore = xSemaphoreCreateBinary();

    if (!g_script_manager.execution_mutex ||
        !g_script_manager.dls_semaphore ||
        !g_script_manager.cam_semaphore) {
        BScript_Log("[ScriptManager] Error: Failed to create synchronization objects");
        return;
    }

    // Initialize contexts
    for (int i = 0; i < SCRIPT_TYPE_COUNT; i++) {
        g_script_manager.contexts[i].max_retries = 3;
        ScriptManager_ResetContext((ScriptType_t)i);
    }

    if (!ScriptStorage_Init()) {
        BScript_Log("[ScriptManager] Error: Failed to initialize script storage");
        return;
    }

    BScript_Log("[ScriptManager] Initialized successfully");

    if (SimpleDataTransfer_Init() != E_OK) {
        BScript_Log("[ScriptManager] Failed to initialize Simple Data Transfer");
    } else {
        BScript_Log("[ScriptManager] Simple Data Transfer initialized successfully");
    }

    ScriptManager_HandleAutoLoad();

}

void ScriptManager_EnableLogFetching(bool enable)
{
    g_log_fetching_enabled = enable;
    if (enable) {
        BScript_Log("[ScriptManager] Background log fetching ENABLED");
    } else {
        BScript_Log("[ScriptManager] Background log fetching DISABLED");
    }
}

/**
 * @brief Load a script binary into the manager
 * @param type Script type
 * @param binary_data Binary data
 * @param size Data size
 * @return true if loaded successfully, false otherwise
 */
_Bool ScriptManager_LoadScript(ScriptType_t type, const uint8_t* binary_data, uint32_t size)
{
    if (type >= SCRIPT_TYPE_COUNT || !binary_data || size == 0 ||
        size > sizeof(g_script_manager.scripts[0].binary_data)) {
        BScript_Log("[ScriptManager] Error: Invalid parameters for script loading");
        return false;
    }

    ScriptStorage_t* storage = &g_script_manager.scripts[type];

    // Copy binary data
    memcpy(storage->binary_data, binary_data, size);
    storage->binary_size = size;

    // Parse the script
    ScriptParseResult result = BScript_ParseScript(storage->binary_data, storage->binary_size, &storage->parsed_script);
    if (result != PARSE_OK) {
        BScript_Log("[ScriptManager] Error: Failed to parse script type %d, result: %d", type, result);
        storage->is_loaded = false;
        return false;
    }

    storage->is_loaded = true;
    ScriptManager_ResetContext(type);

    BScript_Log("[ScriptManager] Successfully loaded script type %d with %d steps",
                type, storage->parsed_script.total_steps);

    // Save script to FRAM for persistence
    ScriptStorageResult_t save_result = ScriptStorage_SaveScript(type, binary_data, size);
    if (save_result == STORAGE_SUCCESS) {
        BScript_Log("[ScriptManager] Script type %d saved to FRAM successfully", type);
    } else {
        BScript_Log("[ScriptManager] Warning: Failed to save script type %d to FRAM (result: %d)",
                   type, save_result);
    }

    return true;
}

/**
 * @brief Start script execution
 */
void ScriptManager_StartExecution(void)
{
    if (!g_script_manager.scripts[SCRIPT_TYPE_INIT].is_loaded) {
        BScript_Log("[ScriptManager] Error: INIT script not loaded");
        return;
    }

    g_script_manager.manager_running = true;
    g_script_manager.init_completed = false;

    // Reset all contexts
    for (int i = 0; i < SCRIPT_TYPE_COUNT; i++) {
        ScriptManager_ResetContext((ScriptType_t)i);
    }

    // Start with INIT script
    g_script_manager.contexts[SCRIPT_TYPE_INIT].state = SCRIPT_EXEC_RUNNING;

    BScript_Log("[ScriptManager] Started execution");
}

/**
 * @brief Stop script execution
 */
void ScriptManager_StopExecution(void)
{
    g_script_manager.manager_running = false;

    // Reset all contexts
    for (int i = 0; i < SCRIPT_TYPE_COUNT; i++) {
        ScriptManager_ResetContext((ScriptType_t)i);
    }

    // Clear time point schedules
    memset(&g_script_manager.dls_schedule, 0, sizeof(TimePointSchedule_t));
    memset(&g_script_manager.cam_schedule, 0, sizeof(TimePointSchedule_t));

    BScript_Log("[ScriptManager] Stopped execution");
}


/**
 * @brief Handle auto-loading scripts from FRAM
 * @return true if auto-load was attempted, false otherwise
 */
static _Bool ScriptManager_HandleAutoLoad(void)
{
    BScript_Log("[ScriptManager] Checking for auto-load conditions...");

    // Check if any scripts exist in FRAM
    ScriptStorageStatus_t storage_status;
    ScriptStorage_GetStatus(&storage_status);

    _Bool any_script_exists = false;
    for (int i = 0; i < SCRIPT_TYPE_COUNT; i++) {
        if (storage_status.script_exists[i]) {
            any_script_exists = true;
            break;
        }
    }

    if (!any_script_exists) {
        BScript_Log("[ScriptManager] No scripts found in FRAM, waiting for user input...");
        // Start a task to request scripts from master after timeout
        // TODO: Create a task that waits and then calls ScriptManager_RequestScriptsFromMaster()
        return false;
    }

    BScript_Log("[ScriptManager] Scripts found in FRAM, starting auto-load sequence...");

    // Start auto-load in a separate task to avoid blocking initialization
    // TODO: Create a task that calls ScriptStorage_AutoLoadScripts()
    // For now, we'll do it directly but this should be moved to a task
    uint8_t loaded_count = ScriptStorage_AutoLoadScripts(&g_script_manager);

    if (loaded_count == SCRIPT_TYPE_COUNT) {
        BScript_Log("[ScriptManager] All scripts auto-loaded successfully");
        return true;
    } else if (loaded_count > 0) {
        BScript_Log("[ScriptManager] Partial auto-load completed (%d/%d scripts)",
                   loaded_count, SCRIPT_TYPE_COUNT);
        ScriptManager_RequestScriptsFromMaster();
        return true;
    } else {
        BScript_Log("[ScriptManager] Auto-load failed, requesting scripts from master");
        ScriptManager_RequestScriptsFromMaster();
        return false;
    }
}

/**
 * @brief Request missing scripts from master
 */
static void ScriptManager_RequestScriptsFromMaster(void)
{
    BScript_Log("[ScriptManager] Requesting scripts from master...");

    // Check which scripts are missing
    ScriptStorageStatus_t storage_status;
    ScriptStorage_GetStatus(&storage_status);

    for (ScriptType_t type = SCRIPT_TYPE_INIT; type < SCRIPT_TYPE_COUNT; type++) {
        if (!g_script_manager.scripts[type].is_loaded) {
            const char* type_name = "";
            switch (type) {
                case SCRIPT_TYPE_INIT: type_name = "INIT"; break;
                case SCRIPT_TYPE_DLS_ROUTINE: type_name = "DLS"; break;
                case SCRIPT_TYPE_CAM_ROUTINE: type_name = "CAM"; break;
                default: break;
            }

            BScript_Log("[ScriptManager] Requesting %s script from master", type_name);

            // TODO: Implement actual master request protocol
            // This might involve sending a specific frame via MODFSP or UART
            // For example:
            // MODFSP_SendScriptRequest(type);
            // or
            // UART_SendScriptRequest(type);
        }
    }

    // TODO: Set a timeout for master response
    // If no response within timeout, fall back to waiting for user input
}

void ScriptManager_GetStorageStatus(ScriptStorageStatus_t* status)
{
    ScriptStorage_GetStatus(status);
}

/**
 * @brief Print script storage status
 */
void ScriptManager_PrintStorageStatus(void)
{
    ScriptStorage_PrintStatus();
}

/**
 * @brief Manually trigger script loading from FRAM
 * @return Number of scripts loaded
 */
uint8_t ScriptManager_LoadFromFRAM(void)
{
    BScript_Log("[ScriptManager] Manual FRAM load requested");
    return ScriptStorage_AutoLoadScripts(&g_script_manager);
}

/**
 * @brief Erase all scripts from FRAM
 * @return true if all scripts erased successfully
 */
_Bool ScriptManager_EraseAllScriptsFromFRAM(void)
{
    BScript_Log("[ScriptManager] Erasing all scripts from FRAM...");

    _Bool all_success = true;
    for (ScriptType_t type = SCRIPT_TYPE_INIT; type < SCRIPT_TYPE_COUNT; type++) {
        ScriptStorageResult_t result = ScriptStorage_EraseScript(type);
        if (result != STORAGE_SUCCESS) {
            const char* type_name = "";
            switch (type) {
                case SCRIPT_TYPE_INIT: type_name = "INIT"; break;
                case SCRIPT_TYPE_DLS_ROUTINE: type_name = "DLS"; break;
                case SCRIPT_TYPE_CAM_ROUTINE: type_name = "CAM"; break;
                default: break;
            }
            BScript_Log("[ScriptManager] Failed to erase %s script: %s",
                       type_name, ScriptStorage_GetResultString(result));
            all_success = false;
        }
    }

    if (all_success) {
        BScript_Log("[ScriptManager] All scripts erased from FRAM successfully");
    }

    return all_success;
}

/**
 * @brief Check if script exists in FRAM
 * @param type Script type
 * @return true if script exists in FRAM
 */
_Bool ScriptManager_ScriptExistsInFRAM(ScriptType_t type)
{
    return ScriptStorage_ScriptExists(type);
}

/*************************************************
 *              MODFSP HANDLERS                  *
 *************************************************/

/**
 * @brief MODFSP Application Handler
 * @param ctx MODFSP context
 * @param id Frame ID
 * @param payload Payload data
 * @param len Payload length
 */
void MODFSP_ApplicationHandler(MODFSP_Data_t *ctx, uint8_t id, const uint8_t *payload, uint16_t len)
{
    if (id == MODFSP_MASTER_ACK ||  id == MODFSP_MASTER_NAK) {
        SimpleDataTransfer_HandleMasterAck(id, payload, len);
        return;
    }

    ScriptManager_HandleMODFSPFrame(id, payload, len);
}

/**
 * @brief Handle incoming MODFSP frames
 * @param frame_id Frame ID
 * @param data Frame data
 * @param length Data length
 */

uint8_t UserActivityDetected(void){
	return g_user_activity_detected;
}

void UserActivityTrigger(void){
	g_user_activity_detected = 1;
}


void ScriptManager_HandleMODFSPFrame(uint8_t frame_id, const uint8_t* data, uint32_t length)
{
	g_user_activity_detected = 1;

    switch (frame_id) {
        case FRAME_ID_INIT:
            ScriptManager_HandleInitFrame(data, length);
            break;
        case FRAME_ID_DLS_ROUTINE:
            ScriptManager_HandleDLSFrame(data, length);
            break;
        case FRAME_ID_CAM_ROUTINE:
            ScriptManager_HandleCAMFrame(data, length);
            break;
        case FRAME_ID_HALT:
            ScriptManager_HandleHaltFrame(data, length);
            break;
        case SEND_TIME_CMD:
        	ScriptManager_HandleSyncTime(data, length);
        	break;
        case RUN_EXPERIMENT_CMD:
        	ScriptManager_HandleRunExperiment(data, length);
        	break;

        case UPDATE_OBC_CMD:
        	ScriptManager_HandleUpdateOBC(data, length);
        	break;

        case MODFSP_TYPE_GET_STM32RTC:
        	ScriptManager_HandleCM4GetRTC(data, length);
        	break;

        case UPDATE_EXP_CMD:
        	ScriptManager_HandleUpdateEXP(data, length);
        	break;

        case 0x98:
        	BScript_Log("[ScriptManager] Response from CM4 OK!");
        	break;


        default:
            BScript_Log("[ScriptManager] Warning: Unknown frame ID: 0x%02X", frame_id);
            break;
    }
}

/**
 * @brief Handle INIT frame
 */
void ScriptManager_HandleInitFrame(const uint8_t* data, uint32_t length)
{
	MODFSP_Send(&cm4_protocol, SCRIPT_ACK_INIT, NULL, 0);
    BScript_Log("[ScriptManager] Received INIT frame, size: %u", length);
    if (ScriptManager_LoadScript(SCRIPT_TYPE_INIT, data, length)) {
        frames_received |= 0x01;  // Set bit 0
        BScript_Log("[ScriptManager] INIT script loaded successfully");

        // Check if all required frames received
        if (frames_received == 0x0F) {  // All 4 bits set (0x01 | 0x02 | 0x04 | 0x08)
            ScriptManager_StartExecution();
            frames_received = 0;
        }
    }
}

/**
 * @brief Handle DLS frame
 */
void ScriptManager_HandleDLSFrame(const uint8_t* data, uint32_t length)
{
	MODFSP_Send(&cm4_protocol, SCRIPT_ACK_DLS, NULL, 0);
    BScript_Log("[ScriptManager] Received DLS frame, size: %u", length);
    if (ScriptManager_LoadScript(SCRIPT_TYPE_DLS_ROUTINE, data, length)) {
        frames_received |= 0x02;  // Set bit 1
        BScript_Log("[ScriptManager] DLS script loaded successfully");

        if (frames_received == 0x0F) {  // All 4 bits set (0x01 | 0x02 | 0x04 | 0x08)
            ScriptManager_StartExecution();
            frames_received = 0;
        }
    }
}

/**
 * @brief Handle CAM frame
 */
void ScriptManager_HandleCAMFrame(const uint8_t* data, uint32_t length)
{
	MODFSP_Send(&cm4_protocol, SCRIPT_ACK_CAM, NULL, 0);
    BScript_Log("[ScriptManager] Received CAM frame, size: %u", length);
    if (ScriptManager_LoadScript(SCRIPT_TYPE_CAM_ROUTINE, data, length)) {
        frames_received |= 0x04;  // Set bit 2
        BScript_Log("[ScriptManager] CAM script loaded successfully");

        if (frames_received == 0x0F) {  // All 4 bits set (0x01 | 0x02 | 0x04 | 0x08)
            ScriptManager_StartExecution();
            frames_received = 0;
        }
    }
}

/**
 * @brief Handle HALT frame
 */
void ScriptManager_HandleHaltFrame(const uint8_t* data, uint32_t length)
{
	MODFSP_Send(&cm4_protocol, FRAME_HALT_ACK, NULL, 0);
    BScript_Log("[ScriptManager] Received HALT frame");
    frames_received = 0;
    ScriptManager_StopExecution();
}

void ScriptManager_HandleSyncTime(const uint8_t* data, uint32_t length)
{
	MODFSP_Send(&cm4_protocol, SEND_TIME_ACK, NULL, 0);
	s_DateTime dt;
    dt.hour   = data[0];
    dt.minute = data[1];
    dt.second = data[2];
    dt.day    = data[3];
    dt.month  = data[4];
    dt.year   = data[5];
	Utils_SetRTC(&dt);
	RV3129_HandleTypeDef *hrtc = RV3129_GetHandle();
	RV3129_SetTime(hrtc, &dt);
	BScript_Log("[ScriptManager] RTC set to %02d:%02d:%02d, %02d/%02d/20%02d",
	                             dt.hour, dt.minute, dt.second, dt.day, dt.month, dt.year);

}

void ScriptManager_HandleRunExperiment(const uint8_t* data, uint32_t length)
{
	MODFSP_Send(&cm4_protocol, RUN_EXPERIMENT_ACK, NULL, 0);
    BScript_Log("[ScriptManager] Received RUN frame");
    frames_received |= 0x08;  // Set bit 4
    if (frames_received == 0x0F) {  // All 4 bits set (0x01 | 0x02 | 0x04 | 0x08)
        ScriptManager_StartExecution();
        frames_received = 0;
    }
}

void ScriptManager_HandleCM4GetRTC (const uint8_t* data, uint32_t length)
{
	s_DateTime dt;
	Utils_GetRTC(&dt);

	uint8_t payload[6];
	payload[0] = dt.hour;
	payload[1] = dt.minute;
	payload[2] = dt.second;
	payload[3] = dt.day;
	payload[4] = dt.month;
	payload[5] = dt.year;

	MODFSP_Send(&cm4_protocol, MODFSP_TYPE_RESP_STM32RTC, payload,
			sizeof(payload));

	BScript_Log(
			"[ScriptManager] Sent RTC: %02d:%02d:%02d, %02d/%02d/20%02d to CM4",
			dt.hour, dt.minute, dt.second, dt.day, dt.month, dt.year);
}

void ScriptManager_HandleUpdateOBC(const uint8_t* data, uint32_t length)
{
	MODFSP_Send(&cm4_protocol, UPDATE_OBC_ACK, NULL, 0);
    BScript_Log("[ScriptManager] Received UPDATE_OBC frame");
    vTaskDelay(200);
    SystemOnBootloader_Reset();
}

void ScriptManager_HandleUpdateEXP(const uint8_t* data, uint32_t length)
{
//	MODFSP_Send(&cm4_protocol, UPDATE_EXP_ACK, NULL, 0);
    BScript_Log("[ScriptManager] Received UPDATE_EXP frame");
    ExpMonitor_SetEnabled(1);
}

uint8_t ExpMonitor_SetEnabled(uint8_t enable) {
    expMonitorFlag = (enable != 0) ? 1 : 0;
    return expMonitorFlag;
}

uint8_t ExpMonitor_IsEnabled(void) {
    return expMonitorFlag;
}


/*************************************************
 *                 TASK FUNCTIONS                *
 *************************************************/

/**
 * @brief Main Script Manager Task
 * @param pvParameters Task parameters
 */
void ScriptManager_Task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        if (!g_script_manager.manager_running) {
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
            continue;
        }

        // Take execution mutex
        if (xSemaphoreTake(g_script_manager.execution_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {

            _Bool should_run_dls = false;
            _Bool should_run_cam = false;

            // Handle INIT script first
            if (!g_script_manager.init_completed &&
                g_script_manager.contexts[SCRIPT_TYPE_INIT].state == SCRIPT_EXEC_RUNNING) {

                ScriptStorage_t* storage = &g_script_manager.scripts[SCRIPT_TYPE_INIT];
                ScriptExecContext_t* context = &g_script_manager.contexts[SCRIPT_TYPE_INIT];

                if (storage->is_loaded && context->current_step < storage->parsed_script.total_steps) {
                    Step* step = &storage->parsed_script.steps[context->current_step];
                    StepExecResult result = ScriptManager_ExecuteStep(SCRIPT_TYPE_INIT, step);
                    ScriptManager_HandleStepResult(SCRIPT_TYPE_INIT, result);
                }
            }

            // Check if it's time to run routines (without modifying schedules yet)
            _Bool dls_time_reached = (g_script_manager.init_completed &&
                g_script_manager.scripts[SCRIPT_TYPE_DLS_ROUTINE].is_loaded &&
                ScriptManager_IsTimeToRunSchedule(&g_script_manager.dls_schedule));

            _Bool cam_time_reached = (g_script_manager.init_completed &&
                g_script_manager.scripts[SCRIPT_TYPE_CAM_ROUTINE].is_loaded &&
                ScriptManager_IsTimeToRunSchedule(&g_script_manager.cam_schedule));

            // If any routine should run, advance schedules and set flags
            if (dls_time_reached || cam_time_reached) {
                if (dls_time_reached) {
                    ScriptManager_AdvanceSchedule(&g_script_manager.dls_schedule, "DLS");
                    should_run_dls = true;
                }

                if (cam_time_reached) {
                    ScriptManager_AdvanceSchedule(&g_script_manager.cam_schedule, "CAM");
                    should_run_cam = true;
                }
            }

            // Release mutex BEFORE signaling semaphores to avoid deadlock
            xSemaphoreGive(g_script_manager.execution_mutex);

            // Signal routines to run (now that mutex is released)
            if (should_run_dls) {
                BScript_Log("[ScriptManager] Triggering DLS routine");
                xSemaphoreGive(g_script_manager.dls_semaphore);
            }

            if (should_run_cam) {
                BScript_Log("[ScriptManager] Triggering CAM routine");
                xSemaphoreGive(g_script_manager.cam_semaphore);
            }

        }
//        else {
//            // Could not get mutex, continue to next iteration
//            BScript_Log("[ScriptManager] Could not acquire execution mutex, skipping iteration");
//        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}

/**
 * @brief DLS Routine Task
 * @param pvParameters Task parameters
 */
void ScriptDLS_Task(void *pvParameters)
{
    while (1) {
        // Wait for semaphore signal
        if (xSemaphoreTake(g_script_manager.dls_semaphore, portMAX_DELAY) == pdTRUE) {

            // Take execution mutex
            if (xSemaphoreTake(g_script_manager.execution_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {

                ScriptStorage_t* storage = &g_script_manager.scripts[SCRIPT_TYPE_DLS_ROUTINE];
                ScriptExecContext_t* context = &g_script_manager.contexts[SCRIPT_TYPE_DLS_ROUTINE];

                BScript_Log("[ScriptDLS] Starting DLS routine execution");

                context->state = SCRIPT_EXEC_RUNNING;
                context->current_step = 0;
                context->retry_count = 0;
                SimpleDataTransfer_SetFatfsOk(true);

                // Execute all steps in the DLS routine
                while (context->current_step < storage->parsed_script.total_steps &&
                       context->state == SCRIPT_EXEC_RUNNING) {

                    Step* step = &storage->parsed_script.steps[context->current_step];
                    StepExecResult result = ScriptManager_ExecuteStep(SCRIPT_TYPE_DLS_ROUTINE, step);

                    if (result == STEP_EXEC_SUCCESS) {
                        context->current_step++;
                        context->retry_count = 0;
                    } else if (result == STEP_EXEC_ERROR) {
                        context->retry_count++;
                        if (context->retry_count >= context->max_retries) {
//                            context->state = SCRIPT_EXEC_FAILED_MAX_RETRIES;

                            g_script_manager.total_errors++;
                            BScript_Log("[ScriptDLS] Max retries reached for step %d — skipping this run", context->current_step);
                            ScriptManager_ResetContext(SCRIPT_TYPE_DLS_ROUTINE);


                            break;
                        } else {
                            BScript_Log("[ScriptDLS] Retrying step %d (attempt %d/%d)",
                                       context->current_step, context->retry_count, context->max_retries);
                        }
                    } else if (result == STEP_EXEC_WAIT) {
                        // Step needs to wait, will be retried next time
                        break;
                    }
                }

                if (context->current_step >= storage->parsed_script.total_steps) {
                    context->state = SCRIPT_EXEC_COMPLETED;
                    g_script_manager.dls_run_count++;
                    BScript_Log("[ScriptDLS] Routine completed successfully (total runs: %u)",
                               g_script_manager.dls_run_count);
                }

                xSemaphoreGive(g_script_manager.execution_mutex);
            }
        }
    }
}

/**
 * @brief CAM Routine Task
 * @param pvParameters Task parameters
 */
void ScriptCAM_Task(void *pvParameters)
{
    while (1) {
        // Wait for semaphore signal
        if (xSemaphoreTake(g_script_manager.cam_semaphore, portMAX_DELAY) == pdTRUE) {

            // Take execution mutex
            if (xSemaphoreTake(g_script_manager.execution_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {

                ScriptStorage_t* storage = &g_script_manager.scripts[SCRIPT_TYPE_CAM_ROUTINE];
                ScriptExecContext_t* context = &g_script_manager.contexts[SCRIPT_TYPE_CAM_ROUTINE];

                BScript_Log("[ScriptCAM] Starting CAM routine execution");

                context->state = SCRIPT_EXEC_RUNNING;
                context->current_step = 0;
                context->retry_count = 0;

                // Execute all steps in the CAM routine
                while (context->current_step < storage->parsed_script.total_steps &&
                       context->state == SCRIPT_EXEC_RUNNING) {

                    Step* step = &storage->parsed_script.steps[context->current_step];
                    StepExecResult result = ScriptManager_ExecuteStep(SCRIPT_TYPE_CAM_ROUTINE, step);

                    if (result == STEP_EXEC_SUCCESS) {
                        context->current_step++;
                        context->retry_count = 0;
                    } else if (result == STEP_EXEC_ERROR) {
                        context->retry_count++;
                        if (context->retry_count >= context->max_retries) {
//                            context->state = SCRIPT_EXEC_FAILED_MAX_RETRIES;

                            g_script_manager.total_errors++;
                            BScript_Log("[ScriptCAM] Max retries reached for step %d — skipping this run", context->current_step);
                            ScriptManager_ResetContext(SCRIPT_TYPE_CAM_ROUTINE);

                            break;
                        } else {
                            BScript_Log("[ScriptCAM] Retrying step %d (attempt %d/%d)",
                                       context->current_step, context->retry_count, context->max_retries);
                        }
                    } else if (result == STEP_EXEC_WAIT) {
                        // Step needs to wait, will be retried next time
                        break;
                    }
                }

                if (context->current_step >= storage->parsed_script.total_steps) {
                    context->state = SCRIPT_EXEC_COMPLETED;
                    g_script_manager.cam_run_count++;
                    BScript_Log("[ScriptCAM] Routine completed successfully (total runs: %u)",
                               g_script_manager.cam_run_count);
                }

                xSemaphoreGive(g_script_manager.execution_mutex);
            }
        }
    }
}

/**
 * @brief Background task to fetch logs when the system is idle.
 * This task attempts to acquire the execution mutex. If successful, it means
 * neither the DLS nor CAM tasks are running, so it's safe to use shared
 * peripherals like SPI and UART.
 * @param pvParameters Task parameters (not used).
 */
void LogFetching_Task(void *pvParameters)
{
	bool date_file_created = false;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(3000));

        if (!g_log_fetching_enabled) {
            continue;
        }

        if (xSemaphoreTake(g_script_manager.execution_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {

            // Check if cleanup is needed (only if date file exists)
            if (NeedCleanup()) {
                BScript_Log("[LogFetching] Log retention period exceeded. Performing cleanup...");

                if (PerformCleanup() == E_OK) {
                    date_file_created = false; // Need to recreate date file
                    BScript_Log("[LogFetching] Cleanup completed successfully");
                } else {
                    BScript_Log("[LogFetching] Cleanup failed, continuing with current files");
                }
            }

            // Create date_time.txt if not created yet (first run or after cleanup)
            if (!date_file_created) {
                if (CreateOrUpdateDateFile() == E_OK) {
                    date_file_created = true;
                    BScript_Log("[LogFetching] Date tracking file created");
                } else {
                    BScript_Log("[LogFetching] Failed to create date file");
                }
            }

            if (!LL_GPIO_IsInputPinSet(EXP_LOG_TRIGGER_GPIO_PORT, EXP_LOG_TRIGGER_GPIO_PIN)) {
                vTaskDelay(pdMS_TO_TICKS(50));
                if (!LL_GPIO_IsInputPinSet(EXP_LOG_TRIGGER_GPIO_PORT, EXP_LOG_TRIGGER_GPIO_PIN)) {
                    BScript_Log("[LogFetching] Stable EXP trigger detected. Initiating special transfer.");

                    char base_filename[48];
                    s_DateTime rtc;
                    Utils_GetRTC(&rtc);

                    snprintf(base_filename, sizeof(base_filename), "exp_log_20%02d%02d%02d_%02d%02d%02d",
                             rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);

                    SimpleDataTransfer_ExecuteTransfer(DATA_TYPE_LOG, 0, base_filename, rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);

                    xSemaphoreGive(g_script_manager.execution_mutex);
                    continue;
                }
            }

            BScript_Log("[LogFetching] No stable EXP trigger, processing buffered logs...");
            LogManager_Process();

            xSemaphoreGive(g_script_manager.execution_mutex);
        }
    }
}

void SDLockRelease_Task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(30000)); // 30s interval

        if (!g_sd_scheduler.configured)
            continue;

        uint32_t now_sec = ScriptManager_GetCurrentDailyTimeSeconds();

        // Reset flags at midnight
        if (now_sec < 60) {
            g_sd_scheduler.release_done_today = false;
            g_sd_scheduler.lockin_done_today = false;
        }

        // Auto-release SD
        if (!g_sd_scheduler.release_done_today &&
            now_sec >= g_sd_scheduler.release_time_sec &&
            now_sec < g_sd_scheduler.release_time_sec + 60) {

        	BScript_Log("[SDScheduler] SD Release");

        	SDMMC1_DeInit();
            SD_Release();

            g_sd_scheduler.release_done_today = true;
        }

        // Auto-lockin SD
        if (!g_sd_scheduler.lockin_done_today &&
            now_sec >= g_sd_scheduler.lockin_time_sec &&
            now_sec < g_sd_scheduler.lockin_time_sec + 60) {

        	BScript_Log("[SDScheduler] SD Lock-in");

        	SD_Lockin();
        	SDMMC1_Init();

        	Std_ReturnType ret = Link_SDFS_Driver();
        	if(ret != E_OK){
        		BScript_Log("[SDScheduler] [Link FATFS Fail]");
        	}else{
        		BScript_Log("[SDScheduler] [Link FATFS Successfully]");
        	}

            g_sd_scheduler.lockin_done_today = true;
        }
    }
}

/*************************************************
 *             STEP EXECUTION FUNCTIONS          *
 *************************************************/

/**
 * @brief Execute a step based on script type
 * @param script_type Script type
 * @param step Step to execute
 * @return Execution result
 */
static StepExecResult ScriptManager_ExecuteStep(ScriptType_t script_type, Step* step)
{
    switch (script_type) {
        case SCRIPT_TYPE_INIT:
            return ScriptManager_ExecuteInitStep(step);
        case SCRIPT_TYPE_DLS_ROUTINE:
            return ScriptManager_ExecuteDLSStep(step);
        case SCRIPT_TYPE_CAM_ROUTINE:
            return ScriptManager_ExecuteCAMStep(step);
        default:
            return STEP_EXEC_ERROR;
    }
}

/**
 * @brief Execute INIT script steps
 * @param step Step to execute
 * @return Execution result
 */
static StepExecResult ScriptManager_ExecuteInitStep(Step* step)
{
    BScript_Log("[ScriptInit] Executing step %u: action_id = 0x%02X", step->step_id, step->action_id);

    switch (step->action_id) {
        case CLEAR_PROFILE: {
            BScript_Log("[ScriptInit] ->CLEAR_PROFILE");
            // TODO: Implement profile clearing
            break;
        }

        case SET_SYSTEM: { // set_system with updated parameters
            uint32_t start, release_time, lockin_time, dls_interval, cam_interval;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &start) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 1, &release_time) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 2, &lockin_time) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 3, &dls_interval) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 4, &cam_interval) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }

            SDScheduler_Configure(release_time, lockin_time);

            // Parse start time and set as system reference (for logging only)
            uint32_t start_daily_time = ScriptManager_ParseStartTime(start);
            g_script_manager.system_start_daily_time = start_daily_time; // Now stores daily time
            g_script_manager.system_time_configured = true;

            // Parse release and lockin times (for logging only, not used in this application)
            uint8_t release_time_hh = (uint8_t)((release_time >> 16) & 0xFF);
            uint8_t release_time_mm = (uint8_t)((release_time >> 8) & 0xFF);
            uint8_t release_time_ss = (uint8_t)(release_time & 0xFF);

            uint8_t lockin_time_hh = (uint8_t)((lockin_time >> 16) & 0xFF);
            uint8_t lockin_time_mm = (uint8_t)((lockin_time >> 8) & 0xFF);
            uint8_t lockin_time_ss = (uint8_t)(lockin_time & 0xFF);


//            uint8_t config_time_data[8];
//
//            config_time_data[0] = (uint8_t)(release_time & 0xFF);
//            config_time_data[1] = (uint8_t)((release_time >> 8) & 0xFF);
//            config_time_data[2] = (uint8_t)((release_time >> 16) & 0xFF);
//            config_time_data[3] = (uint8_t)((release_time >> 24) & 0xFF);
//
//            config_time_data[4] = (uint8_t)(lockin_time & 0xFF);
//            config_time_data[5] = (uint8_t)((lockin_time >> 8) & 0xFF);
//            config_time_data[6] = (uint8_t)((lockin_time >> 16) & 0xFF);
//            config_time_data[7] = (uint8_t)((lockin_time >> 24) & 0xFF);


            BScript_Log("[ScriptInit] ->SET_SYSTEM: System start daily time = %u seconds", start_daily_time);
            BScript_Log("[ScriptInit] ->SET_SYSTEM: Release Time = %02u:%02u:%02u (info only)",
                       release_time_hh, release_time_mm, release_time_ss);
            BScript_Log("[ScriptInit] ->SET_SYSTEM: Lock-in Time = %02u:%02u:%02u (info only)",
                       lockin_time_hh, lockin_time_mm, lockin_time_ss);
            BScript_Log("[ScriptInit] ->SET_SYSTEM: DLS Interval = %u seconds", dls_interval);
            BScript_Log("[ScriptInit] ->SET_SYSTEM: CAM Interval = %u seconds", cam_interval);

            // Generate time points for DLS routine
            if (ScriptManager_GenerateTimePoints(&g_script_manager.dls_schedule, start_daily_time, dls_interval)) {
                BScript_Log("[ScriptInit] ->SET_SYSTEM: DLS time points generated successfully");
                ScriptManager_PrintTimePoints(&g_script_manager.dls_schedule, "DLS");
            } else {
                BScript_Log("[ScriptInit] ->SET_SYSTEM: Failed to generate DLS time points");
                return STEP_EXEC_ERROR;
            }

            // Generate time points for CAM routine
            if (ScriptManager_GenerateTimePoints(&g_script_manager.cam_schedule, start_daily_time + SCHED_OFFSET_SEC, cam_interval)) {
                BScript_Log("[ScriptInit] ->SET_SYSTEM: CAM time points generated successfully");
                ScriptManager_PrintTimePoints(&g_script_manager.cam_schedule, "CAM");
            } else {
                BScript_Log("[ScriptInit] ->SET_SYSTEM: Failed to generate CAM time points");
                return STEP_EXEC_ERROR;
            }

            break;
        }

        case SET_RTC: { // set_rtc
            uint8_t source;
            uint32_t interval;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &source) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 1, &interval) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }

            if(source == 0){
            	BScript_Log("[ScriptInit] ->SET_RTC: Source type: %u - OBC-RTC", source);
            }else if (source == 1){
            	BScript_Log("[ScriptInit] ->SET_RTC: Source type: %u - Nanode-NTP", source);
            }else{
            	BScript_Log("[ScriptInit] ->SET_RTC: Unknown source type - %u", source);
            }

            BScript_Log("[ScriptInit] ->SET_RTC: Set RTC - source=%u, interval=%u", source, interval);
            break;
        }

        case SET_NTC_CONTROL: { // set_ntc_control
        	uint8_t ntc_control_byte = 0;
            uint8_t resp_info[1];
            uint8_t resp_len = 0;

            for (uint8_t i = 0; i < 8; ++i) {
                uint8_t enable_flag;
                if (BScript_ParseParamByIndex(step->parameters, step->param_len, i, &enable_flag) != PARSE_IDX_OK) {
                    BScript_Log("[ScriptInit] ->SET_NTC_CONTROL: Failed to parse enable_index_%u", i);
                    return STEP_EXEC_ERROR;
                }

                if (enable_flag > 1) {
                    BScript_Log("[ScriptInit] ->SET_NTC_CONTROL: Invalid value at enable_index_%u: %u (must be 0 or 1)", i, enable_flag);
                    return STEP_EXEC_ERROR;
                }

                BScript_Log("[ScriptInit] ->SET_NTC_CONTROL: NTC[%u] %s", i, enable_flag ? "ENABLED" : "DISABLED");

                ntc_control_byte |= (enable_flag << i);

            }

            if(MIN_Send_SET_NTC_CONTROL_CMD_WithData(ntc_control_byte, resp_info, &resp_len)){
            	BScript_Log("[ScriptInit] SET_NTC_CONTROL received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptInit] Failed to SET_NTC_CONTROL");
                return STEP_EXEC_ERROR;
            }

            break;
        }

        case SET_TEMP_PROFILE: {
            uint16_t target_temp, min_temp, max_temp, tec_vol;
			uint8_t heater_duty, auto_recover, ntc_primary, ntc_secondary, tec_mask, heater_mask;

            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &target_temp) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 1, &min_temp) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 2, &max_temp) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 3, &ntc_primary) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 4, &ntc_secondary) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 5, &tec_mask) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 6, &heater_mask) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 7, &tec_vol) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 8, &heater_duty) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 9, &auto_recover) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }

            BScript_Log("[ScriptInit] ->SET_TEMP_PROFILE: Target=%u, Min=%u, Max=%u", target_temp, min_temp, max_temp);
            BScript_Log("[ScriptInit] Primary NTC=%u, Secondary NTC=%u", ntc_primary, ntc_secondary);
            BScript_Log("[ScriptInit] TEC mask=0x%02X, Heater mask=0x%02X", tec_mask, heater_mask);
            BScript_Log("[ScriptInit] TEC voltage=%u mV, Heater duty=%u%%, Auto-recover=%u", tec_vol, heater_duty, auto_recover);

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_SET_TEMP_PROFILE_CMD_WithData(target_temp, min_temp, max_temp, ntc_primary, ntc_secondary,
            											auto_recover, tec_mask, heater_mask, tec_vol, heater_duty,  resp_info, &resp_len)){
            	BScript_Log("[ScriptInit] SET_TEMP_PROFILE received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptInit] Failed to SET_TEMP_PROFILE");
                return STEP_EXEC_ERROR;
            }

            if(resp_len == 2 && resp_info[0] != 0x3F ){
            	resp_len = 0;
            	BScript_Log("[ScriptInit] Response SET_TEMP_PROFILE not OK, retry!");
                if(MIN_Send_SET_TEMP_PROFILE_CMD_WithData(target_temp, min_temp, max_temp, ntc_primary, ntc_secondary,
                											auto_recover, tec_mask, heater_mask, tec_vol, heater_duty,  resp_info, &resp_len)){
                	BScript_Log("[ScriptInit] SET_TEMP_PROFILE received: %u bytes", resp_len);
                }else {
                    BScript_Log("[ScriptInit] Failed to SET_TEMP_PROFILE");
                    return STEP_EXEC_ERROR;
                }
            }

            break;
        }

        case START_TEMP_PROFILE: {
            BScript_Log("[ScriptInit] ->START_TEMP_PROFILE");

            uint8_t resp_info[1];
            uint8_t resp_len = 0;

            if(MIN_Send_START_TEMP_PROFILE_CMD_WithData(resp_info, &resp_len)){
            	BScript_Log("[ScriptInit] START_TEMP_PROFILE received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptInit] Failed to START_TEMP_PROFILE");
                return STEP_EXEC_ERROR;
            }

            break;
        }

        case SET_OVERRIDE_TEC_PROFILE: {
            uint16_t interval, tec_vol;
            uint8_t tec_index;

            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &interval) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 1, &tec_index) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 2, &tec_vol) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }

            BScript_Log("[ScriptInit] ->SET_OVERRIDE_TEC_PROFILE: interval=%u sec, index=%u, voltage=%u mV",
                        interval, tec_index, tec_vol);

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_SET_OVERRIDE_TEC_PROFILE_CMD_WithData(interval, tec_index, tec_vol, resp_info, &resp_len)){
            	BScript_Log("[ScriptInit] SET_OVERRIDE_TEC_PROFILE received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptInit] Failed to SET_OVERRIDE_TEC_PROFILE");
                return STEP_EXEC_ERROR;
            }

            if(resp_len == 2 && resp_info[0] != 0x3F ){
            	resp_len = 0;
            	BScript_Log("[ScriptInit] Response SET_OVERRIDE_TEC_PROFILE_CMD not OK, retry!");
                if(MIN_Send_SET_OVERRIDE_TEC_PROFILE_CMD_WithData(interval, tec_index, tec_vol, resp_info, &resp_len)){
                	BScript_Log("[ScriptInit] SET_OVERRIDE_TEC_PROFILE received: %u bytes", resp_len);
                }else {
                    BScript_Log("[ScriptInit] Failed to SET_OVERRIDE_TEC_PROFILE");
                    return STEP_EXEC_ERROR;
                }
            }

            break;
        }

        case START_OVERRIDE_TEC_PROFILE: {
            BScript_Log("[ScriptInit] ->START_OVERRIDE_TEC_PROFILE");

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_START_OVERRIDE_TEC_PROFILE_CMD_WithData( resp_info, &resp_len)){
            	BScript_Log("[ScriptInit] START_OVERRIDE_TEC_PROFILE received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptInit] Failed to START_OVERRIDE_TEC_PROFILE");
                return STEP_EXEC_ERROR;
            }

            break;
        }

        case SET_PDA_PROFILE: {
            uint32_t rate;
			uint16_t pre_laser, in_sample, pos_laser;

            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &rate) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 1, &pre_laser) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 2, &in_sample) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 3, &pos_laser) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }

            BScript_Log("[ScriptInit] ->SET_PDA_PROFILE: Rate=%u Hz, PreLaser=%u us, InSample=%u us, PostLaser=%u us",
                        rate, pre_laser, in_sample, pos_laser);

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_SET_PDA_PROFILE_CMD_WithData(rate, pre_laser, in_sample, pos_laser, resp_info, &resp_len)){
            	BScript_Log("[ScriptInit] SET_PDA_PROFILE received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptInit] Failed to SET_PDA_PROFILE");
                return STEP_EXEC_ERROR;
            }

            if(resp_len == 2 && resp_info[0] != 0x3F ){
            	resp_len = 0;
            	BScript_Log("[ScriptInit] Response SET_PDA_PROFILE_CMD not OK, retry!");
                if(MIN_Send_SET_PDA_PROFILE_CMD_WithData(rate, pre_laser, in_sample, pos_laser, resp_info, &resp_len)){
                	BScript_Log("[ScriptInit] SET_PDA_PROFILE received: %u bytes", resp_len);
                }else {
                    BScript_Log("[ScriptInit] Failed to SET_PDA_PROFILE");
                    return STEP_EXEC_ERROR;
                }
            }

            break;
        }

        case SET_CAMERA_PROFILE: {
            uint8_t resolution, compress_enable;
            uint16_t exposure, gain;

            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &resolution) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 1, &compress_enable) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 2, &exposure) != PARSE_IDX_OK ||
                BScript_ParseParamByIndex(step->parameters, step->param_len, 3, &gain) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }

            const char* resolution_str = "Unknown";
            switch (resolution) {
                case 0: resolution_str = "Low"; break;
                case 1: resolution_str = "Half"; break;
                case 2: resolution_str = "Full"; break;
            }

            BScript_Log("[ScriptInit] ->SET_CAMERA_PROFILE: Res=%s, Compress=%u, Exposure=%u, Gain=%u",
                        resolution_str, compress_enable, exposure, gain);
            // TODO: Apply camera settings
            break;
        }

        default:
            BScript_Log("[ScriptInit] Unknown action: 0x%02X", step->action_id);
            return STEP_EXEC_ERROR;
    }

    return STEP_EXEC_SUCCESS;
}

/**
 * @brief Execute DLS routine steps
 * @param step Step to execute
 * @return Execution result
 */
static StepExecResult ScriptManager_ExecuteDLSStep(Step* step)
{
    BScript_Log("[ScriptDLS] Executing step %u: action_id = 0x%02X", step->step_id, step->action_id);

    switch (step->action_id) {
        case SET_DLS_INTERVAL: { // set_dls_interval (now only for logging)
            uint32_t interval;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &interval) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }

            BScript_Log("[ScriptDLS] ->SET_DLS_INTERVAL: Reached step with interval %u seconds (using time points instead)", interval);
            BScript_Log("[ScriptDLS] Now, OBC shall set EXP Working RTC");

            uint8_t resp_info[5];
            uint8_t resp_len = 0;

            if(MIN_Send_SET_WORKING_RTC_CMD_WithData(resp_info, &resp_len)){
            	BScript_Log("[ScriptDLS] SET_WORKING_RTC received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptDLS] Failed to SET_WORKING_RTC");
                return STEP_EXEC_ERROR;
            }

            break;
        }

        case SET_LASER_INTENSITY: { // set_laser_intensity
            uint8_t intensity;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &intensity) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }
            BScript_Log("[ScriptDLS] ->SET_LASER_INTENSITY: Set laser intensity %u", intensity);


            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_SET_LASER_INTENSITY_CMD_WithData(intensity, resp_info, &resp_len)){
            	BScript_Log("[ScriptDLS] SET_LASER_INTENSITY received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptDLS] Failed to SET_LASER_INTENSITY");
                return STEP_EXEC_ERROR;
            }

            if(resp_len == 2 && resp_info[0] != 0x3F ){
            	resp_len = 0;
            	BScript_Log("[ScriptDLS] Response SET_LASER_INTENSITY not OK, retry!");
                if(MIN_Send_SET_LASER_INTENSITY_CMD_WithData(intensity, resp_info, &resp_len)){
                	BScript_Log("[ScriptDLS] SET_LASER_INTENSITY received: %u bytes", resp_len);
                }else {
                    BScript_Log("[ScriptDLS] Failed to SET_LASER_INTENSITY");
                    return STEP_EXEC_ERROR;
                }
            }

            break;
        }

        case SET_POSITION: { // set_position
            uint8_t position;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &position) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }
            BScript_Log("[ScriptDLS] ->SET_POSITION: Set position: %u", position);

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_SET_POSITION_CMD_WithData(position, resp_info, &resp_len)){
            	BScript_Log("[ScriptDLS] SET_POSITION received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptDLS] Failed to SET_POSITION");
                return STEP_EXEC_ERROR;
            }

            if(resp_len == 2 && resp_info[0] != 0x3F ){
            	resp_len = 0;
            	BScript_Log("[ScriptDLS] Response SET_POSITION not OK, retry!");
                if(MIN_Send_SET_POSITION_CMD_WithData(position, resp_info, &resp_len)){
                	BScript_Log("[ScriptDLS] SET_POSITION received: %u bytes", resp_len);
                }else {
                    BScript_Log("[ScriptDLS] Failed to SET_POSITION");
                    return STEP_EXEC_ERROR;
                }
            }
            break;
        }

        case START_SAMPLING_CYCLE: { // start_sample_cycle
            BScript_Log("[ScriptDLS] ->START_SAMPLING_CYCLE: Start sample cycle");

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_START_SAMPLE_CYCLE_CMD_WithData(resp_info, &resp_len)){
            	BScript_Log("[ScriptDLS] START_SAMPLING_CYCLE received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptDLS] Failed to START_SAMPLING_CYCLE");
                return STEP_EXEC_ERROR;
            }

            break;
        }

        case GET_SAMPLE: { // obc_get_sample

            TickType_t handshake_start_time = xTaskGetTickCount();
            TickType_t overall_timeout_ticks = pdMS_TO_TICKS(SAMPLING_TIMEOUT_MS); // 25 s timeout
            TickType_t minbusy_low_start_time = 0;
            bool minbusy_low_started = false;

            while (1) {
                TickType_t now = xTaskGetTickCount();

                if ((now - handshake_start_time) >= overall_timeout_ticks) {
                    BScript_Log("[ScriptDLS] Timeout: MINBUSY is not LOW in 50ms or HIGH to long %lu ms", (unsigned long)overall_timeout_ticks);
                    return SIMPLE_TRANSFER_ERROR_MINBUSY_TIMEOUT;
                }

                if (!LL_GPIO_IsInputPinSet(EXP_MINBUSY_GPIO_PORT, EXP_MINBUSY_GPIO_PIN)) {
                    // MINBUSY is LOW
                    if (!minbusy_low_started) {
                        minbusy_low_start_time = now;
                        minbusy_low_started = true;
                    } else if ((now - minbusy_low_start_time) >= pdMS_TO_TICKS(50)) {
                        BScript_Log("[ScriptDLS] MINBUSY LOW 50ms, next step.");
                        break;
                    }
                } else {
                    minbusy_low_started = false;
                }
                BScript_Delayms(1);
            }

            BScript_Log("[ScriptDLS] ->GET_SAMPLE: Get sample");

            // Circle task

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_GET_INFO_SAMPLE_CMD_WithData(resp_info, &resp_len)){
            	BScript_Log("[ScriptDLS] GET_INFO_SAMPLE received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptDLS] Failed to GET_INFO_SAMPLE");
                return STEP_EXEC_ERROR;
            }

            uint16_t total_chunk = ((uint16_t)resp_info[0] << 8) | resp_info[1];
//            uint8_t total_chunk = resp_info[0];

            if(total_chunk > 256){
            	BScript_Log("[ScriptDLS] Wrong total chunk: %u (> 256)", total_chunk);
            	return STEP_EXEC_ERROR;
            }

            BScript_Log("[ScriptDLS] Got total chunk: %u (> 256)", total_chunk);

            char base_filename[48];

            s_DateTime rtc;
            Utils_GetRTC(&rtc);
            const char* type_prefix = "";
            type_prefix = "dls_data";
            snprintf(base_filename, sizeof(base_filename), "%s_20%02d%02d%02d_%02d%02d%02d",
                                type_prefix, rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);

            uint16_t chunk_id = 0;
            for(chunk_id = 0; chunk_id < total_chunk; chunk_id++){

                BScript_Log("[ScriptDLS] ->GET_SAMPLE: Chunks=%u, Base=%s",
                		chunk_id, base_filename);
                SimpleTransferResult_t result;
                result = SimpleDataTransfer_ExecuteTransfer(DATA_TYPE_CHUNK, chunk_id, base_filename, rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
                // Check result and return appropriate step result
                if (result == SIMPLE_TRANSFER_SUCCESS) {
                    BScript_Log("[ScriptDLS] ->GET_SAMPLE: Data acquisition chunk: %u completed successfully", chunk_id);
                } else {
                    BScript_Log("[ScriptDLS] ->GET_SAMPLE: Data acquisition failed: %s",
                               SimpleDataTransfer_GetResultString(result));
                    return STEP_EXEC_ERROR;
                }
            }
            BScript_Log("[ScriptDLS] ->GET_SAMPLE: Finish collect Chunk");
            BScript_Log("[ScriptDLS] ->GET_SAMPLE: Now, we shall collect Current sample data");
            type_prefix = "current_data";
            snprintf(base_filename, sizeof(base_filename), "%s_20%02d%02d%02d_%02d%02d%02d",
                                type_prefix, rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
            BScript_Log("[ScriptDLS] ->GET_SAMPLE: Base=%s", base_filename);
            SimpleTransferResult_t result;
            result = SimpleDataTransfer_ExecuteTransfer(DATA_TYPE_CURRENT, 0, base_filename, rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
            // Check result and return appropriate step result
            if (result == SIMPLE_TRANSFER_SUCCESS) {
                BScript_Log("[ScriptDLS] ->GET_SAMPLE: Data acquisition CURRENT completed successfully");
            } else {
                BScript_Log("[ScriptDLS] ->GET_SAMPLE: Data acquisition CURRENT failed: %s",
                           SimpleDataTransfer_GetResultString(result));
                return STEP_EXEC_ERROR;
            }

            BScript_Log("[ScriptDLS] ->GET_SAMPLE: [v] Done this stop");
            break;

        }

        default:
            BScript_Log("[ScriptDLS] Unknown action: 0x%02X", step->action_id);
            return STEP_EXEC_ERROR;
    }

    return STEP_EXEC_SUCCESS;
}

/**
 * @brief Execute CAM routine steps
 * @param step Step to execute
 * @return Execution result
 */
static StepExecResult ScriptManager_ExecuteCAMStep(Step* step)
{
    BScript_Log("[ScriptCAM] Executing step %u: action_id = 0x%02X", step->step_id, step->action_id);

    switch (step->action_id) {
        case SET_CAMERA_INTERVAL: { // set_camera_interval (now only for logging)
            uint32_t interval;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &interval) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }

            BScript_Log("[ScriptCAM] ->SET_CAMERA_INTERVAL: Reached step with interval %u seconds (using time points instead)", interval);
            break;
        }

        case SET_EXT_LASER_INTENSITY: { // set_ext_laser_intensity
            uint8_t intensity;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &intensity) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }
            BScript_Log("[ScriptCAM] ->SET_EXT_LASER_INTENSITY: Ext-laser intensity: %u", intensity);

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_SET_EXT_LASER_INTENSITY_CMD_WithData(intensity, resp_info, &resp_len)){
            	BScript_Log("[ScriptDLS] SET_EXT_LASER_INTENSITY received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptDLS] Failed to SET_EXT_LASER_INTENSITY");
                return STEP_EXEC_ERROR;
            }

            if(resp_len == 2 && resp_info[0] != 0x3F ){
            	resp_len = 0;
            	BScript_Log("[ScriptDLS] Response SET_EXT_LASER_INTENSITY not OK, retry!");
                if(MIN_Send_SET_EXT_LASER_INTENSITY_CMD_WithData(intensity, resp_info, &resp_len)){
                	BScript_Log("[ScriptDLS] SET_EXT_LASER_INTENSITY received: %u bytes", resp_len);
                }else {
                    BScript_Log("[ScriptDLS] Failed to SET_EXT_LASER_INTENSITY");
                    return STEP_EXEC_ERROR;
                }
            }

            break;
        }

        case TURN_ON_EXT_LASER: {
            uint8_t position;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &position) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }
            BScript_Log("[ScriptCAM] ->TURN_ON_EXT_LASER: Turn on ext-laser %u", position);

            uint8_t resp_info[2];
            uint8_t resp_len = 0;

            if(MIN_Send_TURN_ON_EXT_LASER_CMD_WithData(position, resp_info, &resp_len)){
            	BScript_Log("[ScriptDLS] TURN_ON_EXT_LASER_CMD received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptDLS] Failed to TURN_ON_EXT_LASER_CMD");
                return STEP_EXEC_ERROR;
            }

            if(resp_len == 2 && resp_info[0] != 0x3F ){
            	resp_len = 0;
            	BScript_Log("[ScriptDLS] Response TURN_ON_EXT_LASER_CMD not OK, retry!");
                if(MIN_Send_TURN_ON_EXT_LASER_CMD_WithData(position, resp_info, &resp_len)){
                	BScript_Log("[ScriptDLS] TURN_ON_EXT_LASER_CMD received: %u bytes", resp_len);
                }else {
                    BScript_Log("[ScriptDLS] Failed to TURN_ON_EXT_LASER_CMD");
                    return STEP_EXEC_ERROR;
                }
            }

            break;
        }

        case SET_CAMERA_POSITION: {
            uint8_t camPosition;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &camPosition) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }
            BScript_Log("[ScriptCAM] ->SET_CAMERA_POSITION: Set CIS-ID %u", camPosition);
            uint8_t payload[1];
            payload[0] = camPosition;

            if (MODFSP_Send(&cm4_protocol, MODFSP_TYPE_SET_CAM_POSITION_CMD,
                             payload, sizeof(payload)) != MODFSP_OK) {
                BScript_Log("[ScriptCAM] Master set camera position failed");
            }

            break;
        }

        case TAKE_IMG_WITH_TIMEOUT: { // take_img_with_timeout
            BScript_Log("[ScriptCAM] ->TAKE_IMG: Take image with timeout");
            if (MODFSP_Send(&cm4_protocol, MODFSP_TYPE_TAKE_IMAGE_CMD,
                             NULL, 0) != MODFSP_OK) {
                BScript_Log("[ScriptCAM] Master take image failed");

            }
            break;
        }

        case SCRIPT_DELAY: {
            uint32_t delayDuration;
            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &delayDuration) != PARSE_IDX_OK) {
                return STEP_EXEC_ERROR;
            }
            BScript_Log("[ScriptCAM] ->DELAY: Delay for camera %u ms",delayDuration);
            BScript_Log("[ScriptCAM] ->DELAY: Delay start!");
            BScript_Delayms(delayDuration);
            BScript_Log("[ScriptCAM] ->DELAY: Delay end!");
            break;
        }

        case TURN_OFF_EXT_LASER: {
            BScript_Log("[ScriptCAM] ->TURN_OFF_EXT_LASER: Turn all ext-laser off");

            uint8_t resp_info[1];
            uint8_t resp_len = 0;

            if(MIN_Send_TURN_OFF_EXT_LASER_CMD_WithData(resp_info, &resp_len)){
            	BScript_Log("[ScriptDLS] TURN_OFF_EXT_LASER_CMD received: %u bytes", resp_len);
            }else {
                BScript_Log("[ScriptDLS] Failed to TURN_OFF_EXT_LASER_CMD");
                return STEP_EXEC_ERROR;
            }

            break;
        }

        default:
            BScript_Log("[ScriptCAM] Unknown action: 0x%02X", step->action_id);
            return STEP_EXEC_ERROR;
    }

    return STEP_EXEC_SUCCESS;
}

/*************************************************
 *               STATUS FUNCTIONS                *
 *************************************************/

/**
 * @brief Get script execution state
 * @param type Script type
 * @return Current execution state
 */
ScriptExecState_t ScriptManager_GetScriptState(ScriptType_t type)
{
    if (type >= SCRIPT_TYPE_COUNT) return SCRIPT_EXEC_ERROR;
    return g_script_manager.contexts[type].state;
}

/**
 * @brief Get next run time for a script (now based on daily time points)
 * @param type Script type
 * @return Next run daily time in seconds (0-86399), or 0 if not available
 */
uint32_t ScriptManager_GetNextRunTime(ScriptType_t type)
{
    if (type >= SCRIPT_TYPE_COUNT) return 0;

    TimePointSchedule_t* schedule = NULL;

    switch (type) {
        case SCRIPT_TYPE_DLS_ROUTINE:
            schedule = &g_script_manager.dls_schedule;
            break;
        case SCRIPT_TYPE_CAM_ROUTINE:
            schedule = &g_script_manager.cam_schedule;
            break;
        default:
            return 0;
    }

    if (!schedule->is_configured || schedule->current_index >= schedule->count) {
        return 0;
    }

    return schedule->points[schedule->current_index].daily_timestamp;
}

/**
 * @brief Get time until next run
 * @param type Script type
 * @return Time until next run in seconds
 */
uint32_t ScriptManager_GetTimeUntilNextRun(ScriptType_t type)
{
    uint32_t next_run_daily = ScriptManager_GetNextRunTime(type);
    if (next_run_daily == 0) return 0;

    uint32_t current_daily_time = ScriptManager_GetCurrentDailyTimeSeconds();

    // If next run is later today
    if (current_daily_time <= next_run_daily) {
        return (next_run_daily - current_daily_time);
    } else {
        // Next run is tomorrow
        return (SECONDS_PER_DAY - current_daily_time + next_run_daily);
    }
}

/**
 * @brief Get execution statistics
 * @param dls_runs Pointer to store DLS run count
 * @param cam_runs Pointer to store CAM run count
 * @param errors Pointer to store error count
 */
void ScriptManager_GetStatistics(uint32_t* dls_runs, uint32_t* cam_runs, uint32_t* errors)
{
    if (dls_runs) *dls_runs = g_script_manager.dls_run_count;
    if (cam_runs) *cam_runs = g_script_manager.cam_run_count;
    if (errors) *errors = g_script_manager.total_errors;
}

/**
 * @brief Print detailed status report (UPDATED - includes FRAM info)
 */
void ScriptManager_PrintStatus(void)
{
    const char* state_names[] = {
        "IDLE", "RUNNING", "WAITING", "COMPLETED", "ERROR", "FAILED_MAX_RETRIES"
    };

    const char* script_names[] = {
        "INIT", "DLS", "CAM"
    };

    uint32_t current_time = ScriptManager_GetCurrentTimeSeconds();
    s_DateTime current_rtc;
    Utils_GetRTC(&current_rtc);

    BScript_Log("[ScriptManager] === STATUS REPORT ===");
    BScript_Log("[ScriptManager] Current RTC: 20%02d-%02d-%02d %02d:%02d:%02d",
               current_rtc.year, current_rtc.month, current_rtc.day,
               current_rtc.hour, current_rtc.minute, current_rtc.second);
    BScript_Log("[ScriptManager] System time: %u seconds", current_time);
    BScript_Log("[ScriptManager] Manager running: %s", g_script_manager.manager_running ? "YES" : "NO");
    BScript_Log("[ScriptManager] Init completed: %s", g_script_manager.init_completed ? "YES" : "NO");
    BScript_Log("[ScriptManager] Log Fetching: %s", g_log_fetching_enabled ? "ENABLED" : "DISABLED");

    if (g_script_manager.system_time_configured) {
        BScript_Log("[ScriptManager] System start daily time: %u seconds", g_script_manager.system_start_daily_time);
    }

    BScript_Log("[ScriptManager] === SD LOCK/RELEASE STATUS ===");
    if (g_sd_scheduler.configured) {
        uint8_t rel_h = g_sd_scheduler.release_time_sec / 3600;
        uint8_t rel_m = (g_sd_scheduler.release_time_sec % 3600) / 60;
        uint8_t rel_s = g_sd_scheduler.release_time_sec % 60;

        uint8_t lock_h = g_sd_scheduler.lockin_time_sec / 3600;
        uint8_t lock_m = (g_sd_scheduler.lockin_time_sec % 3600) / 60;
        uint8_t lock_s = g_sd_scheduler.lockin_time_sec % 60;

        BScript_Log("  - Configured: YES");
        BScript_Log("  - Release time: %02u:%02u:%02u (%u s)", rel_h, rel_m, rel_s, g_sd_scheduler.release_time_sec);
        BScript_Log("  - Lock-in time: %02u:%02u:%02u (%u s)", lock_h, lock_m, lock_s, g_sd_scheduler.lockin_time_sec);
        BScript_Log("  - Released today: %s", g_sd_scheduler.release_done_today ? "YES" : "NO");
        BScript_Log("  - Locked-in today: %s", g_sd_scheduler.lockin_done_today ? "YES" : "NO");
    } else {
        BScript_Log("  - Configured: NO");
    }

    BScript_Log("  - eMMC status: %s", SimpleDataTransfer_IsFatfsOk() ? "OK" : "FAIL");

    // Script status
    for (int i = 0; i < SCRIPT_TYPE_COUNT; i++) {
        ScriptExecContext_t* ctx = &g_script_manager.contexts[i];
        _Bool fram_exists = ScriptStorage_ScriptExists((ScriptType_t)i);

        BScript_Log("[ScriptManager] %s Script:", script_names[i]);
        BScript_Log("  - Loaded in RAM: %s", g_script_manager.scripts[i].is_loaded ? "YES" : "NO");
        BScript_Log("  - Exists in FRAM: %s", fram_exists ? "YES" : "NO");
        BScript_Log("  - State: %s", state_names[ctx->state]);
        BScript_Log("  - Current step: %d", ctx->current_step);
        BScript_Log("  - Retry count: %d/%d", ctx->retry_count, ctx->max_retries);

        // Show time point information for DLS and CAM
        if (i == SCRIPT_TYPE_DLS_ROUTINE) {
            TimePointSchedule_t* schedule = &g_script_manager.dls_schedule;
            BScript_Log("  - Time points configured: %s", schedule->is_configured ? "YES" : "NO");
            if (schedule->is_configured) {
                BScript_Log("  - Total points: %u", schedule->count);
                BScript_Log("  - Current point: %u", schedule->current_index);
                BScript_Log("  - Interval: %u seconds", schedule->interval_sec);
                BScript_Log("  - Next run: %u (in %u seconds)",
                           ScriptManager_GetNextRunTime((ScriptType_t)i),
                           ScriptManager_GetTimeUntilNextRun((ScriptType_t)i));
            }
        } else if (i == SCRIPT_TYPE_CAM_ROUTINE) {
            TimePointSchedule_t* schedule = &g_script_manager.cam_schedule;
            BScript_Log("  - Time points configured: %s", schedule->is_configured ? "YES" : "NO");
            if (schedule->is_configured) {
                BScript_Log("  - Total points: %u", schedule->count);
                BScript_Log("  - Current point: %u", schedule->current_index);
                BScript_Log("  - Interval: %u seconds", schedule->interval_sec);
                BScript_Log("  - Next run: %u (in %u seconds)",
                           ScriptManager_GetNextRunTime((ScriptType_t)i),
                           ScriptManager_GetTimeUntilNextRun((ScriptType_t)i));
            }
        }
    }

    BScript_Log("[ScriptManager] Statistics:");
    BScript_Log("  - DLS runs: %u", g_script_manager.dls_run_count);
    BScript_Log("  - CAM runs: %u", g_script_manager.cam_run_count);
    BScript_Log("  - Total errors: %u", g_script_manager.total_errors);

    // Print FRAM storage status
    BScript_Log("[ScriptManager] === FRAM STORAGE STATUS ===");
    ScriptStorage_PrintStatus();

    // Print detailed time point schedules
    if (g_script_manager.dls_schedule.is_configured) {
        BScript_Log("[ScriptManager] === DLS TIME POINTS ===");
        ScriptManager_PrintTimePoints(&g_script_manager.dls_schedule, "DLS");
    }

    if (g_script_manager.cam_schedule.is_configured) {
        BScript_Log("[ScriptManager] === CAM TIME POINTS ===");
        ScriptManager_PrintTimePoints(&g_script_manager.cam_schedule, "CAM");
    }

    BScript_Log("[ScriptManager] === END STATUS ===");
}
