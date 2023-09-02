#ifndef _GW_RTC_H_
#define _GW_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "odroid_system.h"

/* Exported constants --------------------------------------------------------*/
extern RTC_TimeTypeDef GW_currentTime;
extern RTC_DateTypeDef GW_currentDate;
extern const char * GW_RTC_Weekday[];

/* Exported functions prototypes ---------------------------------------------*/

// Getters
uint8_t GW_GetCurrentHour(void);
uint8_t GW_GetCurrentMinute(void);
uint8_t GW_GetCurrentSecond(void);

uint8_t GW_GetCurrentMonth(void);
uint8_t GW_GetCurrentDay(void);

uint8_t GW_GetCurrentWeekday(void);
uint8_t GW_GetCurrentYear(void);

time_t GW_GetUnixTime(void);

// Setters
void GW_SetCurrentHour(const uint8_t hour);
void GW_SetCurrentMinute(const uint8_t minute);
void GW_SetCurrentSecond(const uint8_t second);

void GW_SetCurrentMonth(const uint8_t month);
void GW_SetCurrentDay(const uint8_t day);

void GW_SetCurrentWeekday(const uint8_t weekday);
void GW_SetCurrentYear(const uint8_t year);

#ifdef __cplusplus
}
#endif

#endif