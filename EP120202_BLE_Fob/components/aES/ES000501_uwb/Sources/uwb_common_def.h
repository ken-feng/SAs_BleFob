/*
* uwb_common_def.h
*
*  Created on: 2022年7月13日
*      Author: JohnSong
*/
/* 开发未完成工作列表
* ---------------------------------------------------------------------
* 1. 对指令的解析不完整，仅支持少部分逻辑分支
* 2. 对指令的拼装不完整，仅支持部分拼装
* 3. CAN报文拼装没做
* 4. CAN报文解析没做
* 5. 获取硬件信息接没做
* 6. 在进入Ranging Session 后，对RESP和NTF的处理要调整，否则会慢，（加一个状态判断，进入Ranging Session后，每收一条NTF就要走一个循环，不再检查后续是否有RESP
* 7. 状态字解析输出
*
*
*
*
* 缺 Ranger4MAC_DS.pdf
*
*
* Note：
* SPI ：
* 1. 由于一包UCI命令数据 sn信号上下拉一次，所以sn需要手动下拉，snPin设置成GPIO操作。
* 2. UCI与RCI的SPI MAC 层存在差异。不能共用。当前代码实现UCI MAC层通信时序，如果期望使用RCI，需要另外实现。
* 3. RCI/UCI的数据编码格式是LSB的，例如CRC16，低位在前。
*
*
* UCI ：
* 1. UCI 框架下，接收的时候，从机会在Resp头上填充一个0x00，实际Resp从第1个字节开始。
* 2. UCI 框架下，发送数据到29D5后，29D5需要时间处理，所以在接收端口，超时范围要把这个时间算进去，目前测试下来不可以低于150ms。
* 3. UCI 框架下， CMD与RESP并非即时返回，中间可能穿插NTF，所以需要处理CMD出去后，NTF先到，RESP后到的情况。
* 4. UCI 框架下， NTF可能会连续跟多个NTF
* ---------------------------------------------------------------------
*
*
*
* 细节：
* 1. 占空比，后期需要关注，如果SPI不准，需要优化调整。 通过 pwm 组件 或者其他方式 实现，参考lpspi例程。
*
*
*
*
*
*/


#ifndef SOURCES_UWB_COMMON_DEF_H_
#define SOURCES_UWB_COMMON_DEF_H_

//Global define
#define _DEBUG
#define _COMMUNI_TYPE_IS_UART	(1u)
#define NXP_UCI_FRAME
//#define UWB_BLE_ANCHOR_AND_SA_CONTROLL

//#define UWB_RESPONDER		(1u)
#define UWB_INITIATOR		(2u)

#define UWB_ANCHOR_INDEX			(4)


//global def
#define DEF_UWB_CAN_BUF_SIZE 64
#define DEF_RANGING_RESULT_BUFFER_SIZE 5


#include <stdint.h>
#include <stdbool.h>

