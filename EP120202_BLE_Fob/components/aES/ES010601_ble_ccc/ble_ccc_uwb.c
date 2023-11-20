#include "ble_ccc.h"
#include "ble_ccc_uwb.h"
#include "ble_ccc_dk.h"
#include "ble_ccc_evt_notify.h"
#include "../ES000501_uwb/Sources/uwb_SDK_Interface.h"

ccc_uwb_txt_t   ccc_uwb_txt;

ccc_uwb_ranging_capability_rq_t ccc_uwb_ranging_capability_rq;
ccc_uwb_ranging_capability_rs_t ccc_uwb_ranging_capability_rs;

ccc_uwb_ranging_session_rq_t    ccc_uwb_ranging_session_rq;
ccc_uwb_ranging_session_rs_t    ccc_uwb_ranging_session_rs;

ccc_uwb_ranging_session_setup_rq_t  ccc_uwb_ranging_session_setup_rq;
ccc_uwb_ranging_session_setup_rs_t  ccc_uwb_ranging_session_setup_rs;

void ble_ccc_uwb_get_ranging_capability(u8* capBuffer, u16 length);
void ble_ccc_uwb_set_rss_rq_default_parameter(void);
void ble_ccc_uwb_notify_uwb_sdk_step1(void);
void ble_ccc_uwb_notify_uwb_sdk_step2(void);
/**
 * @brief
 *      UWB协商参数初始化      
 * @note
 */
void ble_ccc_uwb_init(void)
{
    u16 retLength = 0;
    core_mm_set((u8*)&ccc_uwb_txt,0x00,sizeof(ccc_uwb_txt));

    core_mm_set((u8*)&ccc_uwb_ranging_capability_rq,0x00,sizeof(ccc_uwb_ranging_capability_rq));
    core_mm_set((u8*)&ccc_uwb_ranging_capability_rs,0x00,sizeof(ccc_uwb_ranging_capability_rs));
    core_mm_set((u8*)&ccc_uwb_ranging_session_rq,0x00,sizeof(ccc_uwb_ranging_session_rq));
    core_mm_set((u8*)&ccc_uwb_ranging_session_rs,0x00,sizeof(ccc_uwb_ranging_session_rs));
    core_mm_set((u8*)&ccc_uwb_ranging_session_setup_rq,0x00,sizeof(ccc_uwb_ranging_session_setup_rq));
    core_mm_set((u8*)&ccc_uwb_ranging_session_setup_rs,0x00,sizeof(ccc_uwb_ranging_session_setup_rs));

    /*For test*/
    // ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len = 2U;
    // ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len = 2U;
    // ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len = 1U;

    /*调用UWB SDK模块获取UWB 协商默认参数*/
    // retLength = RTE_UWB_SDK_Get_Capabilty(bleCccSendBuffer);
    /*todo:获取默认参数*/
    //ble_ccc_uwb_get_ranging_capability(bleCccSendBuffer,retLength);

    /*设置测距会话默认参数为第一组*/
    // ccc_uwb_ranging_session_rq.Selected_DK_Protocol_Version = core_dcm_readBig16(ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version);
    // ccc_uwb_ranging_session_rq.Selected_UWB_Config_id = core_dcm_readBig16(ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id);
    // ccc_uwb_ranging_session_rq.Selected_PulseShape_Combo = core_dcm_readBig16(ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo);

    /*设置测距会话建立默认参数：可配置，后续UWB标定后确定最后的数据*/
    // ble_ccc_uwb_set_rss_rq_default_parameter();
}

/**
 * @brief
 *      发送EVT事件供系统调度
 * @param [uwbStep]     当前流程
 * @param [uwbEvtNtf_t]     当前通知事件
 * @return
 * @note
 */

