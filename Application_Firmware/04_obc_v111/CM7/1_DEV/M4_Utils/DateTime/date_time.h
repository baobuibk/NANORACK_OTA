#ifndef M4_UTILS_DATETIME_DATE_TIME_H_
#define M4_UTILS_DATETIME_DATE_TIME_H_
#include "stdint.h"

#define USE_EXTERNAL_RTC 1
#define EPOCH_OFFSET_UNIX 946684800UL  //  01/01/1970 - 01/01/2000
#define MAX_CRONJOBS 10

typedef struct {
	uint8_t day;    //!< Day: Starting at 1 for the first day
	uint8_t month;  //!< Month: Starting at 1 for January
	uint8_t year;   //!< Year in format YY (2000 - 2099)
	uint8_t hour;   //!< Hour
	uint8_t minute; //!< Minute
	uint8_t second; //!< Second
} s_DateTime;

// Types of cron jobs
typedef enum {
    CRON_TYPE_MOMENT,   // Triggers at a specific moment
    CRON_TYPE_COUNTDOWN,// Triggers after a countdown interval
    CRON_TYPE_EVERY     // Triggers when hour/minute/second match
} CronType_t;

// Time units for CRON_TYPE_EVERY
typedef enum {
    EVERY_HOUR,
    EVERY_MINUTE,
    EVERY_SECOND
} EveryUnit_t;

// Callback function pointer
typedef void (*CronCallback_t)(void *context);

// Cron job structure
typedef struct {
    uint8_t active;               // 1 = active, 0 = inactive
    CronType_t type;              // Type of cron job
    uint8_t hour;                 // Hour (for MOMENT or EVERY)
    uint8_t minute;               // Minute (for MOMENT or EVERY)
    uint8_t second;               // Second (for MOMENT or EVERY)
    uint32_t interval;            // Interval (seconds, for COUNTDOWN)
    uint32_t last_trigger;        // Last trigger time (epoch, for COUNTDOWN)
    EveryUnit_t every_unit;       // Unit for EVERY
    uint8_t last_triggered_unit;  // Last triggered value (for EVERY)
    uint32_t repeat_count;        // Number of repetitions (0 = infinite)
    uint32_t remaining;           // Remaining repetitions
    CronCallback_t callback;      // Callback function
    void *context;
} s_Cronjob;


void Utils_SoftTime_Update(void);
void EpochToDateTime(uint32_t epoch, s_DateTime *dt);
void Utils_GetRTC(s_DateTime *dateTime);
void Utils_SetRTC(const s_DateTime *dateTime);
void Utils_SetEpoch(uint32_t epoch);
uint32_t Utils_GetEpoch(void);
void Utils_GetWorkingTime(uint32_t *days, uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void Utils_SoftTime_Init(void);

// API Cronjob
uint8_t Utils_Cronjob_SetMoment(uint8_t hour, uint8_t minute, uint8_t second, uint32_t repeat_count, CronCallback_t callback, void *context, uint8_t index);
uint8_t Utils_Cronjob_SetCountdown(uint32_t seconds, uint32_t repeat_count, CronCallback_t callback, void *context, uint8_t index);
uint8_t Utils_Cronjob_SetEvery(EveryUnit_t unit, uint8_t value, uint32_t repeat_count, CronCallback_t callback, void *context, uint8_t index);
uint8_t Utils_Cronjob_Delete(uint8_t index);

#if USE_EXTERNAL_RTC
#include "devices.h"
Std_ReturnType Utils_SoftTime_Sync(void);
#endif

#endif /* M4_UTILS_DATETIME_DATE_TIME_H_ */
