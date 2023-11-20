/*
 * rtc_handle.h
 *
 *  Created on: 2020年8月20日
 *      Author: liyupeng
 */

#ifndef RTC_INTERFACE_RTC_HANDLE_H_
#define RTC_INTERFACE_RTC_HANDLE_H_

#include "fsl_rtc.h"

void rtc_init(void);

int rtc_get_time(rtc_datetime_t *time);

void rtc_display_time(void);

int rtc_set_time(rtc_datetime_t *time);

void rtc_test_moudle(void);

#endif /* RTC_INTERFACE_RTC_HANDLE_H_ */