#include "EM000101.h"
#include "EM000401.h"
typedef enum enum_UWB_CommonErrCode_t
{
	//0x0000 0000 - 0x0000 0FFF	Generic Error
	UWB_Err_Success_0 															            = 0x00000000,
	UWB_Err_Success_1 															            = 0x00000001,
	UWB_Err_UnKnowErr															            = 0x00000002,
	UWB_Err_Null_Data_Pointer													            = 0x00000003,
	UWB_Err_CMD_Undefine														            = 0x00000004,
	UWB_Err_Incorrect_CMD_With_Status											            = 0x00000005,

	//0x0000 1000 - 0x0000 1FFF	HardWare Error
	//UWB_Err_SPI_Boot_Failed													            = 0x00001001,
	UWB_Err_Module_Hard_Rest_Faield												            = 0x00001002,
	UWB_Err_Module_Offline_MSG_Reject											            = 0x00001003,
	UWB_Err_DRIV_SPI_Boot_Failed												            = 0x00001004,
	UWB_Err_DRIV_SPI_Send_Failed												            = 0x00001005,
	UWB_Err_DRIV_SPI_Recv_Failed												            = 0x00001006,
	UWB_Err_DRIV_SPI_AbortTransmit_Failed										            = 0x00001007,
	UWB_Err_DRIV_Line_State_Exception											            = 0x00001008,
	UWB_Err_DRIV_Recv_Time_Out_Abort_Failed										            = 0x00001009,
	UWB_Err_DRIV_Send_Time_Out_Abort_Failed										            = 0x0000100A,
	UWB_Err_DRIV_SPI_UCI_Boot_Wakeup_Res_Incorrec								            = 0x0000100B,




	//0x0000 3000 - 0x0000 3FFF uwb_commu module Error
	UWB_Err_COMMU_Send_Failed													            = 0x00003001,
	//UWB_Err_COMMU_CMD_STATUS_Undefined										            = 0x00003002,
	UWB_Err_COMMU_Incorrect_Status												            = 0x00003003,
	UWB_Err_COMMU_Recv_Time_Out													            = 0x00003004,
	UWB_Err_COMMU_SendLine_Is_Bussy												            = 0x00003005,
	UWB_Err_COMMU_Send_Cancel													            = 0x00003006,
	UWB_Err_COMMU_Recv_Cancel													            = 0x00003007,
	UWB_Err_COMMU_Send_Time_Out													            = 0x00003008,
	UWB_Err_COMMU_Default_Unkonw_Error											            = 0x00003009,
	UWB_Err_COMMU_Unkonw_Error													            = 0x0000300A,
	UWB_Err_COMMU_Send_Lens_Illegal												            = 0x0000300B,
	UWB_Err_COMMU_Rest_Failed													            = 0x0000300C,
	UWB_Err_COMMU_Get_Recv_Signal_Failed										            = 0x0000300D,
	UWB_Err_COMMU_The_Line_Still_Bussy											            = 0x0000300E,
	UWB_Err_COMMU_Recv_line_Always_Bussy										            = 0x0000300F,
	UWB_Err_COMMU_Recv_Data_Only_Header											            = 0x00003010,
	UWB_Err_COMMU_Recv_Wait_For_Time_Out										            = 0x00003011,
	UWB_Err_COMMU_Send_Abort_Failed												            = 0x00003012,
	UWB_Err_COMMU_Send_Line_Always_Bussy										            = 0x00003013,
	UWB_Err_COMMU_Recv_Data_Body_Failed											            = 0x00003014,


	//0x0000 E000 - 0x0000 EFFF	UCI Software Error
	UWB_Err_UCI_Status_Ok                                                                   = 0x0000E000,//Success.
	UWB_Err_UCI_Status_Rejected                                                             = 0x0000E001,//Intended operation is not supported in the current state.
	UWB_Err_UCI_Status_Failed                                                               = 0x0000E002,//Intended operation failed to complete.
	UWB_Err_UCI_Status_Syntax_Error                                                         = 0x0000E003,//UCI packet structure is not per spec.
	UWB_Err_UCI_Status_Invalid_Param                                                        = 0x0000E004,//Config ID is correct, and value is not specified
	UWB_Err_UCI_Status_Invalid_Range                                                        = 0x0000E005,//Config ID is correct, and value is not in proper range
	UWB_Err_UCI_Status_Invalid_Message_Size                                                 = 0x0000E006,//UCI packet payload size is not as per spec.
	UWB_Err_UCI_Status_Unknown_GID                                                          = 0x0000E007,//UCI Group ID is not per spec.
	UWB_Err_UCI_Status_Unknown_OID                                                          = 0x0000E008,//UCI Opcode ID is not per spec.
	UWB_Err_UCI_Status_Read_Only                                                            = 0x0000E009,//Config ID is read-only.
	UWB_Err_UCI_Status_Command_Retry                                                        = 0x0000E00A,//UWBS request retransmission from AP

	UWB_Err_UCI_Status_Error_Session_Not_Exist                                              = 0x0000E011,//Session does not exist (is not created)
	UWB_Err_UCI_Status_Error_Session_Duplicate                                              = 0x0000E012,//Session exists (is already created)
	UWB_Err_UCI_Status_Error_Session_Active                                                 = 0x0000E013,//Session is active.
	UWB_Err_UCI_Status_Error_Max_Sessions_Exceeded                                          = 0x0000E014,//Max. number of sessions already created.
	UWB_Err_UCI_Status_Error_Session_Not_Configured                                         = 0x0000E015,//Session is not configured with required app configurations
	UWB_Err_UCI_Status_Error_Active_Sessions_Ongoing                                        = 0x0000E016,//Sessions are actively running in UWBS
	UWB_Err_UCI_Status_Slot_Len_Not_Supported                                               = 0x0000E01A,//Slot length is not supported in the requested configuration
	UWB_Err_UCI_Status_Invalid_Slot_Per_rr                                                  = 0x0000E01B,//Number of slots is invalid in the requested configuration
	UWB_Err_UCI_Status_Invalid_Sts_Idx                                                      = 0x0000E01C,//STS index is not valid.
	UWB_Err_UCI_Status_To_Send													            = 0x0000E01D,
	UWB_Err_UCI_Status_To_Recv													            = 0x0000E01E,
	UWB_Err_UCI_Status_To_NTF													            = 0x0000E01F,

	UWB_Err_UCI_Device_Status_Ok												            = 0x0000E100,
	UWB_Err_UCI_Device_Status_NTF_RangingSession_Is_Ready						            = 0x0000E101,
	UWB_Err_UCI_Device_Status_NTF_RangingSession_Is_Bussy						            = 0x0000E102,
	UWB_Err_UCI_Device_Status_NTF_Device_IsNot_29D5								            = 0x0000E1E3,
	UWB_Err_UCI_Device_Status_NTF_Device_Is_Test_Mode							            = 0x0000E1E4,
	UWB_Err_UCI_Device_Status_NTF_Reboot_By_SoftWare							            = 0x0000E1FB,
	UWB_Err_UCI_Device_Status_NTF_Reboot_From_HPD								            = 0x0000E1FC,
	UWB_Err_UCI_Device_Status_NTF_Reboot_From_RST_Pin							            = 0x0000E1FD,
	UWB_Err_UCI_Device_Status_NTF_Reboot_Power_Fault							            = 0x0000E1FE,
	UWB_Err_UCI_Device_Status_NTF_Status_Error									            = 0x0000E1FF,
	UWB_Err_UCI_Device_Status_NTF_Status_RFU_Error								            = 0x0000E2FF,

	UWB_Err_UCI_Cmd_Analysis_Failed												            = 0x0000E200,
	UWB_Err_UCI_Res_Analysis_Failed												            = 0x0000E201,
	UWB_Err_UCI_Recv_Success_NTF												            = 0x0000E202,
	UWB_Err_UCI_Recv_Success_RES												            = 0x0000E203,
	UWB_Err_UCI_Rest_Failed														            = 0x0000E204,
	UWB_Err_Status_NTF_UWBS_Is_BussyActive										            = 0x0000E205,

	UWB_Err_Session_Status_NTF_Reason_Code_STATE_CHANGE_WITH_SESSION_MANAGEMENT_COMMANDS	= 0x0000E207,
	UWB_Err_Session_Status_NTF_Reason_Code_MAX_RANGING_ROUND_RETRY_COUNT_REACHED			= 0x0000E208,
	UWB_Err_Session_Status_NTF_Reason_Code_MAX_RANGING_BLOCKS_REACHED						= 0x0000E209,
	UWB_Err_Session_Status_NTF_Reason_Code_URSK_EXPIRED_URSK								= 0x0000E20A,
	UWB_Err_Session_Status_NTF_Reason_Code_TERMINATION_ON_MAX_STS_MAX_STS					= 0x0000E20B,
	UWB_Err_Session_Status_NTF_Reason_Code_RFU												= 0x0000E20C,
	UWB_Err_Ranging_NTF_Status_Transaction_Overflow											= 0x0000E20D,
	UWB_Err_Ranging_NTF_Status_Transaction_invalid											= 0x0000E20E,
	UWB_Err_Ranging_NTF_Status_Transaction_Frame_Incorrect									= 0x0000E20F,
	UWB_Err_Ranging_NTF_Status_Loss_of_ranging_control_information							= 0x0000E210,
	UWB_Err_Session_Status_Unsupported_Session_Mode											= 0x0000E211,
	UWB_Err_Session_Status_SessionID_Incorrect												= 0x0000E212,
	UWB_Err_Session_GID_UnSupported															= 0x0000E213,
	UWB_Err_Session_OID_UnSupported															= 0x0000E214,
	UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect										= 0x0000E215,
	UWB_Err_RANGING_SESSION_CONFIG_GROUP_OID_Incorrect										= 0x0000E216,











	//0x0000 F000 - 0x0000 FFFF RCI Software Error

}E_UWBErrCode;
/* ---------------------------------------------------------------------------------------------
 * UWB Driver layer
 * ---------------------------------------------------------------------------------------------*/

 // Hardware timer