u8 ble_ccc_uwb_send_evt(uwbStep_t uwbStep,uwbEvtNtf_t uwbEvtNtf)
{
    ccc_uwb_txt_t ccc_uwb_txt_tmp;
    ccc_uwb_txt_tmp.uwbStep = uwbStep;
    ccc_uwb_txt_tmp.uwbEvtNtf = uwbEvtNtf;
    return ble_ccc_send_evt(CCC_EVT_DK_UWB,ble_ccc_ctx.deviceId,(u8*)&ccc_uwb_txt_tmp,sizeof(ccc_uwb_txt_tmp));
}
/**
 * @brief
 *      设置测距会话 session id
 * @param [sessionId]     会话ID
 * @return
 * @note
 */
void ble_ccc_uwb_set_rs_session_id(u32 sessionId)
{
    ccc_uwb_ranging_session_rq.UWB_Session_Id = sessionId;
}
/**
 * @brief
 *      设置测距会话 channel bitmask
 * @param [Channel_Bitmask]     channel bitmask
 * @return
 * @note
 */
void ble_ccc_uwb_set_rs_channel_bitmask(u8 Channel_Bitmask)
{
    ccc_uwb_ranging_session_rq.Channel_Bitmask = Channel_Bitmask;
}

/**
 * @brief
 *      设置测距会话建立请求参数
 * @return
 * @note
 */
void ble_ccc_uwb_set_rss_rq_default_parameter(void)
{
    ccc_uwb_ranging_session_setup_rq.Session_RAN_Multiplier = 0x01;//ccc_uwb_ranging_session_rs.RAN_Multiplier;
    ccc_uwb_ranging_session_setup_rq.Number_Chaps_per_Slot = 0x01;//ccc_uwb_ranging_session_rs.Slot_BitMask;
    ccc_uwb_ranging_session_setup_rq.Number_Responders_Nodes = 0x01;//
    ccc_uwb_ranging_session_setup_rq.Number_Slots_per_Round = 0x06;
    ccc_uwb_ranging_session_setup_rq.SYNC_Code_Index = 0x00000001;
    ccc_uwb_ranging_session_setup_rq.Selected_Hopping_Config_Bitmask = 0x01;//ccc_uwb_ranging_session_rs.Hopping_Config_Bitmask;
}

/**
 * @brief
 *      获取UWB SDK模块数据
 * @param [capBuffer]     存储UWB能力的数据缓存(DK Message)
 * @param [length]        数据缓存长度
 * @return
 *        
 * @note
 */
void ble_ccc_uwb_get_ranging_capability(u8* capBuffer, u16 length)
{
    u16 offset = 0;
    ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len = capBuffer[offset++];
    core_mm_copy(ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version,capBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len);
    offset += ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len;

    ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len = capBuffer[offset++];
    core_mm_copy(ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id,capBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len);
    offset += ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len;
    
    ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len = capBuffer[offset++];
    core_mm_copy(ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo,capBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len);
    offset += ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len;
}   


/**
 * @brief
 *      拼DK UWB协商指令报文
 * @param [uwbCmd]      uwbCmd_t  DK uwb 指令
 * @param [outBuffer]   拼接好的数据缓存，完整的数据
 * @return
 *        拼好的数据长度
 * @note
 */
u16 ble_ccc_uwb_org_cmd_data(uwbCmd_t uwbCmd,u8* outBuffer)
{
    u16 cmdLength = 0;
    u16 offset = DK_MESSAGE_OFFSET_DATA;
    switch (uwbCmd)
    {
    case DK_UWB_RANGING_CAPABILITY_RQ:
        // LOG_L_S(CCC_MD,"Send CMD: UWB Ranging Capability Request !!!\r\n");
        // outBuffer[offset++] = ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len;
        // core_mm_copy(outBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version,ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len);
        // offset += ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len;

        // outBuffer[offset++] = ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len;
        // core_mm_copy(outBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id,ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len);
        // offset += ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len;

        // outBuffer[offset++] = ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len;
        // core_mm_copy(outBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo,ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len);
        // offset += ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len;
        // cmdLength = offset;
        break;
    case DK_UWB_RANGING_CAPABILITY_RS:
        LOG_L_S(CCC_MD,"Send CMD: UWB Ranging Capability Response !!!\r\n");
        ccc_uwb_ranging_capability_rs.Selected_DK_Protocol_Version = core_dcm_readBig16(ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version);
        ccc_uwb_ranging_capability_rs.Selected_UWB_Config_Id = core_dcm_readBig16(ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id);
        ccc_uwb_ranging_capability_rs.Selected_PulseShape_Combo = core_dcm_readU8(ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo);
        
        core_dcm_writeBig16(outBuffer+offset,ccc_uwb_ranging_capability_rs.Selected_DK_Protocol_Version);
        offset += 2;
        core_dcm_writeBig16(outBuffer+offset,ccc_uwb_ranging_capability_rs.Selected_UWB_Config_Id);
        offset += 2;
        core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_capability_rs.Selected_PulseShape_Combo);
        offset += 1;
        cmdLength = offset;
        break;
    case DK_UWB_RANGING_SESSION_RQ:
        // LOG_L_S(CCC_MD,"Send CMD: UWB Ranging Session Request !!!\r\n");
        // core_dcm_writeBig16(outBuffer+offset,ccc_uwb_ranging_session_rq.Selected_DK_Protocol_Version);
        // offset += 2;
        // core_dcm_writeBig16(outBuffer+offset,ccc_uwb_ranging_session_rq.Selected_UWB_Config_id);
        // offset += 2;
        // core_dcm_writeBig32(outBuffer+offset,ccc_uwb_ranging_session_rq.UWB_Session_Id);
        // offset += 4;
        // core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_rq.Selected_PulseShape_Combo);
        // offset += 1;
        // core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_rq.Channel_Bitmask);
        // offset += 1;
        // cmdLength = offset;
        break;
    case DK_UWB_RANGING_SESSION_RS:
        LOG_L_S(CCC_MD,"Send CMD: UWB Ranging Session Response !!!\r\n");
        ccc_uwb_ranging_session_rs.RAN_Multiplier = 1U;
        ccc_uwb_ranging_session_rs.Slot_BitMask = 0x08;
        ccc_uwb_ranging_session_rs.SYNC_Code_Index_BitMask = 0x00000001;
        if(stSource.stUCIState.stProtocol.stCCCCaps.u8ChannelBitMaskMap&0x02)
        {
            ccc_uwb_ranging_session_rs.Selected_UWB_Channel = 9U;
        }
        else
        {
            ccc_uwb_ranging_session_rs.Selected_UWB_Channel = 5U;
        }
        ccc_uwb_ranging_session_rs.Hopping_Config_Bitmask = 0x80U;//stSource.stUCIState.stProtocol.stCCCCaps.u8HopingConfigBitMask; //0x80U;
        
        core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_rs.RAN_Multiplier);
        offset += 1;
        core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_rs.Slot_BitMask);
        offset += 1;
        core_dcm_writeBig32(outBuffer+offset,ccc_uwb_ranging_session_rs.SYNC_Code_Index_BitMask);
        offset += 4;
        core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_rs.Selected_UWB_Channel);
        offset += 1;
        core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_rs.Hopping_Config_Bitmask);
        offset += 1;
        cmdLength = offset;
        break;
    case DK_UWB_RANGING_SESSION_SETUP_RQ:
        // LOG_L_S(CCC_MD,"Send CMD: UWB Ranging Session Setup Request !!!\r\n");
        // core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_setup_rq.Session_RAN_Multiplier);
        // offset += 2;
        // core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_setup_rq.Number_Chaps_per_Slot);
        // offset += 2;    
        // core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_setup_rq.Number_Responders_Nodes);
        // offset += 2;
        // core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_setup_rq.Number_Slots_per_Round);
        // offset += 2;
        // core_dcm_writeBig32(outBuffer+offset,ccc_uwb_ranging_session_setup_rq.SYNC_Code_Index);
        // offset += 4;
        // core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_setup_rq.Selected_Hopping_Config_Bitmask);
        // offset += 1;
        // cmdLength = offset;     
        break;
    case DK_UWB_RANGING_SESSION_SETUP_RS:
        LOG_L_S(CCC_MD,"Send CMD: UWB Ranging Session Setup Response !!!\r\n");
        ccc_uwb_ranging_session_setup_rs.STS_Index0 = 0x00000000U;
        core_mm_set(ccc_uwb_ranging_session_setup_rs.UWB_Time0,0x00,8U);
        ccc_uwb_ranging_session_setup_rs.HOP_Mode_Key = 0x00000000U;
        ccc_uwb_ranging_session_setup_rs.SYNC_Code_Index = 9U;

        core_dcm_writeBig32(outBuffer+offset,ccc_uwb_ranging_session_setup_rs.STS_Index0);
        offset += 4;
        core_mm_copy(outBuffer+offset,ccc_uwb_ranging_session_setup_rs.UWB_Time0,8U);
        offset += 8;    
        core_dcm_writeBig32(outBuffer+offset,ccc_uwb_ranging_session_setup_rs.HOP_Mode_Key);
        offset += 4;
        core_dcm_writeU8(outBuffer+offset,ccc_uwb_ranging_session_setup_rs.SYNC_Code_Index);
        offset += 1;
        cmdLength = offset;     
        break;
    case DK_UWB_RANGING_SUSPEND_RQ:
    case DK_UWB_RANGING_RECOVERY_RQ:
    case DK_UWB_CONFIG_RANGING_RECOVERY_RQ:
    default:
        break;
    }
    core_dcm_writeU8(outBuffer+DK_MESSAGE_OFFSET_MSGHEADER,DK_MSG_TYPE_UWB_RANGING_SERVICE);
    core_dcm_writeU8(outBuffer+DK_MESSAGE_OFFSET_PAYLOADHEADER,uwbCmd);
    core_dcm_writeBig16(outBuffer+DK_MESSAGE_OFFSET_LENGTH,cmdLength - 4);
    return cmdLength;
} 

/**
 * @brief
 *      解析收到的DK数据
 * @param [uwbCmd]     uwbCmd_t  DK uwb 指令
 * @param [inBuffer]   报文Data 部分数据
 * @param [inLength]   报文Length
 * @return
 *        cccResult_t
 * @note
 */
cccResult_t ble_ccc_uwb_analyse_cmd(uwbCmd_t uwbCmd,u8* inBuffer,u16 inLength)
{
    u8 tmpBuf[40];
    cccResult_t cccResult = CCC_RESULT_SUCCESS;
    u16 offset = 0;
    switch (uwbCmd)
    {
    case DK_UWB_RANGING_CAPABILITY_RQ:
        LOG_L_S(CCC_MD,"Recv CMD: UWB Ranging Capability Request !!!\r\n");
        // if (inLength != 5U)
        // {
        //     /*todo:Error Length*/
        //     LOG_L_S(CCC_MD,"Data Length Error :0x%02x !!!\r\n",inLength);
        //     return CCC_RESULT_ERROR_LENGTH;
        // }
        ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        core_mm_copy(ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version,inBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len);
        offset += ccc_uwb_ranging_capability_rq.Supported_DK_Protocol_Version_Len;
        ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        core_mm_copy(ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id,inBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len);
        offset += ccc_uwb_ranging_capability_rq.Supported_UWB_Config_Id_Len;
        ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        core_mm_copy(ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo,inBuffer+offset,ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len);
        offset += ccc_uwb_ranging_capability_rq.Supported_PulseShape_Combo_Len;
        // ccc_uwb_txt.uwbStep = UWB_STEP_RANGING_CAPABILITY_RS;
        ble_ccc_uwb_send_evt(UWB_STEP_RANGING_CAPABILITY_RQ,UWB_EVT_NTF_NONE);
        break;        
    case DK_UWB_RANGING_CAPABILITY_RS:
        // LOG_L_S(CCC_MD,"Recv CMD: UWB Ranging Capability Response !!!\r\n");
        // if (inLength != 5U)
        // {
        //     /*todo:Error Length*/
        //     LOG_L_S(CCC_MD,"Data Length Error :0x%02x !!!\r\n",inLength);
        //     return CCC_RESULT_ERROR_LENGTH;
        // }
        // ccc_uwb_ranging_capability_rs.Selected_DK_Protocol_Version = core_dcm_readBig16(inBuffer+offset);
        // offset += 2;
        // ccc_uwb_ranging_capability_rs.Selected_UWB_Config_Id = core_dcm_readBig16(inBuffer+offset);
        // offset += 2;
        // ccc_uwb_ranging_capability_rs.Selected_PulseShape_Combo = core_dcm_readU8(inBuffer+offset);
        // // ccc_uwb_txt.uwbStep = UWB_STEP_RANGING_CAPABILITY_RS;
        // ble_ccc_uwb_send_evt(UWB_STEP_RANGING_CAPABILITY_RS,UWB_EVT_NTF_NONE);
        break;
    case DK_UWB_RANGING_SESSION_RQ:
        LOG_L_S(CCC_MD,"Recv CMD: UWB Ranging Session Request !!!\r\n");
        if (inLength != 10U)
        {
            /*todo:Error Length*/
            LOG_L_S(CCC_MD,"Data Length Error :0x%02x !!!\r\n",inLength);
            return CCC_RESULT_ERROR_LENGTH;
        }
        ccc_uwb_ranging_session_rq.Selected_DK_Protocol_Version = core_dcm_readBig16(inBuffer+offset);
        offset += 2;
        ccc_uwb_ranging_session_rq.Selected_UWB_Config_id = core_dcm_readBig16(inBuffer+offset);
        offset += 2;
        ccc_uwb_ranging_session_rq.UWB_Session_Id = core_dcm_readBig32(inBuffer+offset);
        offset += 4;
        ccc_uwb_ranging_session_rq.Selected_PulseShape_Combo = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        ccc_uwb_ranging_session_rq.Channel_Bitmask = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        core_dcm_writeBig32(tmpBuf,ccc_uwb_ranging_session_rq.UWB_Session_Id);
        if (ble_ccc_find_ursk(tmpBuf,tmpBuf+4U) == FALSE)
        {
            LOG_L_S(CCC_MD,"UWB_Session_Id And URSK Not Found!!!\r\n");
            ble_ccc_uwb_send_evt(UWB_STEP_CONFIG_OTHER,UWB_EVT_NTF_URSK_NOT_FOUND);
        }
        else
        {
            ble_ccc_set_select_ursk_sessionid(tmpBuf,tmpBuf+4U);
            // ccc_uwb_txt.uwbStep = UWB_STEP_RANGING_SESSION_RS;
            ble_ccc_uwb_send_evt(UWB_STEP_RANGING_SESSION_RQ,UWB_EVT_NTF_NONE);
        }
        break;        
    case DK_UWB_RANGING_SESSION_RS:
        // LOG_L_S(CCC_MD,"Recv CMD: UWB Ranging Session Response !!!\r\n");
        // if (inLength != 8U)
        // {
        //     /*todo:Error Length*/
        //     LOG_L_S(CCC_MD,"Data Length Error :0x%02x !!!\r\n",inLength);
        //     return CCC_RESULT_ERROR_LENGTH;
        // }
        // ccc_uwb_ranging_session_rs.RAN_Multiplier = core_dcm_readU8(inBuffer+offset);
        // offset += 1;
        // ccc_uwb_ranging_session_rs.Slot_BitMask = core_dcm_readU8(inBuffer+offset);
        // offset += 1;
        // ccc_uwb_ranging_session_rs.SYNC_Code_Index_BitMask = core_dcm_readBig32(inBuffer+offset);
        // offset += 4;
        // ccc_uwb_ranging_session_rs.Selected_UWB_Channel = core_dcm_readU8(inBuffer+offset);
        // offset += 1;
        // ccc_uwb_ranging_session_rs.Hopping_Config_Bitmask = core_dcm_readU8(inBuffer+offset);
        // // ccc_uwb_txt.uwbStep = UWB_STEP_RANGING_SESSION_RS;
        // ble_ccc_uwb_send_evt(UWB_STEP_RANGING_CAPABILITY_RS,UWB_EVT_NTF_NONE);
        break;
    case DK_UWB_RANGING_SESSION_SETUP_RQ:
        LOG_L_S(CCC_MD,"Recv CMD: UWB Ranging Session Setup Request !!!\r\n");
        if (inLength != 9U)
        {
            /*todo:Error Length*/
            LOG_L_S(CCC_MD,"Data Length Error :0x%02x !!!\r\n",inLength);
            return CCC_RESULT_ERROR_LENGTH;
        }
        ccc_uwb_ranging_session_setup_rq.Session_RAN_Multiplier = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        ccc_uwb_ranging_session_setup_rq.Number_Chaps_per_Slot = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        ccc_uwb_ranging_session_setup_rq.Number_Responders_Nodes = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        ccc_uwb_ranging_session_setup_rq.Number_Slots_per_Round = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        ccc_uwb_ranging_session_setup_rq.SYNC_Code_Index = core_dcm_readBig32(inBuffer+offset);
        offset += 4;
        ccc_uwb_ranging_session_setup_rq.Selected_Hopping_Config_Bitmask = core_dcm_readU8(inBuffer+offset);
        offset += 1;
        // ccc_uwb_txt.uwbStep = UWB_STEP_RANGING_SESSION_SETUP_RS;
        ble_ccc_uwb_send_evt(UWB_STEP_RANGING_SESSION_SETUP_RQ,UWB_EVT_NTF_NONE);
        break;      
    case DK_UWB_RANGING_SESSION_SETUP_RS:
        // LOG_L_S(CCC_MD,"Recv CMD: UWB Ranging Session Setup Response !!!\r\n");
        // if (inLength != 17U)
        // {
        //     /*todo:Error Length*/
        //     LOG_L_S(CCC_MD,"Data Length Error :0x%02x !!!\r\n",inLength);
        //     return CCC_RESULT_ERROR_LENGTH;
        // }
        // ccc_uwb_ranging_session_setup_rs.STS_Index0 = core_dcm_readBig32(inBuffer+offset);
        // offset += 4;
        // core_mm_copy(ccc_uwb_ranging_session_setup_rs.UWB_Time0,inBuffer+offset,8);
        // offset += 8;
        // ccc_uwb_ranging_session_setup_rs.HOP_Mode_Key = core_dcm_readBig32(inBuffer+offset);
        // offset += 4;
        // ccc_uwb_ranging_session_setup_rs.SYNC_Code_Index = core_dcm_readU8(inBuffer+offset);
        // // ccc_uwb_txt.uwbStep = UWB_STEP_RANGING_SESSION_SETUP_RS;
        // ble_ccc_uwb_send_evt(UWB_STEP_RANGING_SESSION_SETUP_RS,UWB_EVT_NTF_NONE);
        break;    
    default:
        break;
    }
    return cccResult;
}
/**
 * @brief
 *      解析收到的DK数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_uwb_message_recv(ccc_dk_message_txt_t* dkMessage)
{
    cccResult_t cccResult;
    cccResult = ble_ccc_uwb_analyse_cmd(dkMessage->payloadHeader,dkMessage->data,dkMessage->length);
    if (cccResult != CCC_RESULT_SUCCESS)
    {
        return cccResult;
    }
    switch (dkMessage->payloadHeader)
    {
    case DK_UWB_RANGING_CAPABILITY_RS:
        cccResult = ble_ccc_uwb_send_cmd(DK_UWB_RANGING_SESSION_RQ);
        break;
    case DK_UWB_RANGING_SESSION_RS:
        cccResult = ble_ccc_uwb_send_cmd(DK_UWB_RANGING_SESSION_SETUP_RQ);
        break;
    case DK_UWB_RANGING_SESSION_SETUP_RS:
        /*UWB协商建立完成，通知UWB SDK开始测距*/
        break; 
    case DK_UWB_RANGING_SUSPEND_RS:
        /*收到UWB挂起通知，通知UWB SDK挂起*/
        break;    
    case DK_UWB_RANGING_RECOVERY_RS:
        /*收到UWB恢复测距，通知UWB SDK恢复测距*/
        break;
    case DK_UWB_CONFIG_RANGING_RECOVERY_RS:
        /*收到UWB可配置恢复测距，通知UWB SDK可恢复测距*/
        break;
    default:
        break;
    }
    return cccResult;
}
/**
 * @brief
 *      组织并且发送指令数据
 * @param [uwbCmd]     指令
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_uwb_send_cmd(uwbCmd_t uwbCmd)
{
    u16 sendDataLength;
    sendDataLength = ble_ccc_uwb_org_cmd_data(uwbCmd,bleCccSendBuffer);
    return ble_ccc_uwb_message_send(bleCccSendBuffer,sendDataLength);
}


/**
 * @brief
 *      解析收到的DK数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_uwb_message_send(u8* outBuffer, u16 length)
{
    return ble_ccc_dk_message_send(outBuffer,length);
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
void ble_ccc_connect_uwb_notify(void)
{
    u16 sendDataLength;
    if (ccc_uwb_txt.uwbStep == UWB_STEP_INIT)/*初始状态，需要立刻发送测距能力请求*/
    {
        ble_ccc_uwb_send_cmd(DK_UWB_RANGING_CAPABILITY_RQ);
        return;
    }
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
void ble_ccc_disconnect_uwb_notify(void)
{
    ccc_uwb_txt.uwbStep = UWB_STEP_INIT;
    
}


/**
 * @brief
 *      测距会话建立，握手成功，通知UWB模块开始测距
 * @param 
 * @return
 *        
 * @note 
 */
void ble_ccc_uwb_notify_uwb_sdk_step1(void)
{
    u8 tempBuf[64] = {0U};
    core_dcm_writeBig32(tempBuf, ccc_uwb_ranging_session_rq.UWB_Session_Id);
    core_dcm_writeBig16(tempBuf+4, ccc_uwb_ranging_session_rq.Selected_UWB_Config_id);
    core_dcm_writeU8(tempBuf+4+2, ccc_uwb_ranging_session_rq.Selected_PulseShape_Combo);
    core_dcm_writeU8(tempBuf+4+2+1, ccc_uwb_ranging_session_rs.RAN_Multiplier);
    core_dcm_writeU8(tempBuf+4+2+1+1, ccc_uwb_ranging_session_rs.Selected_UWB_Channel);
    core_dcm_writeU8(tempBuf+4+2+1+1+1, ccc_uwb_ranging_session_setup_rq.Number_Chaps_per_Slot);
    core_dcm_writeU8(tempBuf+4+2+1+1+1+1, ccc_uwb_ranging_session_setup_rq.Number_Slots_per_Round);
    core_dcm_writeU8(tempBuf+4+2+1+1+1+1+1, ccc_uwb_ranging_session_setup_rq.Selected_Hopping_Config_Bitmask);
    core_dcm_writeBig32(tempBuf+4+2+1+1+1+1+1+1, ccc_uwb_ranging_session_setup_rs.STS_Index0);
    core_mm_copy(tempBuf+4+2+1+1+1+1+1+1+4,ccc_uwb_ranging_session_setup_rs.UWB_Time0,8U);
    core_dcm_writeBig32(tempBuf+4+2+1+1+1+1+1+1+4+8, ccc_uwb_ranging_session_setup_rs.HOP_Mode_Key);
    core_dcm_writeU8(tempBuf+4+2+1+1+1+1+1+1+4+8+4, ccc_uwb_ranging_session_setup_rs.SYNC_Code_Index);

    core_dcm_writeU8(tempBuf+4+2+1+1+1+1+1+1+4+8+4+1, 10U);
    core_dcm_writeU8(tempBuf+4+2+1+1+1+1+1+1+4+8+4+1+1, 4U);
    core_dcm_writeU8(tempBuf+4+2+1+1+1+1+1+1+4+8+4+1+1+1, 0x80U);
    //RTE_UWB_SDK_Notify_Setup_Info();
    ble_ccc_send_evt(UWB_EVT_RANGING_SESSION_SETUP_RQ,ble_ccc_ctx.deviceId,tempBuf,64U);
    /*TODO:  */
    LOG_L_S(CCC_MD,"UWB Ranging Session Setup Successfully !!! \r\n");
}
void ble_ccc_uwb_notify_uwb_sdk_step2(void)
{
    u8 tempBuf[64] = {0U};
    core_dcm_writeBig32(tempBuf, ccc_uwb_ranging_session_rq.UWB_Session_Id);
    ble_ccc_find_ursk(tempBuf,tempBuf+4U);
    //RTE_UWB_SDK_Notify_Setup_Info();
    ble_ccc_send_evt(UWB_EVT_RANGING_SESSION_START_RQ,ble_ccc_ctx.deviceId,tempBuf,64U);
    /*TODO:  */
    LOG_L_S(CCC_MD,"UWB Ranging Session Start Successfully !!! \r\n");
}


void ble_ccc_uwb_process(u8* inBuffer, u16 length)
{
    u8 type;
    ccc_uwb_txt_t* ccc_uwb_txt_tmp;
    ccc_uwb_txt_tmp = (ccc_uwb_txt_t*)inBuffer;
    switch (ccc_uwb_txt_tmp->uwbStep)
    {
    case UWB_STEP_INIT:
        
        break;
    case UWB_STEP_RANGING_CAPABILITY_RQ:        /*测距能力请求（RC-RQ）*/
        ble_ccc_uwb_send_cmd(UWB_STEP_RANGING_CAPABILITY_RS);
        // ble_ccc_uwb_send_evt(UWB_STEP_RANGING_SESSION_RQ);
        break;
    case UWB_STEP_RANGING_CAPABILITY_RS:		/*测距能力响应（RC-RS）*/
        
        break;
    case UWB_STEP_RANGING_SESSION_RQ:			/*测距会话请求（RS-RQ）*/
        ble_ccc_uwb_send_cmd(UWB_STEP_RANGING_SESSION_RS);
        // ble_ccc_uwb_send_evt(UWB_STEP_RANGING_SESSION_SETUP_RQ);
        break;
    case UWB_STEP_RANGING_SESSION_RS:	  		/*测距会话响应（RS-RS）*/	
        
        break;
    case UWB_STEP_RANGING_SESSION_SETUP_RQ:		/*测距会话建立请求（RSS-RQ）*/
        ble_ccc_uwb_send_cmd(UWB_STEP_RANGING_SESSION_SETUP_RS);
        /*通知UWB SDK 握手完成*/
        ble_ccc_uwb_notify_uwb_sdk_step1();
        break;
    case UWB_STEP_RANGING_SESSION_SETUP_RS:		/*测距会话建立响应（RSS-RS）*/	
                
        break;
    case UWB_STEP_RANGING_SUSPEND_RQ:			/*测距挂起请求消息（RSD-RQ）*/
        type = 2U;
        ble_ccc_send_evt(UWB_EVT_RANGING_SESSION_SUSPEND_RQ,ble_ccc_ctx.deviceId,(u8*)&type,1U);
        break;
    case UWB_STEP_RANGING_SUSPEND_RS:			/*测距挂起响应消息（RSD-RS）*/	
        
        break;
    case UWB_STEP_RANGING_RECOVERY_RQ:			/*测距恢复请求消息（RR-RQ）*/
        type = 3U;
        ble_ccc_send_evt(UWB_EVT_RANGING_SESSION_SUSPEND_RQ,ble_ccc_ctx.deviceId,(u8*)&type,1U);
        break;
    case UWB_STEP_RANGING_RECOVERY_RS:			/*测距恢复响应消息（RR-RS）*/	
        
        break;
    case UWB_STEP_CONFIG_RANGING_RECOVERY_RQ:	/*可配置测距恢复请求消息（CRR-RQ）*/
        
        break;
    case UWB_STEP_CONFIG_RANGING_RECOVERY_RS:	/*可配置测距恢复响应消息（CRR-RS）*/
        
        break;
    case UWB_STEP_CONFIG_OTHER:	/*可配置测距恢复响应消息（CRR-RS）*/
        if (ccc_uwb_txt_tmp->uwbEvtNtf == UWB_EVT_NTF_URSK_NOT_FOUND)
        {
            ble_ccc_evt_ranging_status_notify(CCC_SUBEVT_RS_URSK_NOT_FOUND);
        }
        break;
    default:
        break;
    }
}
