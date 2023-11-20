#ifndef _BLE_CCC_SE_H_
#define _BLE_CCC_SE_H_
#include "EM000101.h"
#include "ble_ccc_dk.h"
#include "ble_ccc.h"

#define DK_SE_APDU_RQ		0x0B
#define DK_SE_APDU_RS		0x0C

typedef struct 
{
    u8 reqApdu[200];
    u16 reqLength;
    u8 resApdu[200];
    u16 resLength;
}se_select_t;

typedef struct 
{
    u8 reqApdu[20];
    u16 reqLength;
    u8 resApdu[20];
    u16 resLength;
}se_setup_sec_channel_t;


typedef struct 
{
    u8 transaction_id[16];/*slot id*/
    u8 ursk[32];
}se_ursk_t;


typedef struct 
{
    se_select_t se_select;
    se_setup_sec_channel_t se_setup_sec_channel;
    u8 seSelectFlag;            /*当前是否已执行Select指令*/
    u8 seSetupSecChannelFlag;   /*当前是否已执行setup sec channel指令*/
    u8 seExportURSKFlag;        /*当前是否已执行export ursk 指令*/
    u8 seAuthFlag;              /*当前是否已认证成功*/
    u8 aesKey[16];
    u8 aesICV[16];
    se_ursk_t se_ursk;
}se_txt_t;


typedef enum
{
	SE_INS_SELECT = 0xA4, 			
    SE_INS_AUTH0 = 0x80, 			
    SE_INS_AUTH1 = 0x81, 			
    SE_INS_EXCHANGE = 0xC9, 		
    SE_INS_CONTROL_FLOW = 0x3C, 	

    SE_INS_CREATE_RANGING_KEY = 0x71, 		

    SE_INS_DELETE_RANGING_KEY = 0x41,

    SE_INS_SETUP_SEC_CHANNEL = 0x7C, 			
    SE_INS_EXPORT_URSK = 0x7E, 		

    SE_INS_RKE_VERIFY = 0x7A,	
} se_ins_t;








/**
 * @brief
 *      SE模块初始化      
 * @note
 */
void ble_ccc_se_init(void);

/**
 * @brief
 *      解析收到的SE数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_se_message_recv(ccc_dk_message_txt_t* dkMessage);

/**
 * @brief
 *      解析收到的SE数据
 * @param [outBuffer]     	待发送的数据缓存
 * @param [length]     		待发送的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_se_message_send(u8* outBuffer, u16 length);

/**
 * @brief
 *      透传收到的SE响应数据
 * @param [outBuffer]     	待发送的数据缓存
 * @param [length]     		待发送的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_se_recv_apdu_callback(u8* inBuffer, u16 length);


/**
 * @brief   
 *      收到 RKE_AUTH_RQ 后 往SE上下发 rke_verify 
 * @param [inData]     	    传入的数据缓存
 * @param [inLength]     	传入的数据长度
 * @param [outData]     	输出的数据缓存
 * @param [outLength]     	输出的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_rke_send_verify_apdu(u8* inData, u16 inLength, u8* outData,u16* outLength);

#endif
