#include "ble_ccc.h"
#include "ble_ccc_dk.h"
#include "ble_ccc_uwb.h"
#include "ble_ccc_se.h"
#include "ble_ccc_sup_service.h"
#include "ble_ccc_evt_notify.h"
ccc_dk_message_txt_t    ccc_dk_message_txt;
u8 bleCccSendBuffer[256];

/**
 * @brief
 *      DK模块初始化      
 * @note
 */
void ble_ccc_dk_init(void)
{
    core_mm_set((u8*)&ccc_dk_message_txt,0x00,sizeof(ccc_dk_message_txt));
    core_mm_set(bleCccSendBuffer,0x00,256);

    ble_ccc_se_init();
    ble_ccc_uwb_init();
    ble_ccc_sup_service_init();
    ble_ccc_evt_ntf_init();
}


/**
 * @brief
 *      处理接收到的数据报文
 * @param [inBuffer] 报文数据缓存
 * @param [length]   报文数据长度
 * @return
 *        拼好的数据长度
 * @note
 */
cccResult_t ble_ccc_dk_message_recv(u8* inBuffer,u16 length)
{
    cccResult_t cccResult = CCC_RESULT_SUCCESS;
    if (length<4)
    {
        LOG_L_S(CCC_MD,"Data Length Error :0x%02x !!!\r\n",length);
        return CCC_RESULT_ERROR_LENGTH;
    }
    ccc_dk_message_txt.messageHeader = inBuffer[DK_MESSAGE_OFFSET_MSGHEADER];
    ccc_dk_message_txt.payloadHeader = inBuffer[DK_MESSAGE_OFFSET_PAYLOADHEADER];
    ccc_dk_message_txt.length = core_dcm_readBig16(inBuffer+DK_MESSAGE_OFFSET_LENGTH);
    ccc_dk_message_txt.data = inBuffer + DK_MESSAGE_OFFSET_DATA;
    
    switch (ccc_dk_message_txt.messageHeader)
    {
    case DK_MSG_TYPE_FRAMEWORK:
    case DK_MSG_TYPE_SE:
        /*调用SE透传SDK接口*/
        cccResult = ble_ccc_se_message_recv((ccc_dk_message_txt_t*)&ccc_dk_message_txt);
        break;
    case DK_MSG_TYPE_UWB_RANGING_SERVICE:
        cccResult = ble_ccc_uwb_message_recv((ccc_dk_message_txt_t*)&ccc_dk_message_txt);
        break;
    case DK_MSG_TYPE_DK_EVT_NTF:
        cccResult = ble_ccc_evt_ntf_recv((ccc_dk_message_txt_t*)&ccc_dk_message_txt);
        break;
    case DK_MSG_TYPE_SUPPLEMENTARY_SERVICE:
        cccResult = ble_ccc_sup_service_message_recv((ccc_dk_message_txt_t*)&ccc_dk_message_txt);
    default:
        break;
    }
    return cccResult;
}
/**
 * @brief
 *      发送数据接口
 * @param [outBuffer]   包括完整报文头的数据缓存
 * @param [length]   包括完整报文头的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_dk_message_send(u8* outBuffer,u16 length)
{
    return ble_ccc_l2cap_send_data(outBuffer,length);
}

/**
 * @brief
 *      处理蓝牙连接后的事情
 * @param 
 * @return
 *        
 * @note 
 *      连接事件通知
 */
void ble_ccc_connect_dk_notify(void)
{
    // ble_ccc_connect_uwb_notify();
    // ble_ccc_connect_sup_service_notify();
}

/**
 * @brief
 *      处理蓝牙断开连接后的事情
 * @param 
 * @return
 *        
 * @note 
 *      断开连接事件通知
 */
void ble_ccc_disconnect_dk_notify(void)
{
    ble_ccc_disconnect_uwb_notify();
}
