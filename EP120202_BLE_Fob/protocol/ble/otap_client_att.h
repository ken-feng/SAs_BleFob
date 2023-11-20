/*! *********************************************************************************
 * \defgroup BLE OTAP Client ATT
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the interface file for the BLE OTAP Client ATT application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef OTAP_CLIENT_ATT_H
#define OTAP_CLIENT_ATT_H
#include "EM000101.h"
#include "EmbeddedTypes.h"
#include "ble_general.h"
#define OTAP_CENTER         0
#define OTAP_PERIPHERALS    !OTAP_CENTER
#define OTAP_ROLE           OTAP_PERIPHERALS

/* Enable/Disable Controller Adv/Scan/Connection Notifications */
#ifndef gUseControllerNotifications_c
#define gUseControllerNotifications_c 1
#endif



/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/* Profile Parameters */

#define gFastConnMinAdvInterval_c       32 /* 20 ms */
#define gFastConnMaxAdvInterval_c       48 /* 30 ms */

// #define gFastConnMinAdvInterval_c       400 /* 500 ms */
// #define gFastConnMaxAdvInterval_c       400 /* 500 ms */

// #define gReducedPowerMinAdvInterval_c   1600 /* 1 s */
// #define gReducedPowerMaxAdvInterval_c   4000 /* 2.5 s */

// #define gFastConnMinAdvInterval_c       200 /* 125 ms */
// #define gFastConnMaxAdvInterval_c       200 /* 125 ms */

// #define gFastConnMinAdvInterval_c       400 /* 250 ms */
// #define gFastConnMaxAdvInterval_c       400 /* 250 ms */
// #define gFastConnMinAdvInterval_c       800 /* 500 ms */
// #define gFastConnMaxAdvInterval_c       800 /* 500 ms */

// #define gFastConnMinAdvInterval_c       3200 /* 2 s */
// #define gFastConnMaxAdvInterval_c       8000 /* 2 s */
#define gReducedPowerMinAdvInterval_c       3200 /* 2 s */
#define gReducedPowerMaxAdvInterval_c       3200 /* 2 s */

// #define gReducedPowerMinAdvInterval_c   8000 /* 5 s */
// #define gReducedPowerMaxAdvInterval_c   8000 /* 5 s */


#define gAdvTime_c                      10 /* 10 s*/
#define gFastConnAdvTime_c              30  /* s */
#define gReducedPowerAdvTime_c          300 /* s */

#if gAppUseBonding_d
#define gFastConnWhiteListAdvTime_c     10 /* s */
#else
#define gFastConnWhiteListAdvTime_c     0
#endif

#ifndef gIBeaconAdvData_c
#define gIBeaconAdvData_c           0
#endif


#ifndef gHidService_c
#define gHidService_c           0
#endif

#ifndef gSupportLin_c
#define gSupportLin_c           0
#endif
typedef struct appPeerInfo_tag
{
    uint8_t     deviceId;
    uint8_t     ntf_cfg;
    uint16_t    att_mtu;
} appPeerInfo_t;

typedef enum
{
    fastAdvState_c = 0,
    slowAdvState_c
}advType_t;
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void);

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void);

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent);
bleResult_t BleApp_SendData(uint8_t deviceId, uint16_t serviceHandle, 
            uint16_t charUuid, uint16_t length, uint8_t *pData);


u8 BleAppGetStatusValue(void);
void BleAppSetStatusValue(u8 status);

bleResult_t BleApp_NotifyDKData(deviceId_t peer_device_id, uint8_t* aValue,uint8_t aValueLength);
bleResult_t BleApp_NotifyPersionalData(deviceId_t peer_device_id, uint8_t* aValue,uint8_t aValueLength);


/*设置蓝牙广播快慢模式*/
void BleApp_SwitchAdvMode(advType_t mode);
advType_t BleApp_GetAdvMode(void);
#ifdef __cplusplus
}
#endif


#endif /* OTAP_CLIENT_ATT_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
