/*
 * date_time.c
 *
 *  Created on: Feb 24, 2025
 *      Author: CAO HIEU
 */
#include "date_time.h"
#include "string.h"

#define NULL ((void *)0)

static s_DateTime s_RealTimeClock_context = {1, 1, 0, 0, 0, 0};

static s_Cronjob s_Cronjob_List[MAX_CRONJOBS] = {0};

uint8_t last_second = 0xFF, last_minute = 0xFF, last_hour = 0xFF;

static struct
{
    uint32_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} s_WorkingTimeClock_context = {0, 0, 0, 0};

static inline uint8_t isLeapYear(uint16_t fullYear)
{
    return ((fullYear % 4 == 0) && ((fullYear % 100 != 0) || (fullYear % 400 == 0))) ? 1 : 0;
}

static inline uint8_t getMaxDays(uint8_t month, uint16_t fullYear)
{
    static const uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        return 28 + isLeapYear(fullYear);
    } else {
        return daysInMonth[month - 1];
    }
}

static uint32_t DateTimeToEpoch(const s_DateTime *dt)
{
    uint32_t days = 0;
    uint16_t fullYear = 2000 + dt->year;

    for (uint16_t year = 2000; year < fullYear; year++)
    {
        days += 365 + isLeapYear(year);
    }
    for (uint8_t m = 1; m < dt->month; m++)
    {
        days += getMaxDays(m, fullYear);
    }
    days += dt->day - 1;

    return days * 86400UL + dt->hour * 3600UL + dt->minute * 60UL + dt->second;
}

void EpochToDateTime(uint32_t epoch, s_DateTime *dt)
{
    uint32_t days = epoch / 86400;
    uint32_t remSeconds = epoch % 86400;

    dt->hour   = remSeconds / 3600;
    remSeconds %= 3600;
    dt->minute = remSeconds / 60;
    dt->second = remSeconds % 60;

    uint16_t year = 2000;
    while (1)
    {
        uint16_t daysInYear = 365 + isLeapYear(year);
        if (days >= daysInYear)
        {
            days -= daysInYear;
            year++;
        } else
        {
            break;
        }
    }
    dt->year = year - 2000;

    uint8_t month = 1;
    while (1)
    {
        uint8_t dim = getMaxDays(month, year);
        if (days >= dim)
        {
            days -= dim;
            month++;
        }
        else
        {
            break;
        }
    }
    dt->month = month;
    dt->day = days + 1;
}

void Utils_SoftTime_Update(void) {
    // Update RTC
    if (++s_RealTimeClock_context.second >= 60) {
        s_RealTimeClock_context.second = 0;
        if (++s_RealTimeClock_context.minute >= 60) {
            s_RealTimeClock_context.minute = 0;
            if (++s_RealTimeClock_context.hour >= 24) {
                s_RealTimeClock_context.hour = 0;
                if (++s_RealTimeClock_context.day > getMaxDays(s_RealTimeClock_context.month, s_RealTimeClock_context.year)) {
                    s_RealTimeClock_context.day = 1;
                    if (++s_RealTimeClock_context.month > 12) {
                        s_RealTimeClock_context.month = 1;
                        if (++s_RealTimeClock_context.year > 99) {
                            s_RealTimeClock_context.year = 0;
                        }
                    }
                }
            }
        }
    }

    if (++s_WorkingTimeClock_context.seconds >= 60) {
        s_WorkingTimeClock_context.seconds = 0;
        if (++s_WorkingTimeClock_context.minutes >= 60) {
            s_WorkingTimeClock_context.minutes = 0;
            if (++s_WorkingTimeClock_context.hours >= 24) {
                s_WorkingTimeClock_context.hours = 0;
                s_WorkingTimeClock_context.days++;
            }
        }
    }

    // Cronjob
    uint32_t current_epoch = Utils_GetEpoch();
    uint8_t second_changed = (last_second != s_RealTimeClock_context.second);
    uint8_t minute_changed = (last_minute != s_RealTimeClock_context.minute);
    uint8_t hour_changed = (last_hour != s_RealTimeClock_context.hour);

    for (uint8_t i = 0; i < MAX_CRONJOBS; i++) {
        s_Cronjob *job = &s_Cronjob_List[i];
        if (!job->active || !job->callback) continue;

        switch (job->type) {
            case CRON_TYPE_MOMENT:
                if (second_changed && s_RealTimeClock_context.hour == job->hour &&
                    s_RealTimeClock_context.minute == job->minute &&
                    s_RealTimeClock_context.second == job->second) {
                	job->callback(job->context);
                    if (job->repeat_count > 0 && --job->remaining == 0) {
                        job->active = 0;
                    }
                }
                break;

            case CRON_TYPE_COUNTDOWN:
                if (current_epoch - job->last_trigger >= job->interval) {
                	job->callback(job->context);
                    job->last_trigger = current_epoch;
                    if (job->repeat_count > 0 && --job->remaining == 0) {
                        job->active = 0;
                    }
                }
                break;

            case CRON_TYPE_EVERY:
                switch (job->every_unit) {
                    case EVERY_HOUR:
                        if (hour_changed && s_RealTimeClock_context.hour == job->hour) {
                        	job->callback(job->context);
                            job->last_triggered_unit = job->hour;
                            if (job->repeat_count > 0 && --job->remaining == 0) {
                                job->active = 0;
                            }
                        }
                        break;
                    case EVERY_MINUTE:
                        if (minute_changed && s_RealTimeClock_context.minute == job->minute) {
                        	job->callback(job->context);
                            job->last_triggered_unit = job->minute;
                            if (job->repeat_count > 0 && --job->remaining == 0) {
                                job->active = 0;
                            }
                        }
                        break;
                    case EVERY_SECOND:
                        if (second_changed && s_RealTimeClock_context.second == job->second) {
                        	job->callback(job->context);
                            job->last_triggered_unit = job->second;
                            if (job->repeat_count > 0 && --job->remaining == 0) {
                                job->active = 0;
                            }
                        }
                        break;
                }
                break;
        }
    }

    last_second = s_RealTimeClock_context.second;
    last_minute = s_RealTimeClock_context.minute;
    last_hour = s_RealTimeClock_context.hour;
}

