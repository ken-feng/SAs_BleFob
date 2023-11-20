#ifndef _BLE_CCC_DK_H_
#define _BLE_CCC_DK_H_
#include "EM000101.h"
#include "ble_ccc_dk.h"
#include "ble_ccc.h"
typedef enum
{
	DK_MSG_TYPE_FRAMEWORK = 0, 			/*封装的SE信息*/
	DK_MSG_TYPE_SE, 					/*SE消息*/
	DK_MSG_TYPE_UWB_RANGING_SERVICE, 	/*UWB测距服务消息*/
	DK_MSG_TYPE_DK_EVT_NTF, 			/*DK 事件通知*/
	DK_MSG_TYPE_VEHICLE_OEM_APP, 		/*车辆 OEM App 消息*/
	DK_MSG_TYPE_SUPPLEMENTARY_SERVICE,	/*补充服务消息(时间同步+First_Approach+RKE)*/
	DK_MSG_TYPE_HEAD_UNIT_PAIRING, 		/*主机配对消息*/
} dkMsgType_t;

#define DK_MESSAGE_OFFSET_MSGHEADER			0
#define DK_MESSAGE_OFFSET_PAYLOADHEADER		1
#define DK_MESSAGE_OFFSET_LENGTH			2
#define DK_MESSAGE_OFFSET_DATA				4


extern u8 bleCccSendBuffer[256];

typedef struct 
{
	u8 messageHeader;
	u8 payloadHeader;
	u16 length;
	u8* data;
}ccc_dk_message_txt_t;


extern ccc_dk_message_txt_t    ccc_dk_message_txt;

/*数据接口*/
cccResult_t ble_ccc_dk_message_recv(u8* inBuffer,u16 length);
/*发送数据接口*/
cccResult_t ble_ccc_dk_message_send(u8* outBuffer,u16 length);

/**
 * @brief
 *      DK模块初始化      
 * @note
 */
void ble_ccc_dk_init(void);


/**
 * @brief
 *      处理蓝牙连接后的事情
 * @param 
 * @return
 *        
 * @note 
 *      连接事件通知
 */
void ble_ccc_connect_dk_notify(void);

/**
 * @brief
 *      处理蓝牙断开连接后的事情
 * @param 
 * @return
 *        
 * @note 
 *      断开连接事件通知
 */
void ble_ccc_disconnect_dk_notify(void);

#endif
