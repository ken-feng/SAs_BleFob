#ifndef _BLE_CCC_SUP_SERVICE_H_
#define _BLE_CCC_SUP_SERVICE_H_
#include "EM000101.h"
#include "ble_ccc_dk.h"

typedef enum
{
	DK_SS_TIME_SYNC = 0x0D, 	    /*时间同步*/
	DK_SS_FIRST_APPROACH_RQ,	  	/*第一次靠近请求*/	
	DK_SS_FIRST_APPROACH_RS, 		/*第一次靠近响应*/
	DK_SS_RKE_AUTH_RQ = 0x14,	  	/*车控认证请求*/	
	DK_SS_RKE_AUTH_RS,	            /*车控认证响应*/
} ssCmd_t;

/*Ranging_Capability_RQ*/
typedef struct 
{
	u8  DeviceEventCount[8];
    u8  UWB_Device_Time[8];
    u8  UWB_Device_Time_Uncertainty;
    u8  UWB_Clock_Skew_Measurement_available;
    u16 Device_max_PPM;
    u8  Success;
    u16 RetryDelay;
}ccc_ss_time_sync_t;

/**
 * @brief
 *      补充服务 初始化      
 * @note
 */
void ble_ccc_sup_service_init(void);

/**
 * @brief
 *      解析收到的补充服务数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_sup_service_message_recv(ccc_dk_message_txt_t* dkMessage);

/**
 * @brief
 *      解析收到的补充服务数据
 * @param [outBuffer]     	待发送的数据缓存
 * @param [length]     		待发送的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_sup_service_message_send(ssCmd_t ssCmd,u8* outBuffer, u16 length);

#endif