void Utils_SoftTime_Init(void)
{
    s_RealTimeClock_context.year = 0;  // 2000
    s_RealTimeClock_context.month = 1; // January
    s_RealTimeClock_context.day = 1;
    s_RealTimeClock_context.hour = 0;
    s_RealTimeClock_context.minute = 0;
    s_RealTimeClock_context.second = 0;

    s_WorkingTimeClock_context.days = 0;
    s_WorkingTimeClock_context.hours = 0;
    s_WorkingTimeClock_context.minutes = 0;
    s_WorkingTimeClock_context.seconds = 0;

    memset(s_Cronjob_List, 0, sizeof(s_Cronjob_List));
    for (uint8_t i = 0; i < MAX_CRONJOBS; i++) {
            s_Cronjob_List[i].active = 0;
    }
}

// ================= Helper Functions =================
void Utils_GetRTC(s_DateTime *dateTime)
{
    if (dateTime == NULL) return;
    *dateTime = s_RealTimeClock_context;
}

void Utils_SetRTC(const s_DateTime *dateTime)
{
    if (dateTime == NULL) return;
    s_RealTimeClock_context = *dateTime;
}
/*@usage:
 *  s_DateTime newTime = {15, 10, 23, 14, 30, 0}; // 15/10/2023 14:30:00
 *  DateTime_SetRTC(&newTime);
 */

void Utils_SetEpoch(uint32_t epoch)
{
    s_DateTime dt;
    if (epoch < EPOCH_OFFSET_UNIX)
    {
        return;
    }
    EpochToDateTime(epoch - EPOCH_OFFSET_UNIX, &dt);
    Utils_SetRTC(&dt);
}

uint32_t Utils_GetEpoch(void)
{
    return DateTimeToEpoch(&s_RealTimeClock_context) + EPOCH_OFFSET_UNIX;
}

void Utils_GetWorkingTime(uint32_t *days, uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    if (days) *days = s_WorkingTimeClock_context.days;
    if (hours) *hours = s_WorkingTimeClock_context.hours;
    if (minutes) *minutes = s_WorkingTimeClock_context.minutes;
    if (seconds) *seconds = s_WorkingTimeClock_context.seconds;
}

// ================= Cronjob Functions =================
uint8_t Utils_Cronjob_SetMoment(uint8_t hour, uint8_t minute, uint8_t second, uint32_t repeat_count, CronCallback_t callback, void *context, uint8_t index) {
    if (index >= MAX_CRONJOBS || callback == NULL || hour > 23 || minute > 59 || second > 59) return 1;
    if (s_Cronjob_List[index].active) return 1;

    s_Cronjob *job = &s_Cronjob_List[index];
    job->active = 1;
    job->type = CRON_TYPE_MOMENT;
    job->hour = hour;
    job->minute = minute;
    job->second = second;
    job->repeat_count = repeat_count;
    job->remaining = repeat_count;
    job->callback = callback;
    job->context = context;
    return 0;
}

uint8_t Utils_Cronjob_SetCountdown(uint32_t seconds, uint32_t repeat_count, CronCallback_t callback, void *context, uint8_t index) {
    if (index >= MAX_CRONJOBS || callback == NULL || seconds == 0) return 1;
    if (s_Cronjob_List[index].active) return 1;

    s_Cronjob *job = &s_Cronjob_List[index];
    job->active = 1;
    job->type = CRON_TYPE_COUNTDOWN;
    job->interval = seconds;
    job->last_trigger = Utils_GetEpoch();
    job->repeat_count = repeat_count;
    job->remaining = repeat_count;
    job->callback = callback;
    job->context = context;
    return 0;
}

uint8_t Utils_Cronjob_SetEvery(EveryUnit_t unit, uint8_t value, uint32_t repeat_count, CronCallback_t callback, void *context, uint8_t index) {
    if (index >= MAX_CRONJOBS || callback == NULL) return 1;
    if (s_Cronjob_List[index].active) return 1;
    if ((unit == EVERY_HOUR && value > 23) || (unit == EVERY_MINUTE && value > 59) || (unit == EVERY_SECOND && value > 59)) return 1;

    s_Cronjob *job = &s_Cronjob_List[index];
    job->active = 1;
    job->type = CRON_TYPE_EVERY;
    job->every_unit = unit;
    if (unit == EVERY_HOUR) job->hour = value;
    else if (unit == EVERY_MINUTE) job->minute = value;
    else job->second = value;
    job->repeat_count = repeat_count;
    job->remaining = repeat_count;
    job->callback = callback;
    job->context = context;
    return 0;
}

uint8_t Utils_Cronjob_Delete(uint8_t index) {
    if (index >= MAX_CRONJOBS) return 1;
    s_Cronjob_List[index].active = 0;
    return 0;
}

#if USE_EXTERNAL_RTC
Std_ReturnType Utils_SoftTime_Sync(void)
{
	s_DateTime currentTime;
	RV3129_HandleTypeDef *hrtc = RV3129_GetHandle();
	Std_ReturnType ret = E_ERROR;
	ret = RV3129_GetTime(hrtc, &currentTime);
    if(ret == E_OK)
    {
       Utils_SetRTC(&currentTime);
    }
    return ret;
}
#endif
