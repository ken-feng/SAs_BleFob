#ifndef _BLE_CCC_EVT_NTF_H_
#define _BLE_CCC_EVT_NTF_H_
#include "EM000101.h"
#include "ble_ccc_dk.h"


#define DK_EVNET_NTF_MSGID  0x11

typedef struct 
{
    u16 functionId;     /*功能ID*/
    u8  actionId;       /*执行动作ID*/  
    u8  vehilceStatus;         /*执行的结果*/
}ccc_evt_rke_status_t;


extern ccc_evt_rke_status_t    ccc_evt_rke_status;

typedef enum
{
    CCC_SUBEVT_CMD_COMPLETE = 1,        /*Command_Status 命令完成子事件*/
    CCC_SUBEVT_RANGING_SESSION_STATUS,  /*Session_Status 测距会话状态更改子事件*/
    CCC_SUBEVT_DEVICE_RANGING_INTENT,   /*DR_Intent*/
    CCC_SUBEVT_VEHICLE_STATUS_CHANGE,   /*Vehicle_Status*/
    CCC_SUBEVT_RKE_REQUEST,             /*Requested_Action*/
    CCC_SUBEVT_HEAD_UNIT_PAIRING,       /*Headunit_Pairing_Status*/
}ccc_subevent_category_t;

/*命令完成子事件*/
typedef enum
{
    CCC_SUBEVT_CS_DESELECT_SE = 0x00,           /**/
    CCC_SUBEVT_CS_BLE_PAIRING_READY,            /**/
    CCC_SUBEVT_CS_REQ_CAPABILITY_EXCHANGE,      /**/
    CCC_SUBEVT_CS_REQ_STANDARD_TRANSACTION,     /**/
    CCC_SUBEVT_CS_REQ_OWNER_PAIRING,            /**/
    
    CCC_SUBEVT_CS_GENERAL_ERROR = 0x80,         /**/
    CCC_SUBEVT_CS_DEVICE_SE_BUSY,
    CCC_SUBEVT_CS_DEVICE_SE_TRANSACTION_STATE_LOST,
    CCC_SUBEVT_CS_DEVICE_BUSY,
    CCC_SUBEVT_CS_CMD_TEMP_BLOCKED,
    CCC_SUBEVT_CS_UNSUPPORTED_CHANNEL_BITMASK,
    CCC_SUBEVT_CS_OP_DEVICE_NOT_INSIDE_VEHICLE,

    CCC_SUBEVT_CS_OOB_MISMATCH = 0xFC,
    CCC_SUBEVT_CS_BLE_PAIRING_FAILED,
    CCC_SUBEVT_CS_FA_CRYPTO_OPERATION_FAILED,
    CCC_SUBEVT_CS_WRONG_PARAMETERS,
}ccc_subevent_command_status_t;

/*测距会话状态更改子事件*/
typedef enum
{
    CCC_SUBEVT_RS_URSK_REFRESH = 0x00,          /*Vehicle may send this SubEvent to request clean-up for pre- derived URSKs*/
    CCC_SUBEVT_RS_URSK_NOT_FOUND,               /*Device shall send this SubEvent to signal, the device failed to find URSK for this UWB_Session_Id*/
    CCC_SUBEVT_RS_RFU,                          /**/
    CCC_SUBEVT_RS_SECURE_RANGING_FAILED,        /*Vehicle shall send this SubEvent to signal the Vehicle failed to establish secure ranging.*/
    CCC_SUBEVT_RS_TERMINATED,                   /*Vehicle or device may send this subevent if it has stopped the ranging session (e.g. due to URSK TTL expiration)*/
    CCC_SUBEVT_RS_RECOVERY_FAILED = 0x06,       /*Device shall send this SubEvent to signal it failed to recover ranging for this UWB_Session_Id*/
}ccc_subevent_ranging_session_status_t;

/*设备测距意图子事件代码*/
typedef enum
{
    CCC_SUBEVT_DI_LOW_APPROACH_CONFIDENCE = 0x00, /*Device notifies vehicle of user approaching with low confidence*/
    CCC_SUBEVT_DI_MEDIUM_APPROACH_CONFIDENCE,     /*Device notifies vehicle of user approaching with medium confidence*/
    CCC_SUBEVT_DI_HIGH_APPROACH_CONFIDENCE,       /*Device notifies vehicle of user approaching with high confidence*/
}ccc_subevent_dr_intent_t;

typedef enum
{
    CCC_RKE_REQ_ACTION = 0x7F70,        /*Request RKE action. Only present, when RKE action shall be triggered.*/
    CCC_RKE_CONTINUE_ACTION = 0x7F76,   /*Continue enduring RKE action. Only present, when enduring RKE action is in progress.*/
    CCC_RKE_STOP_ACTION = 0x7F77,       /*Stop enduring RKE action. Only present, when the enduring RKE action in progress shall be stopped.*/
}ccc_rke_tag_t;

typedef enum
{
    CCC_FUNCTION_ID_CENTRAL_LOCKING = 0x0001,
    CCC_FUNCTION_ID_LOCKING_SECURE,
    CCC_FUNCTION_ID_PANIC_ALARM = 0x0101,
    CCC_FUNCTION_ID_MANUAL_TRUNK = 0x0110,
}ccc_function_id_t;

typedef enum
{
    CCC_ACTION_ID_LOCK      = 0x00,
    CCC_ACTION_ID_UNLOCK    = 0x01,
    CCC_ACTION_ID_PANIC_MUTE_ALARM     = 0x00,
    CCC_ACTION_ID_PANIC_TRIGGER_ALARM  = 0x01,
    CCC_ACTION_ID_MANUAL_TRUNK_RALEASE = 0x01,
}ccc_action_id_t;

/**
 * @brief
 *      Event notify模块初始化      
 * @note
 */
void ble_ccc_evt_ntf_init(void);

/**
 * @brief
 *      解析收到的SE数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_ntf_recv(ccc_dk_message_txt_t* dkMessage);

/**
 * @brief
 *      发送子事件消息
 * @param [ccc_subevent_code]   子事件代码
 * @param [outBuffer]     	    待发送的数据缓存
 * @param [length]     	    	待发送的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_ntf_send(u8* outBuffer, u16 length);

/**
 * @brief
 *      发送RKE子事件消息
 * @param [ccc_function_id]   车控功能ID
 * @param [ccc_action_id]   车控动作ID
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_rke_cmd(ccc_function_id_t ccc_function_id,ccc_action_id_t ccc_action_id);

/**
 * @brief
 *      发送Ranging Session Status 子事件消息
 * @param [rangingStatus]  状态
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_ranging_status_notify(ccc_subevent_ranging_session_status_t rangingStatus);
#endif
