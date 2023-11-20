#ifndef _BLE_CCC_UWB_H_
#define _BLE_CCC_UWB_H_
#include "EM000101.h"
#include "ble_ccc_uwb.h"
#include "ble_ccc_dk.h"

#define DK_PROTOCOL_VER_MAX_NUMBER		10
#define UWB_CONFIG_ID_MAX_NUMBER		10
#define PULSESHAPE_COMBO_MAX_NUMBER		10





/*Ranging_Capability_RQ*/
typedef struct 
{
	u8 Supported_DK_Protocol_Version_Len;
	u8 Supported_DK_Protocol_Version[DK_PROTOCOL_VER_MAX_NUMBER];
	u8 Supported_UWB_Config_Id_Len;
	u8 Supported_UWB_Config_Id[UWB_CONFIG_ID_MAX_NUMBER];
	u8 Supported_PulseShape_Combo_Len;
	u8 Supported_PulseShape_Combo[PULSESHAPE_COMBO_MAX_NUMBER];
}ccc_uwb_ranging_capability_rq_t;


/*Ranging_Capability_RS*/
typedef struct 
{
	u16 Selected_DK_Protocol_Version;
	u16 Selected_UWB_Config_Id;
	u8 	Selected_PulseShape_Combo;
}ccc_uwb_ranging_capability_rs_t;

/*Ranging_Session_RQ*/
typedef struct 
{
	u16 Selected_DK_Protocol_Version;
	u16 Selected_UWB_Config_id;
	u32 UWB_Session_Id;
	u8  Selected_PulseShape_Combo; 
	u8  Channel_Bitmask;
}ccc_uwb_ranging_session_rq_t;


/*Ranging_Session_RS*/
typedef struct 
{
	u8 	RAN_Multiplier;
	u8 	Slot_BitMask;
	u32 SYNC_Code_Index_BitMask;
	u8  Selected_UWB_Channel;
	u8  Hopping_Config_Bitmask;
}ccc_uwb_ranging_session_rs_t;

/*Ranging_Session_Setup_RQ*/
typedef struct 
{
	u8 	Session_RAN_Multiplier;
	u8 	Number_Chaps_per_Slot;
	u8 	Number_Responders_Nodes;
	u8 	Number_Slots_per_Round;
	u32 SYNC_Code_Index;
	u8 	Selected_Hopping_Config_Bitmask;
}ccc_uwb_ranging_session_setup_rq_t;


/*Ranging_Session_Setup_RS*/
typedef struct 
{
	u32 STS_Index0;
	u8  UWB_Time0[8];
	u32 HOP_Mode_Key;
	u8 	SYNC_Code_Index;
}ccc_uwb_ranging_session_setup_rs_t;

typedef enum
{
	UWB_STEP_INIT = 0,
	UWB_STEP_RANGING_CAPABILITY_RQ,			/*测距能力请求（RC-RQ）*/
	UWB_STEP_RANGING_CAPABILITY_RS,			/*测距能力响应（RC-RS）*/
	UWB_STEP_RANGING_SESSION_RQ, 			/*测距会话请求（RS-RQ）*/
	UWB_STEP_RANGING_SESSION_RS,	  		/*测距会话响应（RS-RS）*/	
	UWB_STEP_RANGING_SESSION_SETUP_RQ,		/*测距会话建立请求（RSS-RQ）*/
	UWB_STEP_RANGING_SESSION_SETUP_RS,		/*测距会话建立响应（RSS-RS）*/	
	UWB_STEP_RANGING_SUSPEND_RQ,			/*测距挂起请求消息（RSD-RQ）*/
	UWB_STEP_RANGING_SUSPEND_RS,			/*测距挂起响应消息（RSD-RS）*/	
	UWB_STEP_RANGING_RECOVERY_RQ,			/*测距恢复请求消息（RR-RQ）*/
	UWB_STEP_RANGING_RECOVERY_RS,			/*测距恢复响应消息（RR-RS）*/	
	UWB_STEP_CONFIG_RANGING_RECOVERY_RQ,	/*可配置测距恢复请求消息（CRR-RQ）*/
	UWB_STEP_CONFIG_RANGING_RECOVERY_RS,	/*可配置测距恢复响应消息（CRR-RS）*/


	UWB_STEP_CONFIG_OTHER,					/*其他流程*/
} uwbStep_t;

typedef enum
{
	UWB_EVT_NTF_NONE = 0,
	UWB_EVT_NTF_URSK_NOT_FOUND,
} uwbEvtNtf_t;
typedef enum
{
	DK_UWB_RANGING_CAPABILITY_RQ = 1, 	/*测距能力请求（RC-RQ）*/
	DK_UWB_RANGING_CAPABILITY_RS,	  	/*测距能力响应（RC-RS）*/	
	DK_UWB_RANGING_SESSION_RQ, 			/*测距会话请求（RS-RQ）*/
	DK_UWB_RANGING_SESSION_RS,	  		/*测距会话响应（RS-RS）*/	
	DK_UWB_RANGING_SESSION_SETUP_RQ,	/*测距会话建立请求（RSS-RQ）*/
	DK_UWB_RANGING_SESSION_SETUP_RS,	/*测距会话建立响应（RSS-RS）*/	
	DK_UWB_RANGING_SUSPEND_RQ,			/*测距挂起请求消息（RSD-RQ）*/
	DK_UWB_RANGING_SUSPEND_RS,			/*测距挂起响应消息（RSD-RS）*/	
	DK_UWB_RANGING_RECOVERY_RQ,			/*测距恢复请求消息（RR-RQ）*/
	DK_UWB_RANGING_RECOVERY_RS,			/*测距恢复响应消息（RR-RS）*/	
	DK_UWB_CONFIG_RANGING_RECOVERY_RQ,	/*可配置测距恢复请求消息（CRR-RQ）*/
	DK_UWB_CONFIG_RANGING_RECOVERY_RS,	/*可配置测距恢复响应消息（CRR-RS）*/	
} uwbCmd_t;

typedef struct 
{
	uwbStep_t 	uwbStep; /*当前处于的步骤*/
	uwbEvtNtf_t uwbEvtNtf;  /*当前通知的事件*/
}ccc_uwb_txt_t;

/**
 * @brief
 *      UWB协商参数初始化      
 * @note
 */
void ble_ccc_uwb_init(void);

/**
 * @brief
 *      解析收到的DK数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_uwb_message_recv(ccc_dk_message_txt_t* dkMessage);

/**
 * @brief
 *      解析收到的DK数据
 * @param [outBuffer]     	待发送的数据缓存
 * @param [length]     		待发送的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_uwb_message_send(u8* outBuffer, u16 length);

/**
 * @brief
 *      设置测距会话 session id
 * @param [sessionId]     会话ID
 * @return
 * @note
 */
void ble_ccc_uwb_set_rs_session_id(u32 sessionId);
/**
 * @brief
 *      设置测距会话 channel bitmask
 * @param [Channel_Bitmask]     channel bitmask
 * @return
 * @note
 */
void ble_ccc_uwb_set_rs_channel_bitmask(u8 Channel_Bitmask);


/**
 * @brief
 *      处理蓝牙连接后的事情，UWB主动上发测距能力请求
 * @param 
 * @return
 *        
 * @note 
 *      连接事件通知
 */
void ble_ccc_connect_uwb_notify(void);
/**
 * @brief
 *      处理蓝牙断开连接后的事情
 * @param 
 * @return
 *        
 * @note 
 *      断开连接事件通知
 */
void ble_ccc_disconnect_uwb_notify(void);

/**
 * @brief
 *      测距会话建立，握手成功，通知UWB模块开始测距
 * @param 
 * @return
 *        
 * @note 
 */
void ble_ccc_uwb_notify_uwb_sdk_step1(void);
/**
 * @brief
 *      测距会话建立，握手成功，通知UWB模块开始测距
 * @param 
 * @return
 *        
 * @note 
 */
void ble_ccc_uwb_notify_uwb_sdk_step2(void);

cccResult_t ble_ccc_uwb_send_cmd(uwbCmd_t uwbCmd);


void ble_ccc_uwb_process(u8* inBuffer, u16 length);
#endif
