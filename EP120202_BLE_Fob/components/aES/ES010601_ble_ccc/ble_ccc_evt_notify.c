#include "ble_ccc.h"
#include "ble_ccc_dk.h"
#include "ble_ccc_evt_notify.h"
#include "ble_ccc_uwb.h"

ccc_evt_rke_status_t    ccc_evt_rke_status;

/**
 * @brief
 *      Event notify模块初始化      
 * @note
 */
void ble_ccc_evt_ntf_init(void)
{
    core_mm_set((u8*)&ccc_evt_rke_status,0x00,sizeof(ccc_evt_rke_status));
}

/**
 * @brief
 *      解析收到的SE数据
 * @param [outBuffer]     	待发送的数据缓存
 * @param [length]     		待发送的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_ntf_send(u8* outBuffer, u16 length)
{
    return ble_ccc_dk_message_send(outBuffer,length);
}
/**
 * @brief
 *      解析command status 数据结构
 * @param [inBuf]     输入数据
 * @param [length]    输入数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_parse_command_complete(u8* inBuf,  u16 length)
{
    cccResult_t cccResult = CCC_RESULT_SUCCESS;
    if (length != 1U)
    {
        return CCC_RESULT_ERROR_LENGTH;
    }
    switch (inBuf[0])
    {
    case CCC_SUBEVT_CS_DESELECT_SE:
    case CCC_SUBEVT_CS_BLE_PAIRING_READY:            /**/
    case CCC_SUBEVT_CS_REQ_STANDARD_TRANSACTION:     /**/
    case CCC_SUBEVT_CS_REQ_OWNER_PAIRING:            /**/
    case CCC_SUBEVT_CS_GENERAL_ERROR:         /**/
    case CCC_SUBEVT_CS_DEVICE_SE_BUSY:
    case CCC_SUBEVT_CS_DEVICE_SE_TRANSACTION_STATE_LOST:
    case CCC_SUBEVT_CS_DEVICE_BUSY:
    case CCC_SUBEVT_CS_CMD_TEMP_BLOCKED:
    case CCC_SUBEVT_CS_UNSUPPORTED_CHANNEL_BITMASK:
    case CCC_SUBEVT_CS_OP_DEVICE_NOT_INSIDE_VEHICLE:
    case CCC_SUBEVT_CS_OOB_MISMATCH:
    case CCC_SUBEVT_CS_BLE_PAIRING_FAILED:
    case CCC_SUBEVT_CS_FA_CRYPTO_OPERATION_FAILED:
    case CCC_SUBEVT_CS_WRONG_PARAMETERS:        
        break;
    
    case CCC_SUBEVT_CS_REQ_CAPABILITY_EXCHANGE:      /*请求 UWB能力交换*/
        ble_ccc_uwb_send_cmd(DK_UWB_RANGING_CAPABILITY_RQ);
        break;
    default:
        break;
    }
    return cccResult;
}