typedef void 		(*fp_Timer_Handler_t)	(void* ptr_handler_param);
typedef uint32_t 	(*fp_Create_Timer_t)	(uint8_t timer_type, uint8_t timer_id, fp_Timer_Handler_t ptr_timer_handler, void* ptr_handler_param);
typedef uint32_t 	(*fp_Start_Timer_t)		(uint8_t timer_id, uint64_t in_nanosecond);
typedef void 		(*fp_Stop_Timer_t)		(uint8_t timer_id);
typedef void 		(*fp_Destory_Timer_t) 	(void);
typedef void 		(*fp_Delay_t)			(uint32_t ns);
typedef void		(*fp_OSDelayMS_t)			(uint32_t ms);

typedef struct struct_Timer_Tools_t
{
	fp_Timer_Handler_t	  			fpHandler;							  		/*< 回调函数 */
	fp_Create_Timer_t           	fpCreateTimer;                            	/**< 创建timer  */
	fp_Start_Timer_t            	fpStartTimer;                             	/**< 启动timer */
	fp_Stop_Timer_t             	fpStopTimer;                              	/**< 停止timer */
	fp_Destory_Timer_t			 	fpDestoryTimer;								/*< 销毁timer */
	void*   					 	fpHandlerParam;								/*< 回调函数参数 */
	fp_Delay_t						fpDelay;									/*< 使用定时器delay*/
	fp_OSDelayMS_t					fpOSDelay;									/*< 系统延时 >*/
}ST_TimerTools;

// SPI Communiucation
typedef uint32_t (*fp_UWB_Cmmu_Init_t)		(void);
typedef uint32_t (*fp_UWB_Cmmu_Reset_t)		(ST_TimerTools* pst_timer_tools, uint8_t* recv_buff_ptr, uint16_t* length);
typedef uint32_t (*fp_UWB_Cmmu_Send_t)		(ST_TimerTools* pst_timer_tools, const uint8_t* send_buff_ptr, const uint16_t length);
typedef uint32_t (*fp_UWB_Cmmu_Recv_t)		(ST_TimerTools* pst_timer_tools, uint8_t* recv_buff_ptr, uint16_t* length);
typedef uint32_t (*fp_UWB_Cmmu_Sleep_t)		(void);
typedef uint32_t (*fp_UWB_Cmmu_Wake_up_t)	(void);
typedef uint32_t (*fp_UWB_Cmmu_Deinit_t)	(void);
typedef bool 	 (*fp_UWB_Cmmu_Isntfcomin_t)(void);
typedef bool	 (*fp_UWB_Cmmu_IsTransmitComplate_t)(void);

typedef struct struct_UWB_Commu_Attr_t
{
	fp_UWB_Cmmu_Init_t					fpCmmuInit;            		/**< init and hard reset */
	fp_UWB_Cmmu_Reset_t					fpCmmuReset;					/**< soft reset */
	fp_UWB_Cmmu_Send_t					fpCmmuSend;					/**< send */
	fp_UWB_Cmmu_Recv_t					fpCmmuRecv;					/**< recv */
	fp_UWB_Cmmu_Sleep_t					fpCmmuSleep;					/**< sleep */
	fp_UWB_Cmmu_Wake_up_t				fpCmmuWakeup;					/**< wakup */
	fp_UWB_Cmmu_Deinit_t				fpCmmuDeinit;					/**< deinit */
	fp_UWB_Cmmu_Isntfcomin_t 			fpCmmuIsntfcomin;			/**< check recv line status */
	fp_UWB_Cmmu_IsTransmitComplate_t		fpCmmuIsTransmitComplate;
}ST_UWBCommuAttr;

/* ---------------------------------------------------------------------------------------------
 * UCI Frame Layer
 * ---------------------------------------------------------------------------------------------*/

typedef struct struct_NXP_UCI_Dat_t
{
	bool 		bIsTheLast;
	uint8_t 	u8Tot;
	uint8_t*	pbufDatHeader;
	uint8_t*	pbufDatBody;
	uint16_t 	u16DatBodyLens;
}ST_NXPUCIDat;

//S32144K Config Flash address at last 32K
//#define CONFIG_FILE_ADDRESS_BEGING 0x00077FFF

//-----------------------------------------NXP_UCI_FRAME BEGING

#define UCISW_HEADER_LENGTH          				(5u)
#define	UCISW_HEADER_LENGTH_OCTET_OFFSET			(4u)
#define UCISW_MAX_PAYLOAD_LENGTH     				(255u)
#define UCISW_MAX_FRAME_LENGTH						(UCISW_HEADER_LENGTH + UCISW_MAX_PAYLOAD_LENGTH)
#define UCI_CMD_PROCESS_MAX_TIME_BY_MILLISECOND		(250UL)//(10MS)

typedef enum enum_UCI_Frame_State_t
{
	UciFrameState_Init								= 0x00u,
	UciFrameState_Connecting						= 0x01u,
	UciFrameState_Connected							= 0x02u,
	UciFrameState_NotConnected						= 0x03u,
	UciFrameState_TransmitCommand					= 0x04u,
	UciFrameState_ReceiveResponse					= 0x05u,
	UciFrameState_ReceiveNotify						= 0x06u,
	UciFrameState_NotifyPending						= 0x07u,
	//UciFrameState_NotifyOut						= 0x08u,
	UciFrameState_Error								= 0xFFu,
}E_UCIFrameState;

typedef enum enum_UWB_Control_Message_Index_t
{
	//recv CCC or Custom message is this idx ONLY can be.
	UWB_MSG_Is_Null										= 0x00,

	//YQ CCC can msg
	UWB_Anchor_WakeUp_RQ 	 							= 0x01,
	UWB_Anchor_WakeUp_RS 	 							= 0x02,
	UWB_Hard_Reset_RQ									= 0x03,
	UWB_Hard_Reset_RS									= 0x04,
	UWB_Timer_Sync_RQ		 							= 0x05,
	UWB_Timer_Sync_RS		 							= 0x06,
	UWB_Ranging_Session_Init_RQ 						= 0x07,
	UWB_Ranging_Session_Init_RS 						= 0x08,
	UWB_Ranging_Session_App_Config_RQ					= 0x09,
	UWB_Ranging_Session_App_Config_RS					= 0x0A,
	UWB_Ranging_Session_Start_RQ 						= 0x0B,
	UWB_Ranging_Session_Start_RS 						= 0x0C,
	UWB_Ranging_Session_Suspend_RQ						= 0x0D,
	UWB_Ranging_Session_Suspend_RS			 			= 0x0E,
	UWB_Ranging_Session_Recover_RQ			 			= 0x0F,
	UWB_Ranging_Session_Recover_RS			 			= 0x10,
	UWB_Ranging_Session_Stop_RQ			 				= 0x11,
	UWB_Ranging_Session_Stop_RS			 				= 0x12,
	UWB_Ranging_Session_Deinit_RQ						= 0x13,
	UWB_Ranging_Session_Deinit_RS						= 0x14,
	UWB_Ranging_Session_GetSeionCnt_RQ					= 0x15,
	UWB_Ranging_Session_GetSeionCnt_RS					= 0x16,
	UWB_Ranging_Session_GetRangCnt_RQ					= 0x17,
	UWB_Ranging_Session_GetRangCnt_RS					= 0x18,
	UBW_Deviec_Status_NTF								= 0x19,
	UWB_Session_Status_NTF								= 0x1A,
	UWB_Ranging_Result_Notice 							= 0xFE,
	UWB_Core_Get_Device_Info_RQ							= 0x1B,
	UWB_Core_Get_Device_Info_RS							= 0x1C,
	//UWB_Debug_Process									= 0xFF,

	//UQ Custom msg
	UWB_Core_Config_Setting_RQ							= 0x82,
	UWB_Core_Config_Setting_RS							= 0x83,
	UWB_Core_Setting_Save_RQ							= 0x84,
	UWB_Core_Setting_Save_RS							= 0x85,
	UWB_Core_Setting_Load_RQ							= 0x86,
	UWB_Core_Setting_Load_RS							= 0x87,
	UWB_APP_Setting_Save_RQ								= 0x88,
	UWB_APP_Setting_Save_RS								= 0x89,
	UWB_Sys_Notice_NTF									= 0x8A,

	//UWB_Module_WakeUp_NTF								= 0x0100,

}E_UWBControlMessageIndex;

typedef enum enum_Module_Work_Status_t
{
	Module_Stat_Ready		= 0,
	Module_Stat_Active		= 1
}E_ModuleWorkStatus;

typedef enum enum_UCI_Session_Status_t
{
	Session_Stat_Init		= 0,
	Session_Stat_Idle		= 1,
	Session_Stat_Aciv		= 2,
	Session_Stat_Deinit		= 3,
	Session_Stat_RFU		= 4,
	Session_Stat_Err		= 0xff
}E_UCISessionStatus;

typedef enum enum_UWBFilterType_t
{
	UWBFT_MathematicalAverage = 0,
	UWBFT_KalmanFilter		= 1,
}E_UWBFilterType;

typedef struct strcut_UCI_Ranging_Result_t
{//后期拿掉
 // single ranging result ntf 23 bytes
 //2 buffer
	uint8_t		bufResultXSlot1[230];
	uint8_t		bufResultXSlot2[230];
	bool 		bIsResultXSlot1Full;
	uint8_t*	pbufResult;
	uint8_t		u8ResultCnt;
}ST_UCIRangingResult;




typedef struct struct_Ranging_Data_t
{
	uint16_t bufDistQueue[DEF_RANGING_RESULT_BUFFER_SIZE];
	uint8_t  u8DisQueCurrIDX;
}ST_Ranging_Data;


typedef struct struct_UCI_Ranging_Session_t
{
	bool						bIsUCISessionInitDone;
	bool						bIsUCISessionAPPConfigDone;
	bool						bIsUCIRangIsStarted;

	uint32_t					u32CurrSessionID;

	uint32_t					u32STSIndex;
	uint32_t					u32CurrentRangingCnt;
	uint8_t						u8CurrentSessionCnt;
	uint8_t						u8UCIRangSessionMode;
	uint16_t					u8CurrentRRIndex;
	uint16_t					u16NextRRIndex;
	uint8_t						bufCCMTag[8];
	E_UWBFilterType				eFilterType;
	uint16_t					u16FilterPayload;
	uint16_t					u16FilterCnt;
	ST_UCIRangingResult			stRangingResult;

	E_UCISessionStatus			eSesionStat;
#if defined(UWB_INITIATOR)//Initiator only
	uint8_t						u8DeviceStatus;
	uint32_t					u32ResponderBitMap;
#elif defined(UWB_RESPONDER)//RESPONDER only
	uint8_t						u8AnchorStatus;
	uint16_t					u16CurrentDistance;
	uint8_t						u8UncertaintyAnchor;
	uint8_t						u8UncertaintyInitiator;
#else
	uint8_t						"You need make a chooes"
#endif

}ST_UCIRangingSession;

typedef struct st_UWB_Caps_t
{
	uint8_t     			u8SlotBitMask;
	uint32_t     			u32SyncCodeBitMask;
	uint8_t     			u8HopingConfigBitMask;
	uint8_t*				pbufHopingKey;                  /*<! 当不使用带AES跳频时，此指针为空*/
	uint8_t     			u8ChannelBitMaskMap;

	uint8_t     			u8PulseShapeCombolens;				/*<! CCC 规范 21.5.3 PulseShape*/
	uint8_t					bufCCCShapeCombo[4];
	uint8_t					u8CCCProtocolVersionLens;
	uint8_t					bufProtocolVersion[4];				/*<! CCC 规范 21.4 UWB Frame Elements Table 21-1 version*/
																//2*2 version 0 is 0x00,0x00, version 1 is 0x00, 0x01
	uint8_t					u8CCCConfigIDLens;
	uint8_t					bufCCCConfigID[4];
	//uint32_t    			u32CCCProtocolVersion;
	//uint16_t    			u16CCCConfigMO;					/*<! CCC 规范 21.4 UWB Frame Elements Table 21-1 Mandatory/ Optional*/
	uint8_t					u8URSKMode;
	uint8_t*				pbufURSK;
}ST_UWBCaps;

typedef struct st_uwb_rang_protocol_t
{
	uint8_t     				u8DevType;                       /*<! 00: 响应者 | 01：发起者*/
	uint8_t     				u8DevID;						/*<! 响应者ID*/
	ST_UWBCaps					stCCCCaps;						/*<! ccc 涉及到的测距能力参数值*/
}ST_UWBProtocol;

typedef struct struct_UCI_Frame_State_t
{
	bool 							bIsInCmd;
	bool							bIsUWBGoingToSleep;
	bool							bIsUWBConnected;
	bool							bIsResponderUWBDeviceType;
	bool							bIsLastMSG;
	bool							bIsNTFCache;

	E_UWBControlMessageIndex		eMSGIdx;
	E_UWBControlMessageIndex		eNTFIdx;
	E_UWBErrCode					eOpCode;
	E_ModuleWorkStatus				eWorkStat;	//0 Ready | 1 Active
	E_UCIFrameState					eState;

	uint8_t							u8UWBSessionCnt;
	uint8_t							u8UWBLocalcIndex;
	uint8_t							u8data;
	uint16_t 						u16lens;

	ST_UCIRangingSession			stSession;
	ST_TimerTools					stTimerTools;
	ST_UWBCommuAttr					stUWBCommu;
	
	ST_NXPUCIDat 					stUCISend;
	ST_NXPUCIDat 					stUCIRecv;
	ST_UWBProtocol					stProtocol;
	ST_Ranging_Data					stRaningDat;
	uint8_t							u8RSTCnt;
#ifdef DEBUG_FEATRUE_FAKE_STS
	uint32_t						FakeSTS;
#endif
}ST_UCIFrameState;


