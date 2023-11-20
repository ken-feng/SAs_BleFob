/*
 * user_timer.h
 *
 *  Created on: 2022��7��7��
 *      Author: JohnSong
 */

#ifndef SOURCES_UWB_DRIVERS_UWB_TIMER_H_
#define SOURCES_UWB_DRIVERS_UWB_TIMER_H_

#include "../uwb_common_def.h"
/* LPIT channel used */
#define UWB_TEMER_INSTANCE (TIMING_OVER_LPIT0_INSTANCE)

#define UWB_LPIT_TIMER0	    0UL		//ns precise delay
#define UWB_LPIT_TIMER1	    1UL		//up to you
#define UWB_LPIT_TIMER2	    2UL		//ms UWB Communication TimerOut

#define NS_PERIOD_UWB_REST_TIME_5_4_MS	(5400000UL)	//uwb boot time RCI，UM11197 | NCJ29D5 RCI user manual NCJ29D5 Remote control interface user manual
#define NS_PERIOD_BY_1S					(1000000000UL) /* The period is 1 second */
#define NS_PERIOD_BY_S(t)				(t*1000000000UL)
#define NS_PERIOD_BY_US(t) 				(t*1000)
#define NS_PERIOD_BY_MS(t) 				(t*1000000UL)

extern uint32_t API_UWB_Timer_Tools_Init(ST_TimerTools* pst_timer_tools);
extern uint32_t API_UWB_Timer_Tools_Deinit(ST_TimerTools* pst_timer_tools);

#endif /* SOURCES_UWB_DRIVERS_UWB_TIMER_H_ */
#if 0 
abandoned code
volatile bool universal_timer_out_flag;
extern st_timer_tool_t 				st_timer_tool;
extern st_timer_tool_t*				pstTimerTools;
#endif
