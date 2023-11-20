#ifndef _RTE_CCC_H_
#define _RTE_CCC_H_
//#include "ES010101.h"
#include "se_sdk.h"
#include "l2ca_cb_interface.h"
#include "gap_interface.h"


#define RTE_SE_Send_APDU                se_sdk_send_apdu//NULL_PTR /*发送APDU指令到SE*/
#define RTE_SE_Recv_APDU_CB             se_sdk_recv_apdu_call_back//NULL_PTR /*收取SE的APDU响应*/
#define RTE_SE_Recv_APDU                se_sdk_recv_apdu//NULL_PTR /*收取SE的APDU响应*/


#define RTE_UWB_SDK_Get_Capabilty       //NULL_PTR /*获取UWB SDK中参数*/
#define RTE_UWB_SDK_Notify_Setup_Info   //NULL_PTR /*获取UWB SDK中参数*/

#define RTE_L2ca_SendLeCbData           L2ca_SendLeCbData
#define RTE_BLE_GAP_LE_READ_PHY     Gap_LeReadPhy    
#define RTE_BLE_GAP_LE_SET_PHY      Gap_LeSetPhy

#define RTE_BLE_SEND_LE_SET_PHY_REQUEST ble_ccc_send_leSetPhyRequest
#endif