//-----------------------------------------NXP_UCI_FRAME END




typedef struct st_CustCmd_t
{
	bool					bIsNeedToSend;
	bool					bIshaveNTF;
	uint8_t*                pbufCmd;					        /*<! cmd buffer 指针 */
	uint16_t                u16Cmdlen;						    /*<! cmd length */
	uint32_t				u32ResCode;						    /*<! 命令执行结果 */
	uint8_t					bufNTF[64];
}ST_CustCmd;



/***
* @brief about 3C
*/

#define TLSIZ_11		2


typedef struct struct_LoopMessage_t
{
	bool		bIsCANMSGInPut;
	uint8_t		bufCMDIn[512];

	bool		bIsCANMSGOutPut;
	uint8_t		bufCMDOut[512];

	bool		bIsNTFMSGOutPut;
	uint8_t		bufNTFOut[512];
}ST_LoopMessage;

typedef struct struct_UQUWBMsgAnchorWakeupRQ_t
{
	bool		bIsSuccess;
	uint8_t*	fpCmd;
	uint8_t		bufRest[5];//{0x20, 0x00, 0x00, 0x01, 0x00};
}ST_UQCMDSeqAnchorWakeupRQ;

typedef struct struct_UQUWBMsgRangingStart_t
{
	//uint8_t CmdCNT;
	bool		bIsSuccess;
	uint8_t*	fpCmd;
	uint8_t		bufCMDSequence[4];
	uint8_t		bufSetCoreConfig		[4 + 61];
	uint8_t		bufInitSession			[4 + 5];//{0x21,0x00,0x00,0x05,0x03,0x00,0x00,0x00,0xA0};
	uint8_t		bufAPPSessionConfig		[4 + 298];//0x21,0x03,0xxx,0xxx,
	uint8_t		bufStartRanging			[4 + 4];//{ 0x22,0x00,0x00,0x04,0x00,0x00,0x00,0x00 };
}ST_UQCMDSeqRangingStart;

typedef struct struct_UQUWBMsgRangingSTOP_t
{
	bool		bIsSuccess;
	bool		bIsTempStop;
	uint8_t		bufStopSession[9];
	uint8_t		bufDeInitSession[9];
}ST_UQCMDSeqRangingStop;

typedef struct struct_UQUWBMsgRangingRecover_t
{
	bool		bIsSuccess;
	bool		bIsConfig;
	uint8_t		bufRecoverSession[4 + 5];
	uint8_t		bufAPPSessionConfig[4 + 298];//0x21,0x03,0xxx,0xxx,
}ST_UQCMDSeqRangingRecover;

// ----------------------------------------------------------------------------------------
/***
*  @brief SDK Interface
*/
typedef enum enumRangingOPType_t
{
	UWBRangingOPType_Start = 0,
	UWBRangingOPType_Suspend = 1,
	UWBRangingOPType_Recover = 2,
	UWBRangingOPType_RecoverAndConfig = 3,
	UWBRangingOPType_Stop = 4
}E_RangingOPType;


#define DEF_UWB_CAN_BUF_SIZE 64
#define UWB_RANGING_FREQUENCY 10
typedef struct structUWBSource_t
{
	/* |InValid_Flag|RFU|Data|*/
	bool					bIsCanBuffLink;
	uint8_t                 bufCANIn_1[DEF_UWB_CAN_BUF_SIZE];
	uint8_t                 bufCANIn_2[DEF_UWB_CAN_BUF_SIZE];
	uint8_t					bufCANOut[DEF_UWB_CAN_BUF_SIZE];
	//uint16_t				bufDistan[UWB_RANGING_FREQUENCY];
	uint16_t				u16LastDistan;
	//uint16_t				u16LastFFCnt;
	ST_UCIFrameState		stUCIState;
	uint16_t				u16HistoryPos;
	uint16_t				u16HistoryNeg;
}ST_UWBSource;


