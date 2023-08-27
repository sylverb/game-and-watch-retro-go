#include "main.h"
#include "rg_rtc.h"
#include "rg_i18n.h"
#include "stm32h7xx_hal.h"
#include <time.h>


RTC_TimeTypeDef GW_currentTime = {0};
RTC_DateTypeDef GW_currentDate = {0};

// Getters
uint8_t GW_GetCurrentHour(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentTime.Hours;

}
uint8_t GW_GetCurrentMinute(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentTime.Minutes;
}
uint8_t GW_GetCurrentSecond(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentTime.Seconds;
}

uint8_t GW_GetCurrentMonth(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentDate.Month;
}
uint8_t GW_GetCurrentDay(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentDate.Date;
}

uint8_t GW_GetCurrentWeekday(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentDate.WeekDay;
}
uint8_t GW_GetCurrentYear(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentDate.Year;
}

// Setters
void GW_SetCurrentHour(const uint8_t hour) {

    // Update time before we can set it
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    // Set time
    GW_currentTime.Hours = hour;
    if (HAL_RTC_SetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}
void GW_SetCurrentMinute(const uint8_t minute) {

    // Update time before we can set it
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    // Set time
    GW_currentTime.Minutes = minute;
    if (HAL_RTC_SetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}

void GW_SetCurrentSecond(const uint8_t second) {

    // Update time before we can set it
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    // Set time
    GW_currentTime.Seconds = second;
    if (HAL_RTC_SetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}

void GW_SetCurrentMonth(const uint8_t month) {

    // Update time before we can set it
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    // Set date
    GW_currentDate.Month = month;

    if (HAL_RTC_SetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}
void GW_SetCurrentDay(const uint8_t day) {

    // Update time before we can set it
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    // Set date
    GW_currentDate.Date = day;

    if (HAL_RTC_SetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}

void GW_SetCurrentWeekday(const uint8_t weekday) {

    // Update time before we can set it
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    // Set date
    GW_currentDate.WeekDay = weekday;

    if (HAL_RTC_SetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}
void GW_SetCurrentYear(const uint8_t year) {

    // Update time before we can set it
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    // Set date
    GW_currentDate.Year = year;

    if (HAL_RTC_SetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}

// Callbacks for UI purposes
bool hour_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {

    int8_t hour = GW_GetCurrentHour();
    int8_t min = 0;
    int8_t max = 23;

    if (event == ODROID_DIALOG_PREV) {
        if (hour == min)
            hour = max;
        else
            hour--;
        GW_SetCurrentHour(hour);
    }
    else if (event == ODROID_DIALOG_NEXT) {
        if (hour == max)
            hour = min;
        else
            hour++;
        GW_SetCurrentHour(hour);
    }

    sprintf(option->value, "%02d", hour);
    return event == ODROID_DIALOG_ENTER;
    return false;

}
bool minute_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) { 
    
    int8_t minute = GW_GetCurrentMinute();
    int8_t min = 0;
    int8_t max = 59;

    if (event == ODROID_DIALOG_PREV) {
        if (minute == min)
            minute = max;
        else
            minute--;
        GW_SetCurrentMinute(minute);
    }
    else if (event == ODROID_DIALOG_NEXT) {
        if (minute == max)
            minute = min;
        else
            minute++;
        GW_SetCurrentMinute(minute);
    }

    sprintf(option->value, "%02d", minute);
    return event == ODROID_DIALOG_ENTER;
    return false;

}
bool second_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) { 
    
    int8_t second = GW_GetCurrentSecond();
    int8_t min = 0;
    int8_t max = 59;

    if (event == ODROID_DIALOG_PREV) {
        if (second == min)
            second = max;
        else
            second--;
        GW_SetCurrentSecond(second);
    }
    else if (event == ODROID_DIALOG_NEXT) {
        if (second == max)
            second = min;
        else
            second++;
        GW_SetCurrentSecond(second);
    }

    sprintf(option->value, "%02d", second);
    return event == ODROID_DIALOG_ENTER;
    return false;

}

bool month_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) { 
        
    int8_t month = GW_GetCurrentMonth();
    int8_t min = 1;
    int8_t max = 12;

    if (event == ODROID_DIALOG_PREV) {
        if (month == min)
            month = max;
        else
            month--;
        GW_SetCurrentMonth(month);
    }
    else if (event == ODROID_DIALOG_NEXT) {
        if (month == max)
            month = min;
        else
            month++;
        GW_SetCurrentMonth(month);
    }

    sprintf(option->value, "%02d", month);
    return event == ODROID_DIALOG_ENTER;
    return false;
    
}
bool day_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) { 
        
    int8_t day = GW_GetCurrentDay();
    int8_t min = 1;
    int8_t max = 31;

    if (event == ODROID_DIALOG_PREV) {
        if (day == min)
            day = max;
        else
            day--;
        GW_SetCurrentDay(day);
    }
    else if (event == ODROID_DIALOG_NEXT) {
        if (day == max)
            day = min;
        else
            day++;
        GW_SetCurrentDay(day);
    }

    sprintf(option->value, "%02d", day);
    return event == ODROID_DIALOG_ENTER;
    return false;

}

time_t GW_GetUnixTime(void) {
    // Function to return Unix timestamp since 1st Jan 1970.
    // The time is returned as an 64-bit value, but only the top 32-bits are populated.
    
    time_t timestamp;
    struct tm timeStruct;

    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    timeStruct.tm_year = GW_currentDate.Year + 100;  // tm_year base is 1900, RTC can only save 0 - 99, so bump to 2000.
    timeStruct.tm_mday = GW_currentDate.Date;
    timeStruct.tm_mon  = GW_currentDate.Month - 1;

    timeStruct.tm_hour = GW_currentTime.Hours;
    timeStruct.tm_min  = GW_currentTime.Minutes;
    timeStruct.tm_sec  = GW_currentTime.Seconds;

    timestamp = mktime(&timeStruct);

    return timestamp;

}

bool weekday_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) { 
    const char * GW_RTC_Weekday[] = {curr_lang->s_Weekday_Mon, curr_lang->s_Weekday_Tue, curr_lang->s_Weekday_Wed, curr_lang->s_Weekday_Thu, curr_lang->s_Weekday_Fri, curr_lang->s_Weekday_Sat, curr_lang->s_Weekday_Sun};
                
    int8_t weekday = GW_GetCurrentWeekday();
    int8_t min = 1;
    int8_t max = 7;

    if (event == ODROID_DIALOG_PREV) {
        if (weekday == min)
            weekday = max;
        else
            weekday--;
        GW_SetCurrentWeekday(weekday);
    }
    else if (event == ODROID_DIALOG_NEXT) {
        if (weekday == max)
            weekday = min;
        else
            weekday++;
        GW_SetCurrentWeekday(weekday);
    }

    sprintf(option->value, "%s", (char *) GW_RTC_Weekday[weekday-1]);
    return event == ODROID_DIALOG_ENTER;
    return false;
    
}
bool year_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) { 
            
    int8_t year = GW_GetCurrentYear();
    int8_t min = 0;
    int8_t max = 99;

    if (event == ODROID_DIALOG_PREV) {
        if (year == min)
            year = max;
        else
            year--;
        GW_SetCurrentYear(year);
    }
    else if (event == ODROID_DIALOG_NEXT) {
        if (year == max)
            year = min;
        else
            year++;
        GW_SetCurrentYear(year);
    }

    sprintf(option->value, "20%02d", year);
    return event == ODROID_DIALOG_ENTER;
    return false;
    
}

bool time_display_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    curr_lang->fmtTime(option->value, curr_lang->s_Time_Format, GW_currentTime.Hours, GW_currentTime.Minutes, GW_currentTime.Seconds);
    return event == ODROID_DIALOG_ENTER;
    return false;    
}
bool date_display_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {

    const char * GW_RTC_Weekday[] = {curr_lang->s_Weekday_Mon, curr_lang->s_Weekday_Tue, curr_lang->s_Weekday_Wed, curr_lang->s_Weekday_Thu, curr_lang->s_Weekday_Fri, curr_lang->s_Weekday_Sat, curr_lang->s_Weekday_Sun};
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);
    
    curr_lang->fmtDate(option->value, curr_lang->s_Date_Format, GW_currentDate.Date, GW_currentDate.Month, GW_currentDate.Year, (char *) GW_RTC_Weekday[GW_currentDate.WeekDay-1]);
    return event == ODROID_DIALOG_ENTER;
    return false;
}