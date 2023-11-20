#include "ble_ccc.h"
#include "ble_ccc_dk.h"
#include "ble_ccc_sup_service.h"
#include "ble_ccc_evt_notify.h"
#include "ble_ccc_se.h"
ccc_ss_time_sync_t  ccc_ss_time_sync;

u8 rkeAuthChallenge[16];

u8 rkeApdu[262];
/**
 * @brief
 *      补充服务 初始化      
 * @note
 */
void ble_ccc_sup_service_init(void)
{
    core_mm_set((u8*)&ccc_ss_time_sync,0x00,sizeof(ccc_ss_time_sync));
    core_mm_set(rkeAuthChallenge,0x00,16);
    core_mm_set(rkeApdu,0x00,262);
    ccc_ss_time_sync.Success = 1U;
}

/**
 * @brief
 *      解析收到的DK数据
 * @param [ssCmd]     ssCmd_t  补充服务 指令
 * @param [inBuffer]   报文Data 部分数据
 * @param [inLength]   报文Length
 * @return
 *        cccResult_t
 * @note
 */
cccResult_t ble_ccc_ss_time_sync_analyse(ssCmd_t ssCmd,u8* inBuffer,u16 inLength)
{ 
    u16 offset = 0;
    switch (ssCmd)
    {
    case DK_SS_TIME_SYNC:
        if (inLength != 23U)
        {
            /*todo:Error Length*/
            return CCC_RESULT_ERROR_LENGTH;
        }
        core_mm_copy(ccc_ss_time_sync.DeviceEventCount,inBuffer+offset,8);
        offset += 8;
        core_mm_copy(ccc_ss_time_sync.UWB_Device_Time,inBuffer+offset,8);
        offset += 8;
        ccc_ss_time_sync.UWB_Device_Time_Uncertainty = core_dcm_readU8(inBuffer+offset);
        offset ++;
        ccc_ss_time_sync.UWB_Clock_Skew_Measurement_available = core_dcm_readU8(inBuffer+offset);
        offset ++;
        ccc_ss_time_sync.Device_max_PPM = core_dcm_readBig16(inBuffer+offset);
        offset += 2;
        ccc_ss_time_sync.Success = core_dcm_readU8(inBuffer+offset);
        offset ++;
        ccc_ss_time_sync.RetryDelay = core_dcm_readBig16(inBuffer+offset);
        break;
    default:
        break;
    }
    return CCC_RESULT_SUCCESS;
}


/**
 * @brief
 *      解析收到的DK数据
 * @param [ssCmd]     ssCmd_t  补充服务 指令
 * @param [inBuffer]   报文Data 部分数据
 * @param [inLength]   报文Length
 * @return
 *        cccResult_t
 * @note
 */
cccResult_t ble_ccc_ss_rke_auth_analyse(ssCmd_t ssCmd,u8* inBuffer,u16 inLength)
{ 
    // u8 tmpApduBuf[100];
    u16 length;
    cccResult_t cccResult;
    switch (ssCmd)
    {
    case DK_SS_RKE_AUTH_RQ:
        if (inLength != 16U)
        {
            /*todo:Error Length*/
            return CCC_RESULT_ERROR_LENGTH;
        }
        core_mm_copy(rkeAuthChallenge,inBuffer,16U);
        cccResult = ble_ccc_rke_send_verify_apdu(rkeAuthChallenge,16U,rkeApdu,&length);
        cccResult = ble_ccc_sup_service_message_send(DK_SS_RKE_AUTH_RS,rkeApdu,length);/*apduLength-2 是去掉0x9000*/
        break;
    default:
        break;
    }
    return CCC_RESULT_SUCCESS;
}



/**
 * @brief
 *      拼se获取签名指令
 * @param [challenage]      RKE_AUTH_RQ 下发的挑战码
 * @param [outBuffer]       拼好的APDU 数据缓存
 * @return
 *        APDU长度
 * @note
 */
u16 ble_ccc_ss_append_se_apdu_sign(u8* challenage,u8* outBuffer)
{ 
    u16 offset = 0;
    rkeApdu[offset++] = 0x80;/*CLA*/
    rkeApdu[offset++] = 0x7A;/*INS*/
    rkeApdu[offset++] = 0x00;/*P1*/
    rkeApdu[offset++] = 0x00;/*P2*/
    rkeApdu[offset++] = 0x1C;/*P3*/

    core_dcm_writeBig16(rkeApdu+offset,0x7F70);
    offset += 2;
    core_dcm_writeU8(rkeApdu+offset,0x07);
    offset++;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x80);/*Function Id tag*/
    offset++;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x02);/*length*/
    offset++;
    core_dcm_writeBig16(bleCccSendBuffer+offset,ccc_evt_rke_status.functionId);/*function id*/
    offset += 2;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x81);/*action id*/
    offset++;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x01);/*length*/
    offset++;
    core_dcm_writeU8(bleCccSendBuffer+offset,ccc_evt_rke_status.actionId);/*action id*/
    offset++;
    core_dcm_writeU8(rkeApdu+offset,0xC0);
    offset++;
    core_dcm_writeU8(rkeApdu+offset,0x10);
    offset++;
    core_mm_copy(rkeApdu+offset,challenage,16);
    offset += 16;
    return offset;
}

