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
