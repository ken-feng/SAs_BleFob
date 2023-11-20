
#include "../../../../../framework/TimersManager/interface/TimersManager.h"
#include "../uwb_common_def.h"
#include "uwb_timer.h"


extern tmrTimerID_t uwbTimer;
static pfTmrCallBack_t uwbTmrCallBack;
static void *uwbParam;

void timing_pal1_channel_callBack0(void * userData)
{
	if(NULL != userData) (void*)userData;
}

void timing_pal1_channel_callBack1(void * userData)
{
	if(NULL != userData) (void*)userData;
}

void timing_pal1_channel_callBack2(void * userData)
{
	if(NULL != userData) (void*)userData;
}

static int create_timer(uint8_t timer_type, uint8_t timer_id, fp_Timer_Handler_t ptr_timer_handler, void* ptr_handler_param)
{
	uwbTmrCallBack = ptr_timer_handler;
	uwbParam = ptr_handler_param;
	return 1;
}

static int start_timer(uint8_t timer_id, uint64_t in_nanosecond)
{
	TMR_StartLowPowerTimer(uwbTimer, gTmrLowPowerSingleShotMillisTimer_c, in_nanosecond/(1000*1000), uwbTmrCallBack, uwbParam);
	return 0;
}

static void stop_timer(uint8_t timer_id)
{
	TMR_StopTimer(uwbTimer);
}

void destory_timer(void)
{

	// TIMING_Deinit(UWB_TEMER_INSTANCE);
}

static void ISR_delayns(void* user_data)
{
	*(bool*)user_data = false;
}

static void timer_tool_delay_ns(uint32_t ns)
{
	volatile bool bIsNotTimeOut = true;
	create_timer(0U, UWB_LPIT_TIMER0,(fp_Timer_Handler_t)&ISR_delayns,(void*)&bIsNotTimeOut);
	start_timer(UWB_LPIT_TIMER0, ns);
	while(bIsNotTimeOut);
	return ;
}

uint32_t API_UWB_Timer_Tools_Init(ST_TimerTools* pst_timer_tools)
{
	//Problem 1. Need the NULL pointer check in this here ..
	pst_timer_tools->fpCreateTimer 			= 	(fp_Create_Timer_t)		create_timer;
	pst_timer_tools->fpStartTimer 			= 	(fp_Start_Timer_t)		start_timer;
	pst_timer_tools->fpStopTimer 			= 	(fp_Stop_Timer_t)		stop_timer;
	pst_timer_tools->fpDestoryTimer			=	(fp_Destory_Timer_t)	destory_timer;
	pst_timer_tools->fpDelay				= 	(fp_Delay_t)			timer_tool_delay_ns;
	return 0;
}

uint32_t API_UWB_Timer_Tools_Deinit(ST_TimerTools* pst_timer_tools)
{
	destory_timer();

	pst_timer_tools->fpCreateTimer 			= NULL;
	pst_timer_tools->fpStartTimer 			= NULL;
	pst_timer_tools->fpStopTimer 			= NULL;
	pst_timer_tools->fpDestoryTimer			= NULL;
	pst_timer_tools->fpDelay				= NULL;
	return 0;
}


#if 0 
abandoned code
st_timer_tool_t 			st_timer_tool;
st_timer_tool_t*				pstTimerTools 	= NULL;
static volatile uint16_t second_cnt = 0;
static volatile uint16_t trgt_secode = 0;
static void ISR_universal(void* user_data)
{

	second_cnt++;
	if(second_cn)
}

static void timer_tool_start_universal_timer(uint16_t second)
{
	trgt_secode = second;
	second_cnt = 0;
	create_timer(TIMER_CHAN_TYPE_CONTINUOUS, UWB_LPIT_TIMER2,(ptr_timer_handler_t)&ISR_universal,NULL);
	start_timer(UWB_LPIT_TIMER0, NS_PERIOD_BY_1S);
	return ;
}

#endif

