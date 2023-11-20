#ifndef _BLE_CCC_H_
#define _BLE_CCC_H_
#include "EM000101.h"
#include "EM000401.h"
#include "RTE_ccc.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "ble_general.h"

#define	CCC_QUEUE_MAX_NUMBER		50
/****L2CAP通道****/
#if defined(gCccL2Cap_d) && (gCccL2Cap_d == 1)
#define gCcc_L2capLePsm_c  				0x0085	//0x004F
#define gCccCmdMaxLength_c 				0x200	//270
#define mAppLeCbInitialCredits_c        (65500)	//(32768)
#endif

extern gapPhyEvent_t  gCccLePhy;


#define CCC_MAX_URSK_NUMBER			2U

#define BLE_L2CAP_STATUS_CONNECT        1
#define BLE_L2CAP_STATUS_DISCONNECT     0

typedef enum
{
	CCC_RESULT_SUCCESS = 0x0000,	        //0x0000 : 成功
	CCC_RESULT_FAILED,			    		//0x0001 : 失败
	CCC_RESULT_ERROR_LENGTH,				//0x0002: 数据长度错误
    CCC_RESULT_ERROR_SE_NOT_SEND,		    //0x0003 SE message 数据不透传到SE
}cccResult_t;

typedef enum
{
	CCC_EVT_IDLE = 0,
	CCC_EVT_STATUS_CONNECT,			/*连接蓝牙通知*/
	CCC_EVT_L2CAP_SETUP_COMPLETE,	/*L2CAP通道建立完成*/
	CCC_EVT_L2CAP_DISCONNECT,		/*L2CAP通道断开*/
	CCC_EVT_STATUS_DISCONNECT,		/*断开蓝牙连接通知*/
	CCC_EVT_CMD_DISCONNECT,			/*主动断开蓝牙连接*/
	CCC_EVT_LESETPHY,				/*设置LE PHY*/
	CCC_EVT_RECV_DATA,				/*收到L2CAP数据*/
	CCC_EVT_SEND_DATA,				/*发送L2CAP数据*/
	CCC_EVT_TIME_SYCN0,				/*蓝牙连上第一次时间同步*/
	CCC_EVT_TIME_SYCN1,				/*第2次时间同步*/
	CCC_EVT_DK,						/*DK 模块事件处理*/
	CCC_EVT_DK_UWB,					/*DK UWB模块事件处理*/
	CCC_EVT_DK_NOTIFY,				/*DK NOTIFY模块事件处理*/
	CCC_EVT_DK_SE,					/*DK SE模块事件处理*/
	CCC_EVT_DK_SUP_SERVICE,			/*DK SUP SERVICE模块事件处理*/


	UWB_EVT_WAKEUP_RQ,				/*唤醒UWB 请求*/
	UWB_EVT_WAKEUP_RS,				/*唤醒UWB 响应*/

	UWB_EVT_TIMER_SYCN_RQ,			/*时间同步 请求*/
	UWB_EVT_TIMER_SYCN_RS,			/*时间同步 响应*/

	UWB_EVT_RANGING_SESSION_SETUP_RQ,	/*测距会话建立 请求*/
	UWB_EVT_RANGING_SESSION_START_RQ,	/*测距会话开始 请求*/
	UWB_EVT_RANGING_SESSION_START_RS,	/*测距会话开始 响应*/

	UWB_EVT_RANGING_SESSION_SUSPEND_RQ,	/*测距会话挂起恢复停止 请求*/
	UWB_EVT_RANGING_SESSION_SUSPEND_RS, /*测距会话挂起恢复停止 响应*/
	
	UWB_EVT_RANGING_RESULT_NOTICE,		/*测距结果通知*/

	UWB_EVT_INT_NOTICE,		/*UWB INT 中断事件*/
	UWB_EVT_RECV_UART,		/*UWB UART*/

}ccc_evt_type_t;

// typedef enum
// {
// 	CCC_LESETPHY_STEP0 = 0,
// 	CCC_LESETPHY_STEP1,
// 	CCC_LESETPHY_STEP2,
// };

typedef struct 
{
	u8 ursk[32];
	u8 sessionId[4];
	u8 validFlag;
}ble_ccc_ursk_t;


typedef struct 
{
    u8 connectStatus;   /*当前蓝牙连接状态  0:断开  1:连上*/
    u8 deviceId;        /*当前连接的设备Id*/
    u8 channelId;       /*当前连接的L2CAP 逻辑通道Id*/

	u16 start_counter;	 /*开始时间同步起始counter值, ce_counter是从连上就开始计数, 
							实际时间同步时也需要从0开始计数，因此需要减去中间的差值*/
	u16 ce_counter;      /*!< Connection event counter, valid for conn event over or Conn Rx event */
	u16 timestamp;       /*!< Timestamp in 625 us slots, valid for Conn Rx event and Conn Created event */

	u16 DeviceEventCount; /*设备连接时间计数值,从0开始*/

	u8 selectUrsk[32];		/*当前选中的URSK*/
	u8 selectSessionId[4];	/*当前选中的SESSION ID*/

	ble_ccc_ursk_t ble_ccc_ursk[CCC_MAX_URSK_NUMBER];
}ble_ccc_ctx_t;

extern ble_ccc_ctx_t ble_ccc_ctx;


typedef struct 
{
    u16 length;
    u8* data;
}ble_ccc_data_t;

typedef struct 
{
    ccc_evt_type_t evtType;	/*消息类型*/
	u8 deviceId;			/*消息设备ID*/
	u16 length;				/*消息数据长度*/
	u8* dataBuff;			/*消息数据体*/
}ble_ccc_queue_msg_t;

/**初始化***/
void ble_ccc_init(void);
/*收取L2CAP 逻辑通道上数据*/
cccResult_t ble_ccc_l2cap_recv_data(u8 deviceId, u8* inData, u16 length);
/*往L2CAP 逻辑通道上发送数据 */
cccResult_t ble_ccc_l2cap_send_data(u8* outData, u16 length);

/*设置L2CAP 连接参数*/
void ble_ccc_l2cap_set_parameter(u8 deviceId, u8 channelId);
/*设置L2CAP 连接状态*/
void ble_ccc_l2cap_set_connect_status(u8 status);
/*获取已连接的L2CAP ChannelId*/
u8 ble_ccc_l2cap_get_connectChannelId(void);


void ble_ccc_task_init(void);
//
u8 ble_ccc_send_leSetPhyRequest(void);
u8 ble_ccc_send_evt(ccc_evt_type_t evtType, u8 deviceId, u8* pdata, u16 length);

/*****时间同步操作******/

/*重置ce counter*/
void ble_ccc_reset_ce_counter(void);
/*增长ce counter*/
void ble_ccc_increase_ce_counter(u16 ceCounter);
/*获取ce counter*/
u16 ble_ccc_get_ce_counter(void);
/*清除缓存ursk*/
void ble_ccc_clear_ursk(void);
/*通过session id 查找 ursk*/
boolean ble_ccc_find_ursk(u8* sessionId, u8* ursk);
/*设置已选中的 ursk 和session id*/
void ble_ccc_set_select_ursk_sessionid(u8* sessionId,u8* ursk);

#endif
