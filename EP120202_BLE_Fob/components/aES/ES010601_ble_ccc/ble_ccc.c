//#include <components/aES/ES000501_uwb_fob/Sources/uwb_SDK_Interface.h>
#include "ble_ccc.h"
#include "flash_api_extern.h"
#include "ble_general.h"
#include "ble_ccc_dk.h"
#include "fsl_os_abstraction.h"
#include "fsl_os_abstraction_free_rtos.h"
#include "ble_ccc_sup_service.h"
#include "ble_ccc_uwb.h"
#include "TimersManager.h"
//#include "uwb_SDK_Interface.h"
#include "Board_Config.h"
#include "../ES000501_uwb/Sources/uwb_SDK_Interface.h"



// ccc_time_sycn_t ccc_time_sycn;
ble_ccc_ctx_t ble_ccc_ctx;
u8 gCccGlobleBuffer[2048];

u8 gQueueAllocFlag = 0;
QueueHandle_t  ccc_queue;
// ble_ccc_queue_msg_t* gRecvQueuePtr;

osaTaskId_t gBleCccTaskId = 0;
void BleCccProcess_Task(void* argument);
OSA_TASK_DEFINE(BleCccProcess_Task, 7, 1, 2048, FALSE );

// void ble_ccc_message_recv(u8 *pdata, u16 length);
void ble_ccc_process(void);

extern void KW38_Int_Start(void);
extern volatile u8 intIRQFlag;
/*消息序列初始化*/
void ble_ccc_queue_init(void)
{
   ccc_queue = xQueueCreate(CCC_QUEUE_MAX_NUMBER,sizeof(ble_ccc_queue_msg_t));
   gQueueAllocFlag = 1;
}
boolean ble_ccc_queue_pop(u8* buffer)
{
   BaseType_t result;
   portBASE_TYPE taskToWake = portMAX_DELAY;
   if (__get_IPSR() != 0U)
   {
       result = xQueueReceiveFromISR(ccc_queue, buffer, &taskToWake);
       assert(pdPASS == result);
       portYIELD_FROM_ISR(taskToWake);
   }
   else
   {
       result = xQueueReceive(ccc_queue,buffer, &taskToWake);
   }
   return (result == pdPASS)? TRUE : FALSE;
}

boolean ble_ccc_queue_push(u8* buffer)
{
   BaseType_t result;
   portBASE_TYPE taskToWake = pdFALSE;
   if (__get_IPSR() != 0U)
   {
       result = xQueueSendFromISR(ccc_queue, buffer, &taskToWake);
       assert(pdPASS == result);
       portYIELD_FROM_ISR(taskToWake);
   }
   else
   {
       result = xQueueSend(ccc_queue, buffer, &taskToWake);
   }
   return (result == pdPASS)? TRUE : FALSE;
}

/*任务初始化*/
void ble_ccc_task_init(void)
{
	gBleCccTaskId = OSA_TaskCreate(OSA_TASK(BleCccProcess_Task), NULL);
    if( NULL == gBleCccTaskId )
    {
         panic(0,0,0,0);
         return;
    }
}
tmrTimerID_t uwbTimer = gTmrInvalidTimerID_c;
tmrTimerID_t testStopTimer = gTmrInvalidTimerID_c;
/**初始化***/
void ble_ccc_init(void)
{
    u8 requestData[64] = {0x00};
    uwbTimer = TMR_AllocateTimer();
    testStopTimer = TMR_AllocateTimer();

    core_mm_set(gCccGlobleBuffer,0x00,2048);
    //core_mm_set((u8*)&ccc_time_sycn,0x00,sizeof(ccc_time_sycn));
    core_mm_set((u8*)&ble_ccc_ctx,0x00,sizeof(ble_ccc_ctx));
    ble_ccc_queue_init();
//    ble_ccc_dk_init();
	

    requestData[0]= 0x80U;
    UQ_UWB_SDK_Interface_init(&stUWBSDK);

    stUWBSDK.fpUQDeviceInit(stUWBSDK.fpUQSendMSG);
    stSource.stUCIState.stTimerTools.fpOSDelay = OSA_TimeDelay ;
    stUWBSDK.fpUQDeviceReset(stUWBSDK.fpUQSendMSG);
    stSource.stUCIState.stTimerTools.fpOSDelay(10);
    stUWBSDK.fpUQAnchorWakup(requestData,64U, stUWBSDK.fpUQSendMSG);


    // requestData[0] = 0x00;
    // requestData[1] = 0x00;
    // requestData[2] = 0x00;
    // requestData[3] = 0x03;
    // stUWBSDK.fpUQRangingSessionSetup(requestData,64U);
    // stUWBSDK.fpUQRangingCtrl(UWBRangingOPType_Start, requestData,64U,stUWBSDK.fpUQSendMSG);

    ble_ccc_dk_init();
    
    BleApp_Start();
}


/*****时间同步操作******/

/*重置ce counter*/
void ble_ccc_reset_ce_counter(void)
{
    ble_ccc_ctx.DeviceEventCount = 0;
}
/*增长ce counter*/
void ble_ccc_increase_ce_counter(u16 ceCounter)
{
    ble_ccc_ctx.DeviceEventCount = ceCounter;
}
/*获取ce counter*/
u16 ble_ccc_get_ce_counter(void)
{
    return ble_ccc_ctx.DeviceEventCount;
}

/*清除缓存ursk*/
void ble_ccc_clear_ursk(void)
{
    for (u8 i = 0; i < CCC_MAX_URSK_NUMBER; i++)
    {
        core_mm_set(&ble_ccc_ctx.ble_ccc_ursk[i],0x00,sizeof(ble_ccc_ursk_t));
    }
}

/*通过session id 查找 ursk*/
boolean ble_ccc_find_ursk(u8* sessionId, u8* ursk)
{
    for (u8 i = 0; i < CCC_MAX_URSK_NUMBER; i++)
    {
        if (ble_ccc_ctx.ble_ccc_ursk[i].validFlag == 1U)
        {
            if (core_mm_compare(sessionId,ble_ccc_ctx.ble_ccc_ursk[i].sessionId,4U) == 0U)
            {
                core_mm_copy(ursk,ble_ccc_ctx.ble_ccc_ursk[i].ursk,32U);
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*设置已选中的 ursk 和session id*/
void ble_ccc_set_select_ursk_sessionid(u8* sessionId,u8* ursk)
{
    core_mm_copy(ble_ccc_ctx.selectSessionId,sessionId, 4U);
    core_mm_copy(ble_ccc_ctx.selectUrsk,ursk,32U);
    return ;
}
/*****BLE通讯操作******/

void ble_ccc_l2cap_set_parameter(u8 deviceId, u8 channelId)
{
    ble_ccc_ctx.deviceId = deviceId;
    ble_ccc_ctx.channelId = channelId;
}
u8 ble_ccc_l2cap_get_connectDeviceId(void)
{
    return ble_ccc_ctx.deviceId;
}
u8 ble_ccc_l2cap_get_connectChannelId(void)
{
    return ble_ccc_ctx.channelId;
}
/*设置L2CAP 连接状态*/
void ble_ccc_l2cap_set_connect_status(u8 status)
{
    ble_ccc_ctx.connectStatus = status;
}
/**
 * @brief
 *      收取L2CAP 逻辑通道上数据
 * @param [deviceId]    当前蓝牙连接通道号
 * @param [inData]      收到的数据缓存(DK Message)
 * @param [length]      收到的数据长度
 * @return
 *        1表示成功，其它失败
 * @note
 */
cccResult_t ble_ccc_l2cap_recv_data(u8 deviceId, u8* inData, u16 length)
{
    ble_ccc_send_evt(CCC_EVT_RECV_DATA,deviceId,inData,length);
    return CCC_RESULT_SUCCESS;
}

/**
 * @brief
 *      往L2CAP 逻辑通道上发送数据
 * @param [outData]     待发送的数据缓存(DK Message)
 * @param [length]      待发送的数据长度
 * @return
 *        1表示成功，其它失败
 * @note
 */
cccResult_t ble_ccc_l2cap_send_data(u8* outData, u16 length)
{
    bleResult_t     bleResult;
    if (ble_ccc_ctx.connectStatus == BLE_L2CAP_STATUS_DISCONNECT)
    {
        return CCC_RESULT_FAILED;
    }
    LOG_L_S_HEX(BLE_MD,"L2CAP Send Data:",outData,length);
    bleResult =  RTE_L2ca_SendLeCbData (ble_ccc_ctx.deviceId,ble_ccc_ctx.channelId,outData,length);
    LOG_L_S(BLE_MD,"Send Result:0x%02x \r\n",bleResult);
    if (bleResult == gBleSuccess_c)
    {
        return CCC_RESULT_SUCCESS;
    }
    else
    {
        return CCC_RESULT_FAILED;
    }
#if 0
    bleResult_t     bleResult = gBleSuccess_c;

    if (otapServerData.transferMethod == gOtapTransferMethodAtt_c)
    {
        /* GATT Characteristic to be written without response - OTAP Client Data */
        gattCharacteristic_t    otapDataChar;

        /* Only the value handle element of this structure is relevant for this operation. */
        otapDataChar.value.handle = mPeerInformation.customInfo.otapServerConfig.hData;
        otapDataChar.value.valueLength = 0;
        otapDataChar.cNumDescriptors = 0;

        bleResult = GattClient_CharacteristicWriteWithoutResponse (mPeerInformation.deviceId,
                                                                   &otapDataChar,
                                                                   chunkCmdLength,
                                                                   pChunk);
    }
    else if (otapServerData.transferMethod == gOtapTransferMethodL2capCoC_c)
    {
        bleResult =  L2ca_SendLeCbData (mPeerInformation.deviceId,
                                        otapServerData.l2capPsmChannelId,
                                        pChunk,
                                        chunkCmdLength);
    }
    else
    {
        ; /* For MISRA compliance */
    }

    if (gBleSuccess_c != bleResult)
    {
        /*! A BLE error has occurred - Disconnect */
        (void)Gap_Disconnect (otapClientDevId);
    }
#endif    
}
void spi_set_rst_line_level(uint8_t level);
uint8_t spi_get_int_line_level(void);
void spi_set_csn_line_level(uint8_t level);
uint8_t spi_get_int_line_level(void);

void INT_Set_Flag(void);
void INT_Clr_Flag(void);

void testStopTimerCallBack(void* param)
{
   // stUWBSDK.fpUQDeviceInit(stUWBSDK.fpUQSendMSG);
    //stUWBSDK.fpUQDeviceReset(stUWBSDK.fpUQSendMSG);


//    uint8_t databuff[20];
//    uint16_t datalen = 0x00;
//    INT_Clr_Flag();
//    spi_set_rst_line_level(0);
//    OSA_TimeDelay(6);
//    spi_set_rst_line_level(1);
//    OSA_TimeDelay(3);
//    while(1==spi_get_int_line_level());
//    spi_set_csn_line_level(0);
//    spi_half_duplex_recv(databuff,6);
//    __asm("NOP");

}


void BleCccProcess_Task(void* argument)
{
	ble_ccc_init();
    while(1)
    {
        // OSA_TimeDelay(10);
        ble_ccc_process();
    }
}

u8 ble_ccc_send_evt(ccc_evt_type_t evtType, u8 deviceId, u8* pdata, u16 length)
{
    u8* dataBuff;
    ble_ccc_queue_msg_t cccQueue;
    if(gQueueAllocFlag == 0U)
    {
    	return 0U;
    }
    cccQueue.evtType = evtType;
    if(pdata != NULL)
    {
    	//portENTER_CRITICAL();
		dataBuff = core_platform_alloc(length);
		//portEXIT_CRITICAL();
		if (dataBuff == NULL)
		{
			LOG_L_S(NFC_MD,"Alloc Ram Space Failed!!!\r\n");
			return 0U;
		}
		core_mm_copy(dataBuff,pdata,length);
		cccQueue.dataBuff = dataBuff;
    }
    else
    {
    	cccQueue.dataBuff = pdata;
    }
    cccQueue.deviceId = deviceId;
    cccQueue.length = length;

    if(FALSE == ble_ccc_queue_push((u8*)&cccQueue))
    {
        LOG_L_S(NFC_MD,"Send Evt Failed!!!\r\n");
    }    
    return 1U;
}

u8 ble_ccc_send_leSetPhyRequest(void)
{
    ble_ccc_send_evt(CCC_EVT_LESETPHY,ble_ccc_ctx.deviceId,NULL,0);
    // ble_ccc_ctx.leSetPhyStep = CCC_LESETPHY_STEP0;
}
u8 ble_ccc_notify_uwb_wakeup(void)
{
	u8 requestData[64] = {0x00};
    requestData[0]= 0x80U;
    ble_ccc_send_evt(UWB_EVT_WAKEUP_RQ,ble_ccc_ctx.deviceId,requestData,64);
}

void ble_ccc_process(void)
{
    ble_ccc_queue_msg_t gRecvQueuePtr;
    boolean result;
    gapPhyEvent_t* tCccLePhy;
    result = ble_ccc_queue_pop((u8*)&gRecvQueuePtr);
    if (result == FALSE)
    {   /*从消息序列中未读出数据，则退出*/
        return ;
    }
    
    if (gRecvQueuePtr.evtType == CCC_EVT_IDLE)
    {

    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_STATUS_CONNECT)
    {
        /*TODO:*/
        ble_ccc_reset_ce_counter();
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_STATUS_DISCONNECT)
    {
        ble_ccc_l2cap_set_connect_status(BLE_L2CAP_STATUS_DISCONNECT);

        ble_ccc_disconnect_dk_notify();
        {
            INT_Set_Flag();
            __disable_irq();
            stUWBSDK.fpUQDeviceInit(stUWBSDK.fpUQSendMSG);
            stSource.stUCIState.stTimerTools.fpOSDelay = OSA_TimeDelay ;
            __enable_irq();
            stUWBSDK.fpUQDeviceReset(stUWBSDK.fpUQSendMSG);
        }
    }
    
    else if (gRecvQueuePtr.evtType == CCC_EVT_L2CAP_SETUP_COMPLETE)
    {
        ble_ccc_l2cap_set_connect_status(BLE_L2CAP_STATUS_CONNECT);

        ble_ccc_l2cap_set_parameter(gRecvQueuePtr.deviceId,gRecvQueuePtr.dataBuff[0]);

        // ble_ccc_connect_dk_notify();
        
        ble_ccc_send_evt(CCC_EVT_TIME_SYCN0,gRecvQueuePtr.deviceId,NULL,0U);
    
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_L2CAP_DISCONNECT)
    {
        ble_ccc_l2cap_set_connect_status(BLE_L2CAP_STATUS_DISCONNECT);
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_TIME_SYCN0)
    {
        LOG_L_S(CCC_MD,"First Time Sycn 0 !!!!\r\n");
        ble_ccc_connect_sup_service_notify();
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_TIME_SYCN1)
    {
        /*TODO:*/
        LOG_L_S(CCC_MD,"Second Time Sycn 1 !!!!\r\n");
        ble_ccc_connect_sup_service_notify();
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_LESETPHY)
    {
        if (gRecvQueuePtr.dataBuff == NULL)
        {
            RTE_BLE_GAP_LE_READ_PHY(gRecvQueuePtr.deviceId);
        }
        else
        {
            tCccLePhy = (gapPhyEvent_t* )gRecvQueuePtr.dataBuff;
            if (tCccLePhy->phyEventType == gPhyRead_c)
            {
                RTE_BLE_GAP_LE_SET_PHY(FALSE,tCccLePhy->deviceId,0x03,tCccLePhy->txPhy,tCccLePhy->rxPhy,gLeCodingNoPreference_c);
            }
            else if (tCccLePhy->phyEventType == gPhyUpdateComplete_c)
            {
                /*收到LeSetPhy指令*/    
                /*做一次时间同步*/
                ble_ccc_send_evt(CCC_EVT_TIME_SYCN1,gRecvQueuePtr.deviceId,NULL,0U);
                // ble_ccc_uwb_send_evt(UWB_STEP_RANGING_CAPABILITY_RQ);
            }
            else
            {

            }
        }
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_RECV_DATA)
    {
        ble_ccc_dk_message_recv(gRecvQueuePtr.dataBuff,gRecvQueuePtr.length);
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_SEND_DATA)
    {
        ble_ccc_l2cap_send_data(gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_DK)
    {
        
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_DK_UWB)
    {
        ble_ccc_uwb_process(gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_DK_NOTIFY)
    {
       
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_DK_SE)
    {
       
    }
    else if (gRecvQueuePtr.evtType == CCC_EVT_DK_SUP_SERVICE)
    {
        
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_TIMER_SYCN_RQ)/*时间同步 请求*/
    {
        
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_TIMER_SYCN_RS)/*时间同步 响应*/
    {

    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_WAKEUP_RQ)/*唤醒UWB 请求*/
    {
    	stUWBSDK.fpUQAnchorWakup(gRecvQueuePtr.dataBuff,gRecvQueuePtr.length, stUWBSDK.fpUQSendMSG);
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_SESSION_SETUP_RQ)/*测距会话建立 请求*/
    {
    	//gRecvQueuePtr.dataBuff[0] = 0x00;
    	//gRecvQueuePtr.dataBuff[1] = 0x00;
    	//gRecvQueuePtr.dataBuff[2] = 0x00;
    	//gRecvQueuePtr.dataBuff[3] = 0x03;
        stUWBSDK.fpUQRangingSessionSetup(gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
        ble_ccc_uwb_notify_uwb_sdk_step2();
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_SESSION_START_RQ)/*测距会话开始 请求*/
    {
        //KW38_Int_Start();
    	INT_Clr_Flag();
        stUWBSDK.fpUQRangingCtrl(UWBRangingOPType_Start, gRecvQueuePtr.dataBuff, gRecvQueuePtr.length,stUWBSDK.fpUQSendMSG);

        //PTC1_TOGGLE();
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_INT_NOTICE)/*测距会话开始 响应*/
    {
        if(intIRQFlag == 0x01)
        {
            intIRQFlag = 0U;
            stUWBSDK.fpUQRangingNTFCache(stUWBSDK.fpUQSendMSG);
        }
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_SESSION_SUSPEND_RQ)/*测距会话挂起恢复停止 请求*/
    {
        LOG_L_S(CCC_MD, "ranging control: %d \r\n", gRecvQueuePtr.dataBuff[0U]);
        if(gRecvQueuePtr.dataBuff[0U] == 1U)
        {
            //stUWBSDK.fpUQRangingCtrl(UWBRangingOPType_Stop, NULL, 0U,stUWBSDK.fpUQSendMSG);
        	//portENTER_CRITICAL();
            //INT_Clr_Flag();
            INT_Set_Flag();
           //KW38_INT_Stop();
//            stSource.stUCIState.stTimerTools.fpOSDelay(50);
//            UQ_UWB_SDK_Interface_init(&stUWBSDK);
//            stSource.stUCIState.stTimerTools.fpOSDelay = OSA_TimeDelay ;
//            __disable_irq();
            //TMR_StartLowPowerTimer(testStopTimer, gTmrLowPowerSingleShotMillisTimer_c, 100, testStopTimerCallBack, NULL);
            //__asm("NOP");
             stUWBSDK.fpUQDeviceInit(stUWBSDK.fpUQSendMSG);
             stSource.stUCIState.stTimerTools.fpOSDelay = OSA_TimeDelay ;
             stUWBSDK.fpUQDeviceReset(stUWBSDK.fpUQSendMSG);
            //  stSource.stUCIState.stTimerTools.fpOSDelay(10);
            //  stUWBSDK.fpUQAnchorWakup(requestData,64U, stUWBSDK.fpUQSendMSG);
             
            //portEXIT_CRITICAL();
//            __enable_irq();
        }
        else if(gRecvQueuePtr.dataBuff[0U] == 2U)
        {
            stUWBSDK.fpUQRangingCtrl(UWBRangingOPType_Suspend, NULL, 0U,stUWBSDK.fpUQSendMSG);
        }
        else if(gRecvQueuePtr.dataBuff[0U] == 3U)
        {
            stUWBSDK.fpUQRangingCtrl(UWBRangingOPType_Recover, NULL, 0U,stUWBSDK.fpUQSendMSG);
        }
        else
        {
            
        }
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_SESSION_SUSPEND_RS)/*测距会话挂起恢复停止 响应*/
    {
        
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_RESULT_NOTICE)/*测距结果通知*/
    {
        
    }
    else if (gRecvQueuePtr.evtType == UWB_EVT_RECV_UART)/*测距结果通知*/
    {
        
    }
    else
    {

    }
    /*出栈了需要释放已申请的空间*/
    if(gRecvQueuePtr.dataBuff != NULL)
    {
    	//portENTER_CRITICAL();
    	core_platform_free(gRecvQueuePtr.dataBuff);
    	//portEXIT_CRITICAL();
    }
}

