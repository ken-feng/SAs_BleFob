/*
 * NXP_UCI_CMD.h
 *
 *  Created on: 2022年7月11日
 *      Author: JohnSong
 */

#ifndef SOURCES_NXP_S32K_APPLICATION_NXP_UCI_CMD_H_
#define SOURCES_NXP_S32K_APPLICATION_NXP_UCI_CMD_H_

#include "../uwb_common_def.h"
//

//define uci msg frist byte
/*------------------MIT----------------------------------------------*/
#define NXP_UCI_MSG_MIT_RFU0                               (0x00U)
#define NXP_UCI_MSG_MIT_CTRL_CMD                           (0x01U)
#define NXP_UCI_MSG_MIT_CTRL_RES                           (0x02U)
#define NXP_UCI_MSG_MIT_CTRL_NOTIFICATION                  (0x03U)
/*------------------PBF----------------------------------------------*/
#define NXP_UCI_MSG_PBF_NOLINK                             (0x00U)
#define NXP_UCI_MSG_PBF_LINK                               (0x01U)
/*------------------RFU1---------------------------------------------*/
#define NXP_UCI_MSG_LENS_IS_1BYTE						   (0x00U)
#define NXP_UCI_MSG_LENS_IS_2BYTE 						   (0x01U)
/*------------------GID----------------------------------------------*/
#define NXP_UCI_MSG_GID_UCI_CORE_GROUP                     (0x00U)
#define NXP_UCI_MSG_GID_UWB_SESS_CONFIG_GROUP              (0x01U)
#define NXP_UCI_MSG_GID_UWB_RANGING_SESSION_CTRL_GROUP     (0x02U)
#define NXP_UCI_MSG_TEST_GROUP                        	   (0x0DU)
#define NXP_UCI_MSG_PROPRIETARY_GROUP_0                    (0x0EU)
#define NXP_UCI_MSG_PROPRIETARY_GROUP_1                    (0x0FU)

#define NXP_UCI_MSG_HEADER_OCT0_GID_UCI_CORE_GROUP(MIT_TYPE)                    (MIT_TYPE<<5 | NXP_UCI_MSG_GID_UCI_CORE_GROUP)
#define NXP_UCI_MSG_HEADER_OCT0_GID_UWB_SESS_CONFIG_GROUP(MIT_TYPE)             (MIT_TYPE<<5 | NXP_UCI_MSG_GID_UWB_SESS_CONFIG_GROUP)
#define NXP_UCI_MSG_HEADER_OCT0_GID_UWB_RANGING_SESSION_CTRL_GROUP(MIT_TYPE)    (MIT_TYPE<<5 | NXP_UCI_MSG_GID_UWB_RANGING_SESSION_CTRL_GROUP)
#define NXP_UCI_MSG_HEADER_OCT0_TEST_GROUP(MIT_TYPE)                            (MIT_TYPE<<5 | NXP20D5D_UCI_MSG_TEST_GROUP)
#define NXP_UCI_MSG_HEADER_OCT0_PROPRIETARY_GROUP_0(MIT_TYPE)                   (MIT_TYPE<<5 | NXP_UCI_MSG_PROPRIETARY_GROUP_0)
#define NXP_UCI_MSG_HEADER_OCT0_PROPRIETARY_GROUP_1(MIT_TYPE)                   (MIT_TYPE<<5 | NXP_UCI_MSG_PROPRIETARY_GROUP_1)



//define uci msg second byte
/*------------------RFU----------------------------------------------*/
#define NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT                   (0x00U)
/*----------------- OID UCI Core Group ------------------------------*/
#define NXP_UCI_MSG_OID_CORE_DEVICE_STATUS_NTF                                  (0x01U)
#define NXP_UCI_MSG_OID_CORE_GENERIC_ERROR_NTF                                  (0x07U)
#define NXP_UCI_MSG_OID_CORE_RFU                                                (0x06U)

#define NXP_UCI_MSG_OID_CORE_DEVICE_RESET_CMD                                   (0x00U)
#define NXP_UCI_MSG_OID_CORE_GET_DEVICE_INFO_CMD                                (0x02U)
#define NXP_UCI_MSG_OID_CORE_GET_CAPS_INFO_CMD                                  (0x03U)
#define NXP_UCI_MSG_OID_CORE_SET_CONFIG_CMD                                     (0x04U)
#define NXP_UCI_MSG_OID_CORE_GET_CONFIG_CMD                                     (0x05U)

#define NXP_UCI_MSG_OID_CORE_DEVICE_RESET_RES                                   (0x00U)
#define NXP_UCI_MSG_OID_CORE_GET_DEVICE_INFO_RES                                (0x02U)
#define NXP_UCI_MSG_OID_CORE_GET_CAPS_INFO_RES                                  (0x03U)
#define NXP_UCI_MSG_OID_CORE_SET_CONFIG_RES                                     (0x04U)
#define NXP_UCI_MSG_OID_CORE_GET_CONFIG_RES                                     (0x05U)

/*----------------- OID UWB Session Config Group --------------------*/
#define NXP_UCI_MSG_OID_SESSION_STATUS_NTF                                      (0x02U)

#define NXP_UCI_MSG_OID_SESSION_INIT_CMD                                        (0x00U)
#define NXP_UCI_MSG_OID_SESSION_DEINIT_CMD                                      (0x01U)
#define NXP_UCI_MSG_OID_SESSION_SET_APP_CONFIG_CMD                              (0x03U)
#define NXP_UCI_MSG_OID_SESSION_GET_APP_CONFIG_CMD                              (0x04U)
#define NXP_UCI_MSG_OID_SESSION_GET_COUNT_CMD                                   (0x05U)
#define NXP_UCI_MSG_OID_SESSION_GET_STATE_CMD                                   (0x06U)
#define NXP_UCI_MSG_OID_SESSION_GET_POSSIBLE_RAN_MULTIPLIER_VALUE_CMD           (0x20U)

#define NXP_UCI_MSG_OID_SESSION_INIT_RES                                        (0x00U)
#define NXP_UCI_MSG_OID_SESSION_DEINIT_RES                                      (0x01U)
#define NXP_UCI_MSG_OID_SESSION_SET_APP_CONFIG_RES                              (0x03U)
#define NXP_UCI_MSG_OID_SESSION_GET_APP_CONFIG_RES                              (0x04U)
#define NXP_UCI_MSG_OID_SESSION_GET_COUNT_RES                                   (0x05U)
#define NXP_UCI_MSG_OID_SESSION_GET_STATE_RES                                   (0x06U)
#define NXP_UCI_MSG_OID_SESSION_GET_POSSIBLE_RAN_MULTIPLIER_VALUE_RES           (0x20U)

/*----------------- OID UWB Ranging Session Control Group ------------*/
#define NXP_UCI_MSG_OID_RANGE_CCC_DATA_NTF                                      (0x20U)

#define NXP_UCI_MSG_OID_RANGE_START_CMD                                         (0x00U)
#define NXP_UCI_MSG_OID_RANGE_STOP_CMD                                          (0x01U)
#define NXP_UCI_MSG_OID_RANGE_GET_RANGING_COUNT_CMD                             (0x03U)
#define NXP_UCI_MSG_OID_RANGE_RESUME_CMD                                        (0x21U)

#define NXP_UCI_MSG_OID_RANGE_START_RES                                         (0x00U)
#define NXP_UCI_MSG_OID_RANGE_STOP_RES                                          (0x01U)
#define NXP_UCI_MSG_OID_RANGE_GET_RANGING_COUNT_RES                             (0x03U)
#define NXP_UCI_MSG_OID_RANGE_RESUME_RES                                        (0x21U)

/*----------------- OID UWB Proprietary Group ------------*/
#define NXP_UCI_MSG_OID_PROPRIETARY_LOG_NTF                                     (0x00U)//	带有日志信息的通知
#define NXP_UCI_MSG_OID_PROPRIETARY_TEST_STOP_NTF                               (0x21U)//	用于停止测试模式的命令
#define NXP_UCI_MSG_OID_PROPRIETARY_TEST_LOOPBACK_NTF                           (0x25U)//	环回通知
#define NXP_UCI_MSG_OID_PROPRIETARY_SET_TRIM_VALUES_NTF                         (0x26U)//	用于配置修剪值的命令
#define NXP_UCI_MSG_OID_PROPRIETARY_USER_DEFINED_RANGE_DATA_NTF                 (0x2CU)//	用户定义会话 0xE1 的测距数据通知

#define NXP_UCI_MSG_OID_PROPRIETARY_RADIO_CONFIG_DOWNLOAD_CMD                   (0x11U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_ACTIVATE_SWUP_CMD                           (0x12U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_TEST_START_CMD                              (0x20U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_TEST_STOP_CMD                               (0x21U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_QUERY_UWB_TIMESTAMP_CMD                     (0x23U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_DEVICE_SUSPEND_CMD                          (0x24U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_SET_TRIM_VALUES_CMD                         (0x26U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_GET_ALL_UWB_SESSIONS_CMD                    (0x27U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_GET_TRIM_VALUES_CMD                         (0x28U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_STORE_PROTECTION_KEY_CMD                    (0x29U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_SET_EPOCH_ID_CMD                            (0x2AU)//
#define NXP_UCI_MSG_OID_PROPRIETARY_TBD                                         (0x2BU)//

#define NXP_UCI_MSG_OID_PROPRIETARY_RADIO_CONFIG_DOWNLOAD_RSP                   (0x11U)//	用于下载无线电配置的命令
#define NXP_UCI_MSG_OID_PROPRIETARY_ACTIVATE_SWUP_RSP                           (0x12U)//	软件更新程序激活
#define NXP_UCI_MSG_OID_PROPRIETARY_TEST_START_RSP                              (0x20U)//	用于启动测试模式的命令
#define NXP_UCI_MSG_OID_PROPRIETARY_TEST_STOP_RSP                               (0x21U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_QUERY_UWB_TIMESTAMP_RSP                     (0x23U)//	获取用于 CCC 时间同步的 UWB 时间
#define NXP_UCI_MSG_OID_PROPRIETARY_DEVICE_SUSPEND_RSP                          (0x24U)//	用于进入低功耗模式的命令
#define NXP_UCI_MSG_OID_PROPRIETARY_SET_TRIM_VALUES_RSP                         (0x26U)//
#define NXP_UCI_MSG_OID_PROPRIETARY_GET_ALL_UWB_SESSIONS_RSP                    (0x27U)//	用于获取已初始化 UWB 会话列表的命令
#define NXP_UCI_MSG_OID_PROPRIETARY_GET_TRIM_VALUES_RSP                         (0x28U)//	用于读取修整值的命令
#define NXP_UCI_MSG_OID_PROPRIETARY_STORE_PROTECTION_KEY_RSP                    (0x29U)//	用于将密钥保护密钥存储在密钥槽中的命令
#define NXP_UCI_MSG_OID_PROPRIETARY_SET_EPOCH_ID_RSP                            (0x2AU)//	用于设置 Epoch Id 的命令。




/*----------------- OID UCI Core Group CMD ---------------------------*/

#define NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_DEVICE_RESET                                               (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_CORE_DEVICE_RESET_CMD)
//0x81
#define NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_DEVICE_STATUS                                              (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_CORE_DEVICE_STATUS_NTF)
#define NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_GET_DEVICE_INFO                                            (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_CORE_GET_DEVICE_INFO_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_GET_CAPS_INFO                                              (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_CORE_GET_CAPS_INFO_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_SET_CONFIG                                                 (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_CORE_SET_CONFIG_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_GET_CONFIG                                                 (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_CORE_GET_CONFIG_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_RFU                                                        (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_CORE_RFU)
#define NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_GENERIC_ERROR                                              (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_CORE_GENERIC_ERROR_NTF)

/*----------------- OID UCI UWB Session Config CMD --------------------*/
#define NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_INIT                                             (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_SESSION_INIT_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_DEINIT                                           (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_SESSION_DEINIT_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_STATUS                                           (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_SESSION_STATUS_NTF)
#define NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_SET_APP_CONFIG                                   (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_SESSION_SET_APP_CONFIG_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_GET_APP_CONFIG                                   (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_SESSION_GET_APP_CONFIG_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_GET_COUNT                                        (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_SESSION_GET_COUNT_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_GET_STATE                                        (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_SESSION_GET_STATE_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_GET_POSSIBLE_RAN_MULTIPLIER_VALUE                (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_SESSION_GET_POSSIBLE_RAN_MULTIPLIER_VALUE_CMD)

/*----------------- OID UCI UWB Ranging Session CMD -------------------*/
#define NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_START                                             (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_RANGE_START_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_STOP                                              (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_RANGE_STOP_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_GET_RANGING_COUNT                                 (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_RANGE_GET_RANGING_COUNT_CMD)
#define NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_CCC_DATA_NTF                                      (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_RANGE_CCC_DATA_NTF)
#define NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_RESUME                                            (NXP_UCI_MSG_HEADER_OCT1_FIXED_BIT | NXP_UCI_MSG_OID_RANGE_RESUME_CMD)