typedef int (*fp_UQ_MSGSend_t)		 		(E_UWBControlMessageIndex type, uint8_t* p_msg, uint16_t* p_msglens);
typedef int (*fp_UQ_Device_Init_t)			(fp_UQ_MSGSend_t fp_send_msg);
typedef int (*fp_UQ_Device_Reset_t)			(fp_UQ_MSGSend_t fp_send_msg);
typedef int (*fp_UQ_Get_Caps_t)				(ST_UWBProtocol* pst_protocol, fp_UQ_MSGSend_t fp_send_msg);
typedef int (*fp_UQ_Set_Caps_t)				(ST_UWBProtocol* pst_protocol, fp_UQ_MSGSend_t fp_send_msg);
typedef int (*fp_UQ_Anchor_Wakup_t)			(uint8_t* cmdin, uint16_t cmdin_lens, fp_UQ_MSGSend_t fp_send_msg);
typedef int (*fp_UQ_Time_Sync_t)			(uint8_t* cmdin, uint16_t cmdin_lens, fp_UQ_MSGSend_t fp_send_msg);
typedef int (*fp_UQ_RangingSessionSetup_t) 	(uint8_t* cmdin, uint16_t cmdin_lens);
typedef int (*fp_UQ_RangingCtrl_t) 			(E_RangingOPType type, uint8_t* cmdin, uint16_t cmdin_lens, fp_UQ_MSGSend_t fp_send_msg);
typedef int (*fp_UQ_RangingResult_t)		(fp_UQ_MSGSend_t fp_send_msg);
typedef int (*fp_UQ_RangingNTFCache_t)		(fp_UQ_MSGSend_t fp_send_msg);
typedef int	(*fp_UQ_ConfigWrite_t)			(uint8_t config_index, uint8_t* p_buf, const uint16_t bufLens);
typedef int (*fp_UQ_ConfigRead_t)			(uint8_t config_index, uint8_t* p_buf, uint16_t* p_buflens);
typedef int (*fp_UQ_CustCMD_t) 				(ST_CustCmd* pst_custcmd, fp_UQ_MSGSend_t fp_send_msg);
typedef void (*fp_UQ_SetNTFCacheFlag_t)		(ST_UWBSource* pst_uwb_source);
typedef void (*fp_UQ_Periodic_Cycle_t)		(ST_UWBSource* pst_uwb_source);

typedef enum enumUQResponseCode_t
{
	UQRESC_Success_0								= 0,
	UQRESC_URSK_UnNegotiated						= 1,
	UQRESC_RangingSessionNotMatch					= 2,
	UQRESC_AnchoIsOffLine							= 3,
	UQRESC_STSIsMaxValue							= 4,
	UQRESC_CommunicationException					= 5,
	UQRESC_WakeupFailed								= 6,
	UQRESC_RangingStartFailed						= 7,
	UQRESC_RangingStopFailed						= 8,
	UQRESC_ResponseIsTimeOut						= 0x21,
	UQRESC_ResponseDataIsIncorrect					= 0x22,

}E_UQRESCode;

//Debug CODE
// ----------------------------------------------------------------------------------------



extern void DEBUGE_PRINT_RESCODE(E_UWBErrCode resc);
// ----------------------------------------------------------------------------------------



#endif /* SOURCES_UWB_COMMON_DEF_H_ */


#if 0
abandoned code

//extern int API_uwb_responder_module_init(st_uwb_rang_ctrl_t* stptr);
//extern void LOG_INIT(void);
//extern void LOG_L_S(CCC_MD,char* c);
//extern void LOG_OUT_HEX(uint8_t* string, uint8_t* Hexinput, uint16_t intputlen);

//uint8_t UWB_Anchor_SN[10] = {0x5A,0xA5,0xA5,0x5A,0x00,0x00,0x00,0x00,0x00,0x00};
//uint8_t UWB_LOC_IND = 0xFF;
//bool	CAN_MSG_BUFFER_IS_EMPTY = false;
//uint8_t CAN_MSG_BUFFER[1 + 64] = { 0 };
//-----------------------------------------NXP_RCI_FRAME BEGING

//#define RCISW_HEADER_LENGTH						(4u)
//#define RCISW_MAX_PAYLOAD_LENGTH					(250u)
//#define RCISW_CRC_LENGTH							(2u)
//#define RCISW_MAX_FRAME_LENGTH					(RCISW_HEADER_LENGTH + RCISW_MAX_PAYLOAD_LENGTH + RCISW_CRC_LENGTH)
//-----------------------------------------NXP_RCI_FRAME END
#endif