/**
 * @brief
 *      解析 ranging session status 数据结构
 * @param [inBuf]     输入数据
 * @param [length]    输入数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_parse_ranging_session_status(u8* inBuf,  u16 length)
{
    cccResult_t cccResult = CCC_RESULT_SUCCESS;
    u8 type;
    if (length != 1U)
    {
        return CCC_RESULT_ERROR_LENGTH;
    }
    switch (inBuf[0])
    {
    case CCC_SUBEVT_RS_URSK_REFRESH:                 /*Vehicle may send this SubEvent to request clean-up for pre- derived URSKs*/
        /**/
        LOG_L_S(CCC_MD,"CCC_SUBEVT_RS_URSK_REFRESH: Event Clear URSK!!!\r\n");
        ble_ccc_clear_ursk();
        break;
    case CCC_SUBEVT_RS_URSK_NOT_FOUND:               /*Device shall send this SubEvent to signal, the device failed to find URSK for this UWB_Session_Id*/
    case CCC_SUBEVT_RS_RFU:                          /**/
    case CCC_SUBEVT_RS_SECURE_RANGING_FAILED:        /*Vehicle shall send this SubEvent to signal the Vehicle failed to establish secure ranging.*/
        break;
    case CCC_SUBEVT_RS_TERMINATED:                   /*Vehicle or device may send this subevent if it has stopped the ranging session (e.g. due to URSK TTL expiration)*/
        LOG_L_S(CCC_MD,"CCC_SUBEVT_RS_TERMINATED: Event Clear URSK!!!\r\n");
        ble_ccc_clear_ursk();
        type = 1U;
        ble_ccc_send_evt(UWB_EVT_RANGING_SESSION_SUSPEND_RQ,ble_ccc_ctx.deviceId,(u8*)&type,1U);
        break;
    case CCC_SUBEVT_RS_RECOVERY_FAILED: 
        break;
    default:
        break;
    }
    return cccResult;
}
/**
 * @brief
 *      解析Vehicle Status On Change 数据结构
 * @param [inBuf]     输入数据
 * @param [length]    输入数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_parse_vehicle_status(u8* inBuf,  u16 length)
{
    cccResult_t cccResult = CCC_RESULT_SUCCESS;
    u8 tmpBuffer[10];
    u16 tmpLength;
    u8 tmpOffset;
    u8 tmpBuffer2[10];
    u16 tmpLength2;
    u8 tmpOffset2;
    u8 ret;
    ret = tlv_resolve(inBuf,length,0x7F21,tmpBuffer,&tmpLength,&tmpOffset);
    if (!ret)
    {
        return CCC_RESULT_FAILED;
    }
    ret = tlv_resolve(inBuf+tmpOffset,length,0x80,tmpBuffer2,&tmpLength2,&tmpOffset2);
    if (!ret)
    {
        return CCC_RESULT_FAILED;
    }
    if (core_dcm_readBig16(tmpBuffer2+2) != ccc_evt_rke_status.functionId)
    {
        return CCC_RESULT_FAILED;
    }
    
    ret = tlv_resolve(inBuf+tmpOffset,length,0x81,tmpBuffer2,&tmpLength2,&tmpOffset2);
    if (!ret)
    {
        return CCC_RESULT_FAILED;
    }
    if (core_dcm_readU8(tmpBuffer2+2) != ccc_evt_rke_status.actionId)
    {
        return CCC_RESULT_FAILED;
    }
    
    ret = tlv_resolve(inBuf+tmpOffset,length,0x82,tmpBuffer2,&tmpLength2,&tmpOffset2);
    if (!ret)
    {
        return CCC_RESULT_FAILED;
    }

    ccc_evt_rke_status.vehilceStatus = core_dcm_readU8(tmpBuffer2+2);
    LOG_L_S(CCC_MD,"Vehicle Execution Status: 0x%02x !!!\r\n",ccc_evt_rke_status.vehilceStatus);
    return cccResult;
}


/**
 * @brief
 *      解析收到的event notify数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_ntf_recv(ccc_dk_message_txt_t* dkMessage)
{
    cccResult_t cccResult = CCC_RESULT_SUCCESS;
    if (dkMessage->payloadHeader != DK_EVNET_NTF_MSGID)
    {
        /**/
        LOG_L_S(CCC_MD,"Event Message Id Error!!!!\r\n");
        cccResult = CCC_RESULT_FAILED;
        return cccResult;
    }
    
    switch (dkMessage->data[0])
    {
    case CCC_SUBEVT_CMD_COMPLETE:
        cccResult = ble_ccc_evt_parse_command_complete(dkMessage->data+1,dkMessage->length-1);
        break;
    case CCC_SUBEVT_RANGING_SESSION_STATUS:
        cccResult = ble_ccc_evt_parse_ranging_session_status(dkMessage->data+1,dkMessage->length-1);
        break;
    case CCC_SUBEVT_DEVICE_RANGING_INTENT:

        break;
    case CCC_SUBEVT_VEHICLE_STATUS_CHANGE:
        cccResult = ble_ccc_evt_parse_vehicle_status(dkMessage->data+1,dkMessage->length-1);
        break;
    case CCC_SUBEVT_RKE_REQUEST:
        
        break; 
    case CCC_SUBEVT_HEAD_UNIT_PAIRING:
        
        break;  
    default:
        break;
    }
	return CCC_RESULT_SUCCESS;
}



/**
 * @brief
 *      发送Ranging Session Status 子事件消息
 * @param [rangingStatus]  状态
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_ranging_status_notify(ccc_subevent_ranging_session_status_t rangingStatus)
{
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_MSGHEADER,DK_MSG_TYPE_DK_EVT_NTF);
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_PAYLOADHEADER,CCC_SUBEVT_RANGING_SESSION_STATUS);
    core_dcm_writeBig16(bleCccSendBuffer+DK_MESSAGE_OFFSET_LENGTH,1U);
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_DATA,rangingStatus);
    return ble_ccc_evt_ntf_send(bleCccSendBuffer,5U);

}
/**
 * @brief
 *      发送RKE子事件消息
 * @param [ccc_function_id]   车控功能ID
 * @param [ccc_action_id]   车控动作ID
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_evt_rke_cmd(ccc_function_id_t ccc_function_id,ccc_action_id_t ccc_action_id)
{
    u16 offset;
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_MSGHEADER,DK_MSG_TYPE_DK_EVT_NTF);
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_PAYLOADHEADER,DK_EVNET_NTF_MSGID);
    
    offset = DK_MESSAGE_OFFSET_DATA;
    /*Subevent Category*/
    core_dcm_writeU8(bleCccSendBuffer+offset,CCC_SUBEVT_RKE_REQUEST);
    offset++;
    /*Subevent Code*/
    core_dcm_writeBig16(bleCccSendBuffer+offset,CCC_RKE_REQ_ACTION);/*0x7F70*/
    offset += 2;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x07);/*lengh*/
    offset++;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x80);/*Function Id tag*/
    offset++;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x02);/*length*/
    offset++;
    core_dcm_writeBig16(bleCccSendBuffer+offset,ccc_function_id);/*function id*/
    offset += 2;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x81);/*action id*/
    offset++;
    core_dcm_writeU8(bleCccSendBuffer+offset,0x01);/*length*/
    offset++;
    core_dcm_writeU8(bleCccSendBuffer+offset,ccc_action_id);/*action id*/
    offset++;

    core_dcm_writeBig16(bleCccSendBuffer+DK_MESSAGE_OFFSET_LENGTH,offset - DK_MESSAGE_OFFSET_DATA);


    ccc_evt_rke_status.functionId = ccc_function_id;
    ccc_evt_rke_status.actionId = ccc_action_id;

    return ble_ccc_evt_ntf_send(bleCccSendBuffer,offset);

}