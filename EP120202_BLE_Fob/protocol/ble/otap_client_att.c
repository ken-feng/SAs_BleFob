/*! *********************************************************************************
* \addtogroup BLE OTAP Client ATT
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the BLE OTAP Client ATT application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"

/* Framework / Drivers */
#include "RNG_Interface.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "Panic.h"
#if (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#endif
#include "OtaSupport.h"
#include "MemManager.h"
/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#if !defined(MULTICORE_APPLICATION_CORE) || (!MULTICORE_APPLICATION_CORE)
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
//#if (gHidService_c)||(gHidService_c)
//#include "hid_interface.h"
//#endif
/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "otap_client_att.h"
#include "EM000401.h"
#include "app_preinclude.h"
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#include "mcmgr.h"
#endif

//#include "lin_cfg.h"
//#include "lin_api.h"
//#include "lin_common.h"
#include "EM000101.h"
#include "EM000401.h"

#include "ble_ccc.h"
/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mBatteryLevelReportInterval_c   (10)        /* battery level report interval in seconds  */

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/


typedef struct advState_tag{
    bool_t      advOn;
    advType_t   advType;
} advState_t;

#if defined(gCccL2Cap_d) && (gCccL2Cap_d == 1)
#define mAppLeCbInitialCredits_c        (32768)
#endif
gapPhyEvent_t     gCccLePhy;/**/
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t mPeerInformation[gAppMaxConnections_c];
appPeerInfo_t* curPeerInformation;

uintn8_t mBleDeviceAddress[6] = { 0, 0, 0, };
uintn8_t mBleConnectStatus = 0;
uintn8_t mPhoneDeviceAddress[6] = { 0, 0, 0, };
uintn8_t  mConnectedDeviceId = 0;/*记录当前连接的蓝牙设备索引号*/
tmrTimerID_t vcPressTimer = gTmrInvalidTimerID_c;

//tmrTimerID_t uwbTimer = gTmrInvalidTimerID_c;
/* Adv Parmeters */
static advState_t  mAdvState;
/* Service Data */
#if (gHidService_c)||(gHidService_c)
static hidConfig_t hidServiceConfig = {(uint16_t)service_hid, (uint8_t)gHid_ReportProtocolMode_c};
static uint16_t cpHandles[] = { (uint16_t)value_hid_control_point };

/* Application Data */
#endif

static uint16_t SeviceWriteNotifHandles[] = {value_yqdk_1,value_persional_1};
#define VALUE_YQDK_IS_WRITEABLE(handle)   ((value_yqdk_1 == handle))
/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);

static void BleApp_Config(void);
static void BleApp_Advertise (void);

/************************************************************************************
*************************************************************************************
* App functions
*************************************************************************************
************************************************************************************/
static void BleApp_PairingSuccessCallback(void *pParam);
static void BleApp_GetRssiCallback(void *pParam);
static void BleApp_DisconnectCallBack(void *pParam);

/***********************************************************************************/
static bleResult_t BleAppSendData(u8 deviceId, u16 serviceHandle,
    u16 charUuid, u16 length, u8 *testData);

/*设置蓝牙广播快慢模式*/
void BleApp_SwitchAdvMode(advType_t mode)
{
    mAdvState.advType = mode;
}
advType_t BleApp_GetAdvMode(void)
{
    return mAdvState.advType;
}


static u8 ascii2u8(u8*  src, u8 length, u8* dest)
{
	u8 i ;
	u8 tmpData;
  for(i = 0;i<length;i++)
  {
    tmpData = (src[i]>>4)&0x0f;
    if((tmpData>=0) && (tmpData<=9))
    {
      dest[2*i] =   tmpData+'0';
    }
    else
    {
      dest[2*i] =   tmpData -0x0A+'A';
    }
    tmpData = src[i]&0x0f;
    if((tmpData>=0) && (tmpData<=9))
    {
      dest[2*i+1] =   tmpData+'0';
    }
    else
    {
      dest[2*i+1] =   tmpData-0x0A+'A';
    }
  }
  return length*2;
}

static void BleAppGetBLEName(u8* bleName)
{
    ascii2u8(mBleDeviceAddress,6,bleName);
    return ;
}

static void BleAppNotifyPairStatus(deviceId_t peer_device_id, u8 status)
{
	bool_t isBonded = FALSE;
    uint8_t nvmIndex = gInvalidNvmIndex_c;
	if (gBleSuccess_c == Gap_CheckIfBonded(peer_device_id, &isBonded,&nvmIndex) && isBonded) 
    {
		status |= 0x01;
	}
	LOG_L_S(BLE_MD, "Ble Pair State=%x\n", status);
	//BleAppSendData(peer_device_id, service_pair_state, 0xFFE1, sizeof(status), &status);
}



static bleResult_t BleAppSendData(u8 deviceId, u16 serviceHandle,
    u16 charUuid, u16 length, u8 *pdata)
{
    u16  handle;
    bleResult_t result;
    u16  handleCccd;
    bool_t isNotifActive;
    bleUuid_t uuid;

    if (gInvalidDeviceId_c == mPeerInformation[deviceId].deviceId) 
    {
        return gBleInvalidState_c;
    }

    uuid.uuid16 = charUuid;
    /* Get handle of  characteristic */
    result = GattDb_FindCharValueHandleInService(serviceHandle, gBleUuidType16_c, &uuid, &handle);    
    if (result != gBleSuccess_c) 
    {
        LOG_L_S(BLE_MD, "uuid %x not find\n", charUuid);
        return result;
    }

    /* Write characteristic value */
    result = GattDb_WriteAttribute(handle, length, pdata);
    if (result != gBleSuccess_c) 
    {
        LOG_L_S(BLE_MD, "uuid %x wr err %x\n", charUuid, result);
        return result;
	}

    /* Get handle of CCCD */
    if ((result = GattDb_FindCccdHandleForCharValueHandle(handle, &handleCccd)) != gBleSuccess_c) 
    {
        LOG_L_S(BLE_MD, "uuid %x cccd err %x\n", charUuid, result);
        return result;
	}

    result = Gap_CheckNotificationStatus(deviceId, handleCccd, &isNotifActive);
    if ((gBleSuccess_c == result) && (TRUE == isNotifActive)) 
    {
        result = GattServer_SendInstantValueNotification(deviceId, handle, length, pdata);
        if (result != gBleSuccess_c)
        {
            LOG_L_S(BLE_MD, "ble send value notify: did=[%d], handle=[0x%x] result=[0x%x]\n", \
                deviceId, handle, result);
        }
    } 
    else
    {
    	/*
    	 * todo: retry / resend  3
    	 * */
    	LOG_L_S(BLE_MD, "NotificationStatus Failed: deviceId=[%d], handleCccd=[0x%x] isNotifActive=[0x%x]\n", \
    			deviceId, handleCccd, isNotifActive);
    }
    return result;
}


/***********************************************************************************/

static void BleApp_ConnectionTimeoutCallback(void *pParam)
{
    uintn8_t index;
    //uintn8_t status;
    if (pParam == NULL)
    {
        return ;
    }
    
    index = *(uintn8_t *)pParam;
	if (gInvalidDeviceId_c != mPeerInformation[index].deviceId) 
    {
        LOG_L_S(CAN_MD,"BLE Connection Timeout !!!\r\n");
		Gap_Disconnect(mPeerInformation[index].deviceId);

	}
}






/***********************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void)
{
    /* Initialize application support for drivers */
    BOARD_InitAdc();

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    /* Init eRPC host */
    init_erpc_host();
#endif

    /* Initialize application specific peripheral drivers here. */
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    Gap_ReadPublicDeviceAddress();
}

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
static void BleApp_HandleControllerNotification(bleNotificationEvent_t *pNotificationEvent)
{
    switch(pNotificationEvent->eventType)
    {
        case gNotifEventNone_c:
        {
            LOG_L_S(BLE_MD, "Configured notification status:0x%02x \r\n", pNotificationEvent->status);
            break;
        }

        case gNotifConnEventOver_c:
        {
            LOG_L_S(BLE_MD, "CONN Event Over device :0x%02x, on channel:0x%02x,with RSSI:0x%02x,and event counter:0x%04x \r\n",
            		pNotificationEvent->deviceId, pNotificationEvent->channel,
					(uint8_t)pNotificationEvent->rssi, (uint16_t)pNotificationEvent->ce_counter);
            break;
        }

        case gNotifConnRxPdu_c:
        {
            //LOG_L_S(BLE_MD, "CONN Rx PDU from device:0x%02x, on channel:0x%02x, with RSSI:0x%02x, with event counter:0x%04x, and timestamp:0x%04x \r\n",
            //		pNotificationEvent->deviceId, pNotificationEvent->channel,(uint8_t)pNotificationEvent->rssi,
            //		(uint16_t)pNotificationEvent->ce_counter, pNotificationEvent->timestamp);
            ble_ccc_increase_ce_counter(pNotificationEvent->ce_counter);
            break;
        }

        case gNotifAdvEventOver_c:
        {
            LOG_L_S(BLE_MD, "ADV Event Over.\n\r");
            break;
        }

        case gNotifAdvTx_c:
        {
            LOG_L_S(BLE_MD, "ADV Tx on channel :0x%02x \r\n", pNotificationEvent->channel);
            break;
        }

        case gNotifAdvScanReqRx_c:
        {
            LOG_L_S(BLE_MD, "ADV Rx Scan Req on channel:0x%02x  with RSSI:0x%02x \r\n", pNotificationEvent->channel, (uint8_t)pNotificationEvent->rssi);
            break;
        }

        case gNotifAdvConnReqRx_c:
        {
            LOG_L_S(BLE_MD, "ADV Rx Conn Req on channel:0x%02x  with RSSI:0x%02x \r\n", pNotificationEvent->channel, (uint8_t)pNotificationEvent->rssi);
            break;
        }

        case gNotifScanEventOver_c:
        {
            LOG_L_S(BLE_MD, "SCAN Event Over on channel:0x%02x \r\n ", pNotificationEvent->channel);
            break;
        }

        case gNotifScanAdvPktRx_c:
        {
            LOG_L_S(BLE_MD, "SCAN Rx Adv Pkt on channel:0x%02x  with RSSI:0x%02x \r\n", pNotificationEvent->channel, (uint8_t)pNotificationEvent->rssi);
            break;
        }

        case gNotifScanRspRx_c:
        {
            LOG_L_S(BLE_MD, "SCAN Rx Scan Rsp on channel:0x%02x  with RSSI:0x%02x \r\n ", pNotificationEvent->channel, (uint8_t)pNotificationEvent->rssi);
            break;
        }

        case gNotifScanReqTx_c:
        {
            LOG_L_S(BLE_MD, "SCAN Tx Scan Req on channel:0x%02x \r\n ", pNotificationEvent->channel);
            break;
        }

        case gNotifConnCreated_c:
        {
            LOG_L_S(BLE_MD, "CONN Created with device:0x%02x with timestamp:0x%04x\r\n", pNotificationEvent->deviceId, pNotificationEvent->timestamp);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
}
#endif


#if defined(gCccL2Cap_d) && (gCccL2Cap_d == 1)
/*! *********************************************************************************
* \brief        Callback for incoming credit based data.
*
* \param[in]    deviceId        The device ID of the connected peer that sent the data
* \param[in]    lePsm           Channel ID
* \param[in]    pPacket         Pointer to incoming data
* \param[in]    packetLength    Length of incoming data
********************************************************************************** */
static void BleApp_L2capPsmDataCallback (deviceId_t     deviceId,
                                         uint16_t       lePsm,
                                         uint8_t*       pPacket,
                                         uint16_t       packetLength)
{
    LOG_L_S(BLE_MD,"L2CAP lePsm:0x%04x deviceId:0x%02x \r\n",lePsm,deviceId);
    // OtapClient_HandleDataChunk (deviceId,packetLength,pPacket);
    LOG_L_S_HEX(BLE_MD,"L2CAP Receive Data:",pPacket,packetLength);
    ble_ccc_l2cap_recv_data(deviceId,pPacket,packetLength);
}

/*! *********************************************************************************
* \brief        Callback for control messages.
*
* \param[in]    pMessage    Pointer to control message
********************************************************************************** */
static void BleApp_L2capPsmControlCallback(l2capControlMessage_t* pMessage)
{
    u8 tmpBuffer[32];
    bleResult_t bleResult;
    LOG_L_S(BLE_MD,"l2capControlMessageType: 0x%02x \r\n",pMessage->messageType);
    switch (pMessage->messageType)
    {
        case gL2ca_LePsmConnectRequest_c:
        {
            l2caLeCbConnectionRequest_t *pConnReq = &pMessage->messageData.connectionRequest;

            /* This message is unexpected on the OTAP Client, the OTAP Client sends L2CAP PSM connection
             * requests and expects L2CAP PSM connection responses.
             * Disconnect the peer. */
            //(void)Gap_Disconnect (pConnReq->deviceId);
            /* Respond to the peer L2CAP CB Connection request - send a connection response. */
            bleResult = L2ca_ConnectLePsm (pConnReq->lePsm,
                               pConnReq->deviceId,
                               pConnReq->initialCredits);
            LOG_L_S(BLE_MD,"Result: 0x%02x L2ca_ConnectLePsm: 0x%04x DeviceId: 0x%02x initialCredits: 0x%04x\r\n",
                    bleResult,pConnReq->lePsm,pConnReq->deviceId,pConnReq->initialCredits);                   
            break;
        }
        case gL2ca_LePsmConnectionComplete_c:
        {
            l2caLeCbConnectionComplete_t *pConnComplete = &pMessage->messageData.connectionComplete;

            /* Call the application PSM connection complete handler. */
            // OtapClient_HandlePsmConnectionComplete (pConnComplete);
            if (pMessage->messageData.connectionComplete.result == gSuccessful_c)
            {   
                LOG_L_S(BLE_MD,"L2CAP Connect Success,peerMps: 0x%04x mtu: 0x%04x initialCredits: 0x%04x DeviceId: 0x%02x Channel Id:0x%04x \r\n",
                    pMessage->messageData.connectionComplete.peerMps,pMessage->messageData.connectionComplete.peerMtu,pMessage->messageData.connectionComplete.initialCredits,
                    pMessage->messageData.connectionComplete.deviceId,pMessage->messageData.connectionComplete.cId);
                // ble_ccc_l2cap_set_parameter(pMessage->messageData.connectionComplete.deviceId,
                //                         pMessage->messageData.connectionComplete.cId);
                // ble_ccc_l2cap_set_connect_status(BLE_L2CAP_STATUS_CONNECT);   
                // ble_ccc_connect_dk_notify();
                tmpBuffer[0] = pMessage->messageData.connectionComplete.cId;
                ble_ccc_send_evt(CCC_EVT_L2CAP_SETUP_COMPLETE,pMessage->messageData.connectionComplete.deviceId,tmpBuffer,1U);
                // LOG_L_S(BLE_MD,"Gap_LeSetPhy !!!\r\n");
                // (void)Gap_LeSetPhy(TRUE, pMessage->messageData.connectionComplete.deviceId, 0, gLePhy1MFlag_c, gLePhy1MFlag_c, 0);
            }
            else
            {
                LOG_L_S(BLE_MD,"L2CAP Connect Failed, result : 0x%02x \r\n",pMessage->messageData.connectionComplete.result );
                /*如果当前L2CAP建立失败，则断开蓝牙连接*/
                (void)Gap_Disconnect (pMessage->messageData.connectionComplete.deviceId);
                // ble_ccc_l2cap_set_connect_status(BLE_L2CAP_STATUS_DISCONNECT);
                // //ble_ccc_evt_notify_sdk(SDK_EVT_TAG_BLE_DISCONNECT,pMessage->messageData.connectionComplete.deviceId,NULL_PTR,0U);
                // ble_ccc_disconnect_dk_notify();
                ble_ccc_send_evt(CCC_EVT_L2CAP_DISCONNECT,pMessage->messageData.connectionComplete.deviceId,NULL,0U);
               
            }
            break;
        }
        case gL2ca_LePsmDisconnectNotification_c:
        {
            l2caLeCbDisconnection_t *pCbDisconnect = &pMessage->messageData.disconnection;

            /* Call the application PSM disconnection handler. */
            // OtapClient_HandlePsmDisconnection (pCbDisconnect);
            // ble_ccc_l2cap_set_connect_status(BLE_L2CAP_STATUS_DISCONNECT);
            ble_ccc_send_evt(CCC_EVT_L2CAP_DISCONNECT,pMessage->messageData.disconnection.deviceId,NULL,0U);
            break;
        }
        case gL2ca_NoPeerCredits_c:
        {
            l2caLeCbNoPeerCredits_t *pCbNoPeerCredits = &pMessage->messageData.noPeerCredits;
            (void)L2ca_SendLeCredit (pCbNoPeerCredits->deviceId,
                               ble_ccc_l2cap_get_connectChannelId(),//otapClientData.l2capPsmChannelId,
                               mAppLeCbInitialCredits_c);
            break;
        }
        case gL2ca_LocalCreditsNotification_c:
        {
            l2caLeCbLocalCreditsNotification_t *pMsg = &pMessage->messageData.localCreditsNotification;

            break;
        }
        case gL2ca_Error_c:
        {
            /* Handle error */
            LOG_L_S(BLE_MD,"L2CAP Connect Failed, result : 0x%02x \r\n",pMessage->messageData.error.errorSource );
            /*如果当前L2CAP建立失败，则断开蓝牙连接*/
            (void)Gap_Disconnect (pMessage->messageData.connectionComplete.deviceId);
            break;
        }
        default:
            ; /* For MISRA compliance */
            break;
    }
}
#endif

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    u8 evtByte;
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    //LOG_L_S(BLE_MD, "Generic EVT=[%d]\n", pGenericEvent->eventType);
    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            BleApp_Config();
        }
        break;

        case gPublicAddressRead_c:
        {
#if (OTAP_ROLE == OTAP_PERIPHERALS)              
            mBleDeviceAddress[0] = pGenericEvent->eventData.aAddress[5];
            mBleDeviceAddress[1] = pGenericEvent->eventData.aAddress[4];
            mBleDeviceAddress[2] = pGenericEvent->eventData.aAddress[3];
            mBleDeviceAddress[3] = pGenericEvent->eventData.aAddress[2];
            mBleDeviceAddress[4] = pGenericEvent->eventData.aAddress[1];
            mBleDeviceAddress[5] = pGenericEvent->eventData.aAddress[0];
            LOG_L_S(BLE_MD, "BLE MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mBleDeviceAddress[0], mBleDeviceAddress[1], \
                mBleDeviceAddress[2], mBleDeviceAddress[3], \
                mBleDeviceAddress[4], mBleDeviceAddress[5]);

            /* Set advertising parameters*/
            gAdvParams.filterPolicy = gProcessAll_c;       
#if (cPWR_UsePowerDownMode)
            PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
#endif            
            //Gap_SetAdvertisingParameters(&gAdvParams);
            BleApp_Advertise();
#endif            
        }
        break;

        case gAdvertisingParametersSetupComplete_c:
        {
            u32 passkey = 0;//, random = 0;
            u8 device_name[13] = { 0, 0, 0, };
            //u8 status = 0;
			BleAppGetBLEName(device_name);
			advDataUpdate(device_name, 12);
            Gap_SetLocalPasskey(passkey);
            /*设置DEVICE NAME*/
            uint16_t valueHandle = 0, serviceHandle = 0, length;
            bleUuid_t   uuid;
            bleResult_t result;
#if 0//APP_DEBUG_MODE            
            u8 deviceName[20] = {'E', 'P', '0', '1', '0', '2', '0', '1',
                                 '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
#else
            u8 deviceName[18] = {'B', 'I', 'C', 'O', 'S', 'E', 
	                             '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
#endif                                 
            uuid.uuid16 = gBleSig_GenericAccessProfile_d;
            (void)GattDb_FindServiceHandle(1, gBleUuidType16_c, &uuid, &serviceHandle);
            uuid.uuid16 = gBleSig_GapDeviceName_d;
            (void)GattDb_FindCharValueHandleInService(serviceHandle, gBleUuidType16_c, &uuid, &valueHandle);
#if 0//APP_DEBUG_MODE               
            BleAppGetBLEName(deviceName+8);
            result =  GattDb_WriteAttribute(valueHandle, 20, deviceName);
#else
            BleAppGetBLEName(deviceName+6);
            result =  GattDb_WriteAttribute(valueHandle, 18, deviceName);
#endif             
            if (result != gBleSuccess_c)
            {
                LOG_L_S(BLE_MD, "set up device name %x wr err %x\n", valueHandle, result);
            }
            /*开始广播*/
            (void)Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);

            LOG_L_S(BLE_MD, "Advertising Device Name:[EP010201%s]\n", device_name);
            LOG_L_S(BLE_MD, "Start Advertising, advMode:%d \n",mAdvState.advType);
        }
        break;

        case gAdvertisingDataSetupComplete_c:
        {
            (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
        case gControllerNotificationEvent_c:
        {
            BleApp_HandleControllerNotification(&pGenericEvent->eventData.notifEvent);
        }
        break;
#endif
        case gLePhyEvent_c:
        {
            LOG_L_S(BLE_MD,"gLePhyEvent_c....phyEventType:%d deviceId:%d txPhy:%d rxPhy:%d\r\n",
                    pGenericEvent->eventData.phyEvent.phyEventType,
                    pGenericEvent->eventData.phyEvent.deviceId,
                    pGenericEvent->eventData.phyEvent.txPhy,
                    pGenericEvent->eventData.phyEvent.rxPhy);
            core_mm_copy(&gCccLePhy,&pGenericEvent->eventData.phyEvent,sizeof(gapPhyEvent_t));
            ble_ccc_send_evt(CCC_EVT_LESETPHY,pGenericEvent->eventData.phyEvent.deviceId,&gCccLePhy,sizeof(gapPhyEvent_t));
        }
        break;
        case gLeScLocalOobData_c:
        {
            //core_mm_copy(&gCccLeScOobData,&pGenericEvent->eventData.localOobData,sizeof(gapLeScOobData_t));
            LOG_L_S_HEX(BLE_MD,"OOB Data:r (Random value)",pGenericEvent->eventData.localOobData.randomValue,gSmpLeScRandomValueSize_c);
            LOG_L_S_HEX(BLE_MD,"OOB Data:Cr (Random Confirm value)",pGenericEvent->eventData.localOobData.confirmValue,gSmpLeScRandomConfirmValueSize_c);
        }
        break;
        default:
            ; /* For MISRA compliance */
            break;
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config(void)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    if (GattDbDynamic_CreateDatabase() != gBleSuccess_c)
    {
        panic(0,0,0,0);
        return;
    }
#endif /* MULTICORE_APPLICATION_CORE */

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

 #if defined(gCccL2Cap_d) && (gCccL2Cap_d == 1)
    /* Register CCC L2CAP PSM */
    L2ca_RegisterLePsm (gCcc_L2capLePsm_c,
                        gCccCmdMaxLength_c);  /*!< The negotiated MTU must be higher than the biggest data chunk that will be sent fragmented */

    /* Register stack callbacks */
    (void)App_RegisterLeCbCallbacks(BleApp_L2capPsmDataCallback, BleApp_L2capPsmControlCallback);
#endif

    /* Register stack callbacks */
    (void)App_RegisterGattServerCallback (BleApp_GattServerCallback);
    GattServer_RegisterHandlesForWriteNotifications (NumberOfElements(SeviceWriteNotifHandles),
                                                         SeviceWriteNotifHandles);
#if (gHidService_c)||(gHidService_c)														 
	GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
#endif
	for (uint8_t i = 0; i < gAppMaxConnections_c; i++)
	{
		mPeerInformation[i].deviceId= gInvalidDeviceId_c;
	}
#if (gHidService_c)||(gHidService_c)	
	(void)Hid_Start(&hidServiceConfig);
#endif
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
#if defined(gUseControllerNotificationsCallback_c) && (gUseControllerNotificationsCallback_c)
    Controller_RegisterEnhancedEventCallback(BleApp_ControllerNotificationCallback);
#endif
#endif 

#if (cPWR_UsePowerDownMode)
    PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
#endif

    vcPressTimer = TMR_AllocateTimer();

//    uwbTimer = TMR_AllocateTimer();
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will satrt after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    switch (mAdvState.advType)
    {
        case fastAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
        }
        break;

        case slowAdvState_c:
        {
            gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.maxInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
        }
        break;

        default:
            ; /* For MISRA compliance */
        break;
    }

    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAdvParams);
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;

            if(mAdvState.advOn)
            {
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        default:
            ; /* For MISRA compliance */
            break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent)
{
    uintn8_t index = peerDeviceId;
    uint8_t nvmIndex = gInvalidNvmIndex_c; 
    if (pConnectionEvent->eventType != gConnEvtRssiRead_c)
    {
        LOG_L_S(BLE_MD, "Connection CB: DID=[%d] EVT=[0x%x]\n", peerDeviceId, pConnectionEvent->eventType);
    }
    
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
#if (gHidService_c)||(gHidService_c)		
			 (void)Hid_Subscribe(peerDeviceId);
#endif			 
            bool_t isBonded = FALSE;
            // if (gBleSuccess_c == Gap_CheckIfBonded(peerDeviceId, &isBonded,&nvmIndex) &&
            //     TRUE == isBonded) 

            /* Subscribe client*/
            mPeerInformation[peerDeviceId].deviceId = peerDeviceId;
            FLib_MemCpyReverseOrder(mPhoneDeviceAddress, pConnectionEvent->eventData.connectedEvent.peerAddress, \
                gcBleDeviceAddressSize_c);
            LOG_L_S_HEX(BLE_MD, "Phone Address: ", mPhoneDeviceAddress, sizeof(mPhoneDeviceAddress));
            LOG_L_S(BLE_MD, "connect: isBonded [%d]\n\n", isBonded);

			curPeerInformation = &mPeerInformation[index];
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
            /*使能连接事件通知*/
            Gap_ControllerEnhancedNotification(gNotifConnRxPdu_c, peerDeviceId);
#endif
#if (cPWR_UsePowerDownMode)
            /* Device does not need to sleep until some information is exchanged with the peer. */
            PWR_DisallowDeviceToSleep();
#endif
            ble_ccc_send_evt(CCC_EVT_STATUS_CONNECT,peerDeviceId,NULL,0);
//            yq_location_connect();
//
//            /*连接后唤醒CAN网络*/
//            Board_Set_WakeupSource(WAKEUP_SRC_BLE);
//            app_RTE_SetBleConnectNotPairStatus(1);
//            Board_LowPower_Recovery();

        }
        break;

        case gConnEvtPairingRequest_c:
        {
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
            LOG_L_S(BLE_MD, "Accept Pairing\n");
#else
            LOG_L_S(BLE_MD, "Reject Pairing\n");
#endif            
        }
        break;

        case gConnEvtParameterUpdateComplete_c:
        {
            LOG_L_S(BLE_MD, "Connect Param Update: %d %d %d %d\n",
                pConnectionEvent->eventData.connectionUpdateComplete.status,
                pConnectionEvent->eventData.connectionUpdateComplete.connInterval,
                pConnectionEvent->eventData.connectionUpdateComplete.connLatency,
                pConnectionEvent->eventData.connectionUpdateComplete.supervisionTimeout);
        }
        break;
        case gConnEvtLeScOobDataRequest_c:
        {
            //Gap_LeScSetPeerOobData(peerDeviceId,&gCccLeScOobData);
            LOG_L_S(BLE_MD, "gConnEvtLeScOobDataRequest_c\n");
        }
        break;
        case gConnEvtOobRequest_c:
        {
            LOG_L_S(BLE_MD, "gConnEvtOobRequest_c\n");
        }
        break;
        case gConnEvtLeDataLengthChanged_c:
        {
            LOG_L_S(BLE_MD, "Le Data Length Changed\n");
        }
        break;

        case gConnEvtPasskeyDisplay_c:
        {
            LOG_L_S(BLE_MD, "PasskeyDisplay: %d\n", pConnectionEvent->eventData.passkeyForDisplay);
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            LOG_L_S(BLE_MD, "Authentication Rejected: [0x%x]\n",  pConnectionEvent->eventData.failReason);
            Gap_Disconnect (peerDeviceId);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            uintn8_t bondedCount = 0;
#if (gHidService_c)||(gHidService_c)
			(void)Hid_Unsubscribe();
#endif
#ifdef FIT_FOR_TEST
             NVIC_SystemReset();
#endif // DEBUG
            if ((pConnectionEvent->eventData.disconnectedEvent.reason == 0x122)||(pConnectionEvent->eventData.disconnectedEvent.reason == 0x13E))
            {
                NVIC_SystemReset();
            }

            LOG_L_S(BLE_MD, "\n", peerDeviceId);
            BleAppNotifyPairStatus(mPeerInformation[peerDeviceId].deviceId, 0x00);
            /* Unsubscribe client */
            mPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
            if (gBleSuccess_c == Gap_GetBondedDevicesCount(&bondedCount)) 
            {
                LOG_L_S(BLE_MD, "Bonded Count=%d\n", bondedCount);
                if (bondedCount >= gMaxBondedDevices_c)
                {
                    static uint8_t nvmIndex = 0;
                
                    bleResult_t ret = Gap_RemoveBond(nvmIndex);
                    LOG_L_S(BLE_MD, "Remove bond=%d %x\n", nvmIndex, ret);
                    nvmIndex = (nvmIndex + 1) % gMaxBondedDevices_c;
                }
            }
            BleApp_DisconnectCallBack(&index);
            PWR_AllowDeviceToSleep();

            ble_ccc_send_evt(CCC_EVT_STATUS_DISCONNECT,peerDeviceId,NULL,0);

            L2ca_DisconnectLeCbChannel(ble_ccc_l2cap_get_connectDeviceId(),ble_ccc_l2cap_get_connectChannelId());
            // ble_ccc_disconnect_dk_notify();
            /* Restart advertising*/
            BleApp_Start();
            LOG_L_S(BLE_MD, "BLE [%d] Disconnected,reason:[0x%x]\n\n", peerDeviceId,pConnectionEvent->eventData.disconnectedEvent.reason);
        }
        break;

#if gAppUsePairing_d
        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful) 
            {
                BleApp_PairingSuccessCallback(&index);
                LOG_L_S(BLE_MD, "Device bonding [0x%x]\n", pConnectionEvent->eventData.pairingCompleteEvent.pairingCompleteData.withBonding);
            } 
            else 
            {
                // for ios
                LOG_L_S(BLE_MD, "Pair Fail [0x%x]\n", pConnectionEvent->eventData.pairingCompleteEvent.pairingCompleteData.failReason);
                Gap_Disconnect (peerDeviceId);
            }
        }
        break;

        case gConnEvtEncryptionChanged_c:
        {
            if (pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState) 
            {
                LOG_L_S(BLE_MD, "Encryption Success\n");
                BleApp_PairingSuccessCallback(&index);
            } 
            else 
            {
                LOG_L_S(BLE_MD, "Encryption Fail\n");
                Gap_Disconnect(peerDeviceId);
            }

        }
        break;
#endif /* gAppUsePairing_d */

        case gConnEvtRssiRead_c:
        {
            // RSSI for an active connection has been read.
            int8_t rssi_dbm = pConnectionEvent->eventData.rssi_dBm;
            //LOG_L_S(BLE_MD, "RSSI = %d\n", rssi_dbm);
        }
        break;

    default:
        ; /* For MISRA compliance */
        break;
    }
}


/* Buffer used for Characteristic related procedures */
gattCharacteristic_t     mCharProcBuffer;

/*! *********************************************************************************
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent)
{
    bleResult_t status;
    uint8_t nvmIndex = gInvalidNvmIndex_c;
    LOG_L_S(BLE_MD, "GattServer CB: DID=[%d], EVT=[0x%x]\n", deviceId, pServerEvent->eventType);
    switch (pServerEvent->eventType)
    {
        case gEvtMtuChanged_c:
        {
            mPeerInformation[deviceId].att_mtu = pServerEvent->eventData.mtuChangedEvent.newMtu;
		    LOG_L_S(BLE_MD, "MTU %d\r\n", mPeerInformation[deviceId].att_mtu);
        }
        break;

        case gEvtHandleValueConfirmation_c:
        {
            //NONE
        }
        break;        

        case gEvtAttributeWritten_c:
        {
            bleResult_t bleResult;
            uint16_t handle = pServerEvent->eventData.attributeWrittenEvent.handle;
            uint16_t length = pServerEvent->eventData.attributeWrittenEvent.cValueLength;
            uint8_t *pValue = pServerEvent->eventData.attributeWrittenEvent.aValue;

            if (VALUE_YQDK_IS_WRITEABLE(handle)) 
            {
                bleResult = GattServer_SendAttributeWrittenStatus (deviceId,
                                                                handle,
                                                                gAttErrCodeNoError_c);
                if (gBleSuccess_c == bleResult) 
                {
                    GattDb_WriteAttribute(handle, length, pValue);
                    LOG_L_S_HEX(BLE_MD, "DK WriteAttribute", pValue, length);
                    // ble_data_process(BLE_CHANNEL_1,pValue,length);
                } 
                else 
                {
                    /*! A BLE error has occurred - Disconnect */
                	Gap_Disconnect (deviceId);
                }
            } 
            else if(handle == value_persional_1)
            {
            	bleResult = GattServer_SendAttributeWrittenStatus (deviceId,
																handle,
																gAttErrCodeNoError_c);
				if (gBleSuccess_c == bleResult)
				{
					GattDb_WriteAttribute(handle, length, pValue);
					LOG_L_S_HEX(BLE_MD, "Persional WriteAttribute", pValue, length);
#if EM_000301_CONFIG_FEATURE_APP_SNA
                    ble_data_process(BLE_CHANNEL_2,pValue,length);
#endif                    
				}
				else
				{
					/*! A BLE error has occurred - Disconnect */
					Gap_Disconnect (deviceId);
				}

            }
            else 
            {
                /*! A GATT Server is trying to GATT Write an unknown attribute value.
                *  This should not happen. Disconnect the link. */
            	Gap_Disconnect(deviceId);
                LOG_L_S(BLE_MD, "Attribute Can't Write\n");
            }
        }
        break;

        case gEvtCharacteristicCccdWritten_c:
        {
            LOG_L_S(BLE_MD, "CccdWritten: handle=[%d], Cccd=[%d]\n", 
                                    pServerEvent->eventData.charCccdWrittenEvent.handle,
                                    pServerEvent->eventData.charCccdWrittenEvent.newCccd);
        }
        break;

        case gEvtAttributeWrittenWithoutResponse_c:
        {
        	//Gap_Disconnect(deviceId);
            LOG_L_S(BLE_MD, "attributeWrittenEvent: handle=[%d]\n",pServerEvent->eventData.attributeWrittenEvent.handle);
            LOG_L_S_HEX(BLE_MD,"attributeWrittenEvent Data:",pServerEvent->eventData.attributeWrittenEvent.aValue,pServerEvent->eventData.attributeWrittenEvent.cValueLength);
        }
        break;

        case gEvtError_c:
        {
            
            attErrorCode_t attError = (attErrorCode_t) (pServerEvent->eventData.procedureError.error & 0xFF);
            if (attError == gAttErrCodeInsufficientEncryption_c     ||
                attError == gAttErrCodeInsufficientAuthorization_c  ||
                attError == gAttErrCodeInsufficientAuthentication_c)
            {
#if gAppUsePairing_d
#if gAppUseBonding_d
                bool_t isBonded = FALSE;

                /* Check if the devices are bonded and if this is true than the bond may have
                 * been lost on the peer device or the security properties may not be sufficient.
                 * In this case try to restart pairing and bonding. */
                if (gBleSuccess_c == Gap_CheckIfBonded(deviceId, &isBonded,&nvmIndex) &&
                    TRUE == isBonded)
#endif /* gAppUseBonding_d */
                {
                    (void)Gap_SendSlaveSecurityRequest(deviceId, &gPairingParameters);
                }
#endif /* gAppUsePairing_d */                
            }

            LOG_L_S(BLE_MD, "EvtError: %d\n", pServerEvent->eventData.procedureError.error);
        }
        break;

        case gEvtAttributeRead_c:
        {

            //GattServer_SendAttributeReadStatus();
            /* Received Read Request from Central on Proxy Server. Forward request to the Peripheral */
            mCharProcBuffer.value.handle = pServerEvent->eventData.attributeReadEvent.handle;

            if (mCharProcBuffer.value.handle > 0U)
            {
                mCharProcBuffer.value.paValue = MEM_BufferAlloc(2);
                status = GattClient_ReadCharacteristicValue(deviceId, &mCharProcBuffer, 2);

                if (status != gBleSuccess_c)
                {
                    (void)MEM_BufferFree(mCharProcBuffer.value.paValue);

                    /* Send Read Attribute Response to the Initiator */
                    (void)GattServer_SendAttributeReadStatus(deviceId, pServerEvent->eventData.attributeReadEvent.handle, (uint8_t)gAttErrCodeUnlikelyError_c);
                }
            }
        }
        break;

        default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Reads the battery level at mBatteryLevelReportInterval_c time interval.
*
********************************************************************************** */
//static void BatteryMeasurementTimerCallback(void * pParam)
//{
//    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
//    (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
//}


bleResult_t BleApp_NotifyDKData(deviceId_t peer_device_id, uint8_t* aValue,uint8_t aValueLength)
{
    if (mBleConnectStatus == 0)
    {
        return gBleInvalidState_c;
    }
    
    LOG_L_S_HEX(BLE_MD, "WriteAttribute Data:", aValue, aValueLength);
    return BleAppSendData(peer_device_id,service_yqdk,0xFFE2,aValueLength,aValue);
}
bleResult_t BleApp_NotifyPersionalData(deviceId_t peer_device_id, uint8_t* aValue,uint8_t aValueLength)
{
    if (mBleConnectStatus == 0)
    {
        return gBleInvalidState_c;
    }
    LOG_L_S_HEX(BLE_MD, "WriteAttribute Data:", aValue, aValueLength);
    return BleAppSendData(peer_device_id,service_persional,0xFFE5,aValueLength,aValue);
}

static void BleApp_DisconnectCallBack(void *pParam)
{
#if EM_000101_CONFIG_FEATURE_RAM_MANAGEMENT
    core_platform_ram_reset();
#endif
    /*断开连接让CAN网络进入睡眠*/
    mBleConnectStatus = 0;
}
static void BleApp_PairingSuccessCallback(void *pParam)
{
    uintn8_t index;
	//u8 status;
    if (pParam == NULL)
    {
        return ;
    }
    index = *(uintn8_t *)pParam;
    if (gInvalidDeviceId_c != mPeerInformation[index].deviceId)
    {
		/* Start rssi measurements */
		Gatt_GetMtu(mPeerInformation[index].deviceId, &mPeerInformation[index].att_mtu);
		LOG_L_S(BLE_MD, "MTU %d\r\n", mPeerInformation[index].att_mtu);
    }
    // /*连接后唤醒CAN网络*/
    mBleConnectStatus = 1;
    // Board_Set_WakeupSource(WAKEUP_SRC_BLE);
    // Board_LowPower_Recovery();

}
