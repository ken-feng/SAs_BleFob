/*
 * rtc_handle.c
 *
 *  Created on: 2020年8月20日
 *      Author: liyupeng
 */

#include "MKW38A4.h"
//#include "components\aEM\em00040101_log\EM000401.h"
//#include "framework\RTC\Interface\rtc_handle.h"
#include "EM000401.h"
#include "rtc_handle.h"
void rtc_init(void)
{
	rtc_config_t rtcConfig;
	rtc_datetime_t date;
    RTC_GetDefaultConfig(&rtcConfig);

    RTC_Init(RTC, &rtcConfig);
	date.year = 2000;
	date.month = 1;
	date.day = 1;
	date.hour = 12;
	date.minute = 0;
	date.second = 0;
	rtc_set_time(&date);

	rtc_get_time(&date);
	LOG_L_S(BLE_MD,"Current datetime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
            date.year,
            date.month,
            date.day,
            date.hour,
            date.minute,
            date.second);
}
void rtc_display_time(void)
{
	rtc_datetime_t date;
	rtc_get_time(&date);
	LOG_L_S(BLE_MD,"Current datetime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
            date.year,
            date.month,
            date.day,
            date.hour,
            date.minute,
            date.second);
}
int rtc_get_time(rtc_datetime_t *time)
{
	if (time == NULL)
	{
		return -1;
	}

	RTC_GetDatetime(RTC, time);
	return 0;
}

int rtc_set_time(rtc_datetime_t *time)
{
	int ret = 0;

	if (time == NULL)
	{
		return -1;
	}

	RTC_StopTimer(RTC);
	ret = RTC_SetDatetime(RTC, time);
	RTC_StartTimer(RTC);
	return ret;
}

void rtc_test_moudle(void)
{
	rtc_init();
	rtc_datetime_t date;
	rtc_get_time(&date);
	LOG_L_S(BLE_MD,"Current datetime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
            date.year,
            date.month,
            date.day,
            date.hour,
            date.minute,
            date.second);
	date.year = 2020;
	date.month = 8;
	date.day = 20;
	date.hour = 12;
	date.minute = 0;
	date.second = 0;
	rtc_set_time(&date);

	rtc_get_time(&date);
	LOG_L_S(BLE_MD,"Current datetime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
            date.year,
            date.month,
            date.day,
            date.hour,
            date.minute,
            date.second);
}