typedef enum enum_UWB_UCI_Command_Index_t
{
    CORE_CMD_DEVICE_RESET,
    CORE_CMD_DEVICE_STATUS,
    CORE_CMD_GET_DEVICE_INFO,
    CORE_CMD_GET_CAPS_INFO,
	CORE_CMD_SET_TRIM_VALUES,
    CORE_CMD_SET_CONFIG,
    CORE_CMD_GET_CONFIG,
    CORE_CMD_RFU,
    CORE_CMD_GENERIC_ERROR,
    SESSION_CONFIG_CMD_INIT,
    SESSION_CONFIG_CMD_DEINIT,
    SESSION_CONFIG_CMD_STATUS,
    SESSION_CONFIG_CMD_SET_APP_CONFIG,
    SESSION_CONFIG_CMD_GET_APP_CONFIG,
    SESSION_CONFIG_CMD_GET_COUNT,
    SESSION_CONFIG_CMD_GET_STATE,
    SESSION_CONFIG_CMD_GET_POSSIBLE_RAN_MULTIPLIER_VALUE,
    RANGE_SESSION_CMD_START,
    RANGE_SESSION_CMD_STOP,
    RANGE_SESSION_CMD_GET_RANGING_COUNT,
    RANGE_SESSION_CMD_CCC_DATA_NTF,
    RANGE_SESSION_CMD_RESUME,


}E_UWBUCICmdIdx;


typedef union union_NXP_UCI_Msg_OCT1_t
{
	uint8_t u8BytePart;
	struct
	{
		uint8_t  gid:4;
		uint8_t  pbf:1;
		uint8_t  mt:3;
	}stBitPart;

}unio_NXPUCIMsgOCT1;

typedef union union_NXP_UCI_Msg_OCT2_t
{
	uint8_t BytePart;
	struct
	{
		uint8_t  oid : 6;
		uint8_t  rfu2 : 1;
		uint8_t  rfu1 : 1;
	}stBitPart;

}unio_NXPUCIMsgOCT2;

#pragma pack(1)
typedef struct struct_NXP_UCI_Msg_t//st_nxp_uci_msg_t
{
	uint8_t								u8PaddingByte;
	unio_NXPUCIMsgOCT1      			unioOCT0;
	unio_NXPUCIMsgOCT2         			unioOCT1;
	uint8_t								u8OCT2;
	uint8_t								u8OCT3;
	uint16_t                   			u16PaylaodLens;
	uint8_t                   			*pbufPaylaod;
}ST_NXPUCIMsg;
#pragma pack()

typedef struct struct_NXP_UCI_Core_Info_t
{
	bool		IsUCICore;						/*<当前是否为UCI Core*/
    uint16_t    generic_verison;                /*<! 高八位|低八位：主要编号|次要编号*/
    uint8_t     additional_info_len;            /*<! 附加信息长度*/
    uint8_t*    additional_info;                /*<! 附加信息*/
}ST_NXPUCICoreInfo;



typedef enum enum_UCI_Session_Seq_t
{
	SESSION_INIT_CMD				= 0,
	SESSION_SET_APP_CONFIG_CMD		= 1,
	RANGE_START_CMD					= 3,
	RANGE_CCC_DATA_NTF				= 4
}E_UCISessionSeq;

/*
static void			nxp_uci_cmd_parse(st_NXPUCIDat* pst_send, ST_UCIFrameState* pstate);
static E_UWBErrCode nxp_uci_ccc_cmd_package(uint32_t cmd, st_NXPUCIDat* pst_send, ST_UCIFrameState* pstate);
static void			nxp_uci_res_parse(st_NXPUCIDat* pst_recv, ST_UCIFrameState* pstate);
static void			nxp_uci_ccc_res_analyze(st_NXPUCIDat* pst_recv, ST_UCIFrameState *pstate);
*/

extern E_UWBErrCode API_UCI_Frame_Init(ST_UCIFrameState* pst_frame_state);
extern E_UWBErrCode API_UCI_Frame_Entry(ST_UCIFrameState* pst_frame_state);
extern uint8_t bufSessionSetupCANBuffer[64];

#endif /* SOURCES_NXP_S32K_APPLICATION_NXP_UCI_CMD_H_ */

#if 0 
abandoned code
static UWBErrCode nxp_uci_proprietary_cmd_package(uint32_t cmd, st_nxp_uci_dat* pst_send, st_uci_frame_state* pstate);
extern UWBErrCode API_uci_frame_init(st_uci_frame_state *pstate);
extern uint8_t 	uwb_uci_send_buffer[UCISW_MAX_FRAME_LENGTH];
extern uint8_t 	uwb_uci_recv_buffer[UCISW_MAX_FRAME_LENGTH];
static void nxp_uci_proprietary_res_analyze(st_nxp_uci_dat* pst_recv, st_uci_frame_state *pstate);
st_nxp_uci_dat 			st_uci_send;
st_nxp_uci_dat 			st_uci_recv;
static UciFrameState 			FrameState = UciFrameState_Error;
#endif