/**
 * @brief
 *      解析收到的补充服务数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_sup_service_message_recv(ccc_dk_message_txt_t* dkMessage)
{
    cccResult_t cccResult;
    switch (dkMessage->payloadHeader)
    {
    case DK_SS_TIME_SYNC:
        cccResult = ble_ccc_ss_time_sync_analyse(dkMessage->payloadHeader,dkMessage->data,dkMessage->length);
        break;
    case DK_SS_FIRST_APPROACH_RQ:
        /*RFU: FOB上没有此指令*/
        break;
    case DK_SS_RKE_AUTH_RQ:
        cccResult = ble_ccc_ss_rke_auth_analyse(dkMessage->payloadHeader,dkMessage->data,dkMessage->length);
        break;
    default:
        break;
    }
    return cccResult;
}
/**
 * @brief
 *      解析收到的补充服务数据
 * @param [outBuffer]     	待发送的数据缓存
 * @param [length]     		待发送的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_sup_service_message_send(ssCmd_t ssCmd,u8* outBuffer, u16 length)
{
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_MSGHEADER,DK_MSG_TYPE_SUPPLEMENTARY_SERVICE);
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_PAYLOADHEADER,ssCmd);
    core_dcm_writeBig16(bleCccSendBuffer+DK_MESSAGE_OFFSET_LENGTH,length);
    core_mm_copy(bleCccSendBuffer+DK_MESSAGE_OFFSET_DATA,outBuffer,length);
    return ble_ccc_se_message_send(bleCccSendBuffer,length+4);
}

/**
 * @brief
 *      拼时间同步报文
 * @param [outBuffer]   报文Data 部分数据
 * @return
 *        长度
 * @note
 */
u8 ble_ccc_ss_time_sync_org(u8* outBuffer)
{ 
    u16 offset = 0;

    core_dcm_writeBig16(outBuffer+offset,ble_ccc_get_ce_counter());
    offset += 8;
    /*ccc_ss_time_sync.UWB_Device_Time = UWB_SDK_GET_*/
    core_mm_copy(outBuffer+offset,ccc_ss_time_sync.UWB_Device_Time,8);
    offset += 8;
    /*ccc_ss_time_sync.UWB_Device_Time = UWB_SDK_GET_*/
    core_dcm_writeU8(outBuffer+offset,ccc_ss_time_sync.UWB_Device_Time_Uncertainty);
    offset ++;
    /*ccc_ss_time_sync.UWB_Device_Time = UWB_SDK_GET_*/
    core_dcm_writeU8(outBuffer+offset,ccc_ss_time_sync.UWB_Clock_Skew_Measurement_available);
    offset ++;
    /*ccc_ss_time_sync.UWB_Device_Time = UWB_SDK_GET_*/
    core_dcm_writeBig16(outBuffer+offset,ccc_ss_time_sync.Device_max_PPM);
    offset += 2;
    /*ccc_ss_time_sync.UWB_Device_Time = UWB_SDK_GET_*/
    core_dcm_writeU8(outBuffer+offset,ccc_ss_time_sync.Success);
    offset ++;
    /*ccc_ss_time_sync.UWB_Device_Time = UWB_SDK_GET_*/
    core_dcm_writeBig16(outBuffer+offset,ccc_ss_time_sync.RetryDelay);
    offset += 2;
    return offset;
}
/**
 * @brief
 *      处理蓝牙连接后的事情，UWB主动上发测距能力请求
 * @param 
 * @return
 *        
 * @note 
 *      连接事件通知
 */
void ble_ccc_connect_sup_service_notify(void)
{
    /*连接成功马上发送 时间同步 timer_sycn0*/
    u8 tempBuf[30];
    u16 length;
    ccc_ss_time_sync_t ccc_ss_time_sync;
    core_mm_set((u8*)&ccc_ss_time_sync,0x00,sizeof(ccc_ss_time_sync));
    core_mm_set(tempBuf,0x00,sizeof(tempBuf));
    length = ble_ccc_ss_time_sync_org(tempBuf);
    ble_ccc_sup_service_message_send(DK_SS_TIME_SYNC,tempBuf,length);
}