#include "ble_ccc.h"
#include "ble_ccc_dk.h"
#include "ble_ccc_se.h"
#include "se_sdk.h"
#include "EM000201.h"
#include "EM000101.h"
#include "ble_ccc_evt_notify.h"
#include "ble_ccc_sup_service.h"
#include "SecLib.h"


#define SW_URSK_FULL        0x6484
#define SW_OTHER_RESION     0x6400
#define SW_SUCCESS          0x9000


const u8 selectApduBuffer[18] = {0x00,0xA4,0x04,0x00,0x0D,0xA0,0x00,0x00,0x08,0x09,0x43,0x43,0x43,0x44,0x4B,0x41,0x76,0x31};
const u8 setupSecChannelApdhuBuffer_req[5] = {0x80,0x7C,0x00,0x00,0x43};
const u8 setupSecChannelApdhuBuffer_sure[5] = {0x80,0x7C,0x01,0x00,0x20};
const u8 exportURSKBuffer[5] = {0x80,0x7E,0x00,0x00,0x00};
const u8 rkeVerifyBuffer[5] = {0x80,0x7A,0x00,0x00,0x0A};

const u8 cccTempPubKey[65] = {0x04,0x82,0x7D,0x3A,0x01,0x7C,0x8C,0xF2,0x4E,0xAF,0x48,0x5F,0x46,0xB6,0x53,0xB3,0x2A,
                              0x5A,0x75,0x75,0x92,0xBD,0x90,0xD2,0xCA,0x72,0x15,0xB2,0x60,0xB6,0x0A,0xF9,0xCF,
                              0xA8,0x4C,0x59,0xAC,0xDE,0x72,0x8F,0xB1,0x8E,0xCD,0x7E,0x73,0xF7,0x05,0x6E,0x1D,
                              0x83,0x90,0x2B,0x97,0x5C,0xD0,0xCB,0x59,0x81,0x60,0x86,0xCC,0x12,0x2F,0xFD,0x59};
const u8 cccTempPriKey[32] = {0x0D,0x1E,0xD1,0x93,0x6D,0xD6,0x87,0x9E,0x10,0xAC,0x3A,0xFF,0x26,0x7E,0x42,0xF7,
                              0x3B,0xBE,0x42,0x7F,0xB3,0x98,0xE7,0xBB,0xBC,0x1C,0xD7,0x02,0x55,0xBD,0x9A,0xCC};

const u8 material[10] = {'s','e','c','u','r','i','t','y','_','s'};
const u8 saltBuf[4] =   {0x00,0x00,0x00,0x01};

se_txt_t se_txt;
u8 seMessageType;
u8 cccSeTempBuffer[256];

cccResult_t ble_ccc_se_message_send(u8* outBuffer, u16 length);
void ble_ccc_se_bus_init(void);
/**
 * @brief
 *      SE模块初始化      
 * @note
 */
void ble_ccc_se_init(void)
{
    seMessageType = DK_MSG_TYPE_SE;

    core_mm_set(&se_txt,0x00,sizeof(se_txt));

    se_txt.se_select.reqLength = 18U;
    core_mm_copy(se_txt.se_select.reqApdu,selectApduBuffer,se_txt.se_select.reqLength);
    /*SE SDK初始化*/
    se_sdk_init();
    ble_ccc_se_bus_init();
}

void ble_ccc_se_cmd_select(void)
{
    RTE_SE_Send_APDU(se_txt.se_select.reqApdu,se_txt.se_select.reqLength);
    se_txt.se_select.resLength = RTE_SE_Recv_APDU(se_txt.se_select.resApdu);

    if(se_txt.se_select.resLength <= 2)
    {
        RTE_SE_Send_APDU(se_txt.se_select.reqApdu,se_txt.se_select.reqLength);
        se_txt.se_select.resLength = RTE_SE_Recv_APDU(se_txt.se_select.resApdu);
    }
    
    se_txt.seSelectFlag = 1U;
}

void ble_ccc_se_cmd_setup_sec_channle(void)
{
    ecdhPrivateKey_t ecdhPrivateKey;
    ecdhPublicKey_t  ecdhPublicKey;
    ecdhDhKey_t     ecdhDhKey;
    u8 sePubKey[65];
    u8 tmpBuffer[80];
    u8 seRandom[32];
    u16 apduLength;
    // ECDH_P256_GenerateKeys(&ecdhPublicKey,&ecdhPrivateKey);
    // LOG_L_S_HEX(BLE_MD,"Gen Private Key",ecdhPrivateKey.raw_8bit,32);
    // LOG_L_S_HEX(BLE_MD,"Gen Public Key",ecdhPublicKey.raw,64);

    /*拼Setup Security Channle 请求APDU*/
    core_mm_copy(tmpBuffer,setupSecChannelApdhuBuffer_req,5);
    tmpBuffer[5] = 0xC0;
    tmpBuffer[6] = 0x41;
    core_mm_copy(tmpBuffer+7,cccTempPubKey,0x41);
    apduLength = 0x41+2+5;

    RTE_SE_Send_APDU(tmpBuffer,apduLength);
    apduLength = RTE_SE_Recv_APDU(tmpBuffer);

    if (apduLength != 87)
    {
        LOG_L_S(BLE_MD," Setup Security Channel Requst Error!!!\r\n");
    }
    seRandom[0] = 0xC1;
    seRandom[1] = 0x10;
    core_mm_copy(seRandom+2,tmpBuffer+2,16);

    core_mm_copy(sePubKey,tmpBuffer+20,65);
    /*ECDH*/
    /*TODO:*/
    /*Sab = ECDH(BLE_ePK, SE_eSK) =  ECDH(SE_ePK, BLE_eSK)*/
    LOG_L_S_HEX(BLE_MD,"Vehicle Private Key",cccTempPriKey,32);
    LOG_L_S_HEX(BLE_MD,"SE Public Key",sePubKey+1,64);
    core_algo_swap_u8(ecdhPrivateKey.raw_8bit, cccTempPriKey,32);
    core_algo_swap_u8(ecdhPublicKey.raw,sePubKey+1,32);
    core_algo_swap_u8(ecdhPublicKey.raw+32,sePubKey+1+32,32);
    // core_mm_copy(ecdhPublicKey.raw, sePubKey+1,64);
    ECDH_P256_ComputeDhKey(&ecdhPrivateKey,&ecdhPublicKey,&ecdhDhKey);
    core_algo_swap_u8(tmpBuffer,ecdhDhKey.raw,32);
    core_algo_swap_u8(tmpBuffer+32,ecdhDhKey.raw+32,32);
    LOG_L_S_HEX(BLE_MD,"ECDH Key",tmpBuffer,64);
    /*Kdh = sha_256(Sab.x + '00000001' + material)*/
    core_mm_set(tmpBuffer,0x00,80);
    core_algo_swap_u8(tmpBuffer,ecdhDhKey.raw,32);/*Sab.x*/
    //core_mm_copy(tmpBuffer,ecdhDhKey.raw,32);/*Sab.x*/
    core_mm_copy(tmpBuffer+32,saltBuf,4);/*salt*/
    core_mm_copy(tmpBuffer+32+4,material,10);/*material*/
    LOG_L_S_HEX(BLE_MD,"Sab.x+'00000001'+material",tmpBuffer,32+4+10);
    core_algo_sha256Initial();
    core_algo_sha256Update(tmpBuffer,32+4+10);
    core_algo_sha256Final(tmpBuffer);
    LOG_L_S_HEX(BLE_MD,"KDH",tmpBuffer,32);
    /*AES_KEY =Kdh1-16 ,AES_ICV = Kdh17-32*/
    core_mm_copy(se_txt.aesKey,tmpBuffer,16);
    core_mm_copy(se_txt.aesICV,tmpBuffer+16,16);

    /*拼Setup Security Channle 确认 APDU*/
    LOG_L_S_HEX(BLE_MD,"seRandom",seRandom,18);
    apduLength = core_algo_aes_cbc_cipher_padding_M2(MODE_ENCRYPT,se_txt.aesKey,16,16,se_txt.aesICV,16,seRandom,tmpBuffer+5,18);
    core_mm_copy(tmpBuffer,setupSecChannelApdhuBuffer_sure,5);
    apduLength += 5;
    RTE_SE_Send_APDU(tmpBuffer,apduLength);
    apduLength = RTE_SE_Recv_APDU(tmpBuffer);
    if (apduLength != 2U)
    {
        LOG_L_S(BLE_MD," Setup Security Channel Sure Error!!!\r\n");
    }
    se_txt.seSetupSecChannelFlag = 1U;
}
void ble_ccc_se_bus_init(void)
{
    /*第一步: select SE applet*/
    ble_ccc_se_cmd_select();
    /*第二步: se_setup_sec_channel*/
    ble_ccc_se_cmd_setup_sec_channle();
}

void ble_ccc_se_export_ursk(u8* outBuffer, u16* length)
{
    u8 tmpBuffer[80];
    u16 apduLength;
    u8 cnt = 0U;
    apduLength = 5U;
    LOG_L_S(BLE_MD," Begin Export URSK !!!\r\n");
    core_mm_copy(tmpBuffer,exportURSKBuffer,apduLength);
    RTE_SE_Send_APDU(tmpBuffer,apduLength);
    apduLength = RTE_SE_Recv_APDU(tmpBuffer);
    if (apduLength <= 2U)
    {
        LOG_L_S(BLE_MD," Export URSK Error!!!\r\n");
        core_dcm_writeBig16(outBuffer,SW_OTHER_RESION);
        *length = 2U;
        return;
    }
    LOG_L_S_HEX(BLE_MD," AESKey: \r\n",se_txt.aesKey,16U);
    LOG_L_S_HEX(BLE_MD," AESICV: \r\n",se_txt.aesICV,16U);
    apduLength = core_algo_aes_cbc_cipher_padding_M2(MODE_DECRYPT,se_txt.aesKey,16U,16U,se_txt.aesICV,16U,tmpBuffer,cccSeTempBuffer,apduLength-2);
    LOG_L_S_HEX(BLE_MD," Decryto Data: \r\n",cccSeTempBuffer,apduLength);

    core_mm_copy(se_txt.se_ursk.transaction_id,cccSeTempBuffer+2,16U);
    core_mm_copy(se_txt.se_ursk.ursk,cccSeTempBuffer+20,32U);

    for (u8 i = 0U; i < CCC_MAX_URSK_NUMBER; i++)
    {
        if(ble_ccc_ctx.ble_ccc_ursk[i].validFlag == 1U)
        {
            cnt++;
        } 
        else
        {
            ble_ccc_ctx.ble_ccc_ursk[i].validFlag = 1U;
            core_mm_copy(ble_ccc_ctx.ble_ccc_ursk[i].sessionId,cccSeTempBuffer+2+12,4U);//
            LOG_L_S_HEX(BLE_MD," Session Id: \r\n",ble_ccc_ctx.ble_ccc_ursk[i].sessionId,4U);
            core_mm_copy(ble_ccc_ctx.ble_ccc_ursk[i].ursk,cccSeTempBuffer+20,32U);
            LOG_L_S_HEX(BLE_MD," URSK: \r\n",ble_ccc_ctx.ble_ccc_ursk[i].ursk,32U);
            break;
        }
    }
    if (cnt == CCC_MAX_URSK_NUMBER)
    {
        LOG_L_S(BLE_MD," URSK Is Full!!!\r\n");
        core_dcm_writeBig16(outBuffer,SW_URSK_FULL);
        *length = 2U;
        return;
    }
    se_txt.seExportURSKFlag = 1U;
    core_dcm_writeBig16(outBuffer,SW_SUCCESS);
    *length = 2U;
    ble_ccc_notify_uwb_wakeup();
    return;
}

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
cccResult_t ble_ccc_rke_send_verify_apdu(u8* inData, u16 inLength, u8* outData,u16* outLength)
{
    // u8 tmpApduBuf[35];
    u8 apduLength;
    core_mm_copy(cccSeTempBuffer,rkeVerifyBuffer,5);
    apduLength = 5;
    core_dcm_writeBig16(cccSeTempBuffer+apduLength,CCC_RKE_REQ_ACTION);/*0x7F70*/
    apduLength += 2;
    core_dcm_writeU8(cccSeTempBuffer+apduLength,0x07);/*lengh*/
    apduLength++;
    core_dcm_writeU8(cccSeTempBuffer+apduLength,0x80);/*Function Id tag*/
    apduLength++;
    core_dcm_writeU8(cccSeTempBuffer+apduLength,0x02);/*length*/
    apduLength++;
    core_dcm_writeBig16(cccSeTempBuffer+apduLength,ccc_evt_rke_status.functionId);/*function id*/
    apduLength += 2;
    core_dcm_writeU8(cccSeTempBuffer+apduLength,0x81);/*action id*/
    apduLength++;
    core_dcm_writeU8(cccSeTempBuffer+apduLength,0x01);/*length*/
    apduLength++;
    core_dcm_writeU8(cccSeTempBuffer+apduLength,ccc_evt_rke_status.actionId);/*action id*/
    apduLength++;

    core_dcm_writeU8(cccSeTempBuffer+apduLength,0xC0);/**/
	apduLength++;
	core_dcm_writeU8(cccSeTempBuffer+apduLength,inLength);/**/
	apduLength++;
	core_mm_copy(cccSeTempBuffer+apduLength,inData,inLength);
	apduLength += inLength;
	cccSeTempBuffer[4] = apduLength - 5U;

    RTE_SE_Send_APDU(cccSeTempBuffer,apduLength);
    apduLength = RTE_SE_Recv_APDU(cccSeTempBuffer);
    if (apduLength <= 2U)
    {
        LOG_L_S(BLE_MD," RKE Challenge Verify Error!!!\r\n");
    }
    core_mm_copy(outData,cccSeTempBuffer,apduLength-2);
    *outLength = apduLength - 2;
    return CCC_RESULT_SUCCESS;
}


/**
 * @brief
 *      解析SE message 里的数据，管控当前的流程
 * @param [inData]     	    传入的数据缓存
 * @param [inLength]     	传入的数据长度
 * @param [outData]     	输出的数据缓存
 * @param [outLength]     	输出的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_se_parse_apdu(u8* inData, u16 inLength, u8* outData,u16* outLength)
{
#define APDU_CLA    inData[0]
#define APDU_INS    inData[1]
#define APDU_P1     inData[2]
#define APDU_P2     inData[3]
#define APDU_P3     inData[4]
#define APDUB       (inData + 5)
    switch (APDU_INS)
    {
    case SE_INS_SELECT:
        core_mm_copy(outData,se_txt.se_select.resApdu,se_txt.se_select.resLength);
        *outLength = se_txt.se_select.resLength;
        return CCC_RESULT_ERROR_SE_NOT_SEND;
    case SE_INS_AUTH0:
    case SE_INS_AUTH1:
    case SE_INS_EXCHANGE:
        break;   
    case SE_INS_CONTROL_FLOW:
        if ((APDU_P1 == 0x01)&&(APDU_P2 == 0x02))
        {
            se_txt.seAuthFlag = 1U;
        }
        core_dcm_writeBig16(outData,0x9000);
        *outLength = 2;
        return CCC_RESULT_ERROR_SE_NOT_SEND;  
    case SE_INS_CREATE_RANGING_KEY:
        ble_ccc_se_export_ursk(outData,outLength);
        return CCC_RESULT_ERROR_SE_NOT_SEND;
    case SE_INS_DELETE_RANGING_KEY:
        break;                       
    default:
        break;
    }
    return CCC_RESULT_SUCCESS;
}


/**
 * @brief
 *      解析收到的DK数据
 * @param [dkMessage]     ccc_dk_message_txt_t收到的报文数据结构体
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_se_message_recv(ccc_dk_message_txt_t* dkMessage)
{
    cccResult_t cccResult;
    u8 tmpBuffer[256];
    u16 tmpLength;
    seMessageType = dkMessage->messageHeader;/*Framework Or SE*/
    switch (dkMessage->payloadHeader)
    {
    case DK_SE_APDU_RQ:
        cccResult = ble_ccc_se_parse_apdu(dkMessage->data,dkMessage->length,tmpBuffer,&tmpLength);
        if (cccResult == CCC_RESULT_ERROR_SE_NOT_SEND)
        {
            RTE_SE_Recv_APDU_CB(tmpBuffer,tmpLength);
        }
        else
        {
            RTE_SE_Send_APDU(dkMessage->data,dkMessage->length);
            tmpLength = RTE_SE_Recv_APDU(tmpBuffer);
            RTE_SE_Recv_APDU_CB(tmpBuffer,tmpLength);
        }
        break;
    default:
        break;
    }
    return cccResult;
}
/**
 * @brief
 *      透传收到的SE响应数据
 * @param [outBuffer]     	待发送的数据缓存
 * @param [length]     		待发送的数据长度
 * @return
 *        
 * @note
 */
cccResult_t ble_ccc_se_recv_apdu_callback(u8* inBuffer, u16 length)
{
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_MSGHEADER,seMessageType);
    core_dcm_writeU8(bleCccSendBuffer+DK_MESSAGE_OFFSET_PAYLOADHEADER,DK_SE_APDU_RS);
    core_dcm_writeBig16(bleCccSendBuffer+DK_MESSAGE_OFFSET_LENGTH,length);
    core_mm_copy(bleCccSendBuffer+DK_MESSAGE_OFFSET_DATA,inBuffer,length);
    return ble_ccc_se_message_send(bleCccSendBuffer,length+4);
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
cccResult_t ble_ccc_se_message_send(u8* outBuffer, u16 length)
{
    return ble_ccc_dk_message_send(outBuffer,length);
}
