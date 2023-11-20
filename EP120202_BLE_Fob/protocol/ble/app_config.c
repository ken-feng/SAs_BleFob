/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file contains configuration data for the application and stack
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "gap_interface.h"
#include "ble_constants.h"
#if !defined(MULTICORE_APPLICATION_CORE) || (!MULTICORE_APPLICATION_CORE)
#include "gatt_db_handles.h"
#else
#include "dynamic_gatt_database.h"
#endif
#include "otap_client_att.h"
#include "EM000101.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define smpEdiv                 0x1F99
#define mcEncryptionKeySize_c   16

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/* Default Advertising Parameters. Values can be changed at runtime 
    to align with profile requirements */
#define gGapAdvertisingInterval_042ms_c 0x0044 //42.5ms    
#define gGapAdvertisingInterval_050ms_c 0x0050
#define gGapAdvertisingInterval_100ms_c 0x00A0
#define gGapAdvertisingInterval_125ms_c 0x00C8
#define gGapAdvertisingInterval_250ms_c 0x0190
#define gGapAdvertisingInterval_500ms_c 0x0320
#define gGapAdvertisingInterval_1000ms_c 0x0640
#define gGapAdvertisingInterval_5000ms_c 0x1F40
gapAdvertisingParameters_t gAdvParams =
{ 
    /* minInterval */         gGapAdvertisingInterval_042ms_c,//gGapAdvertisingInterval_050ms_c,
    /* maxInterval */         gGapAdvertisingInterval_042ms_c,//gGapAdvertisingInterval_100ms_c,
    /* advertisingType */     gAdvConnectableUndirected_c, 
    /* addressType */         gBleAddrTypePublic_c, 
    /* directedAddressType */ gBleAddrTypePublic_c, 
    /* directedAddress */     {0, 0, 0, 0, 0, 0}, 
    /* channelMap */          (gapAdvertisingChannelMapFlags_t) (gGapAdvertisingChannelMapDefault_c), 
    /* filterPolicy */        gProcessAll_c 
};





/* Scanning and Advertising Data */
static const uint8_t adData0[1] =  { (gapAdTypeFlags_t)(gLeGeneralDiscoverableMode_c | gBrEdrNotSupported_c) };

#if 0//APP_DEBUG_MODE
static uint8_t adData2[20] =
{
    'E', 'P', '0', '1', '0', '2', '0', '1',
	'0', '0', '0', '0', '0', '0', '0','0', '0', '0', '0', '0',
};
#else
static uint8_t adData2[18] =
{
    'F', 'I', 'T', 'F', 'O', 'B',
	'0', '0', '0', '0', '0', '0', '0','0', '0', '0', '0', '0',
};
#endif

static uint8_t cccDKUuidBuf[19] =
{
    0x58, 0x10, 0xBB, 0xC0, 0xB4, 0x99, 0x11, 0xE9, 0xA2, 0xA3, 0x2A, 0x2A, 0xE2, 0xDB, 0xCC, 0xE4,//uuid_service_ccc_dk
    0x01,//IntentConfiguration
    0x00,0x00,//Vehicle manufacturer
};

static uintn8_t adData3[5] = 
{
	0, 0, 0, 0,			/* random */
	0					/* device status */
};
static const uint8_t adData5[2] = { UuidArray(gBleSig_HidService_d) };
static const uint8_t adData1[2] = { UuidArray(0xFFE0) };
static const uint8_t cccDKUuid[2] = { UuidArray(0xFFF5) };
static const gapAdStructure_t advScanStruct[] = 
{
    {
        .length = NumberOfElements(adData0) + 1,
        .adType = gAdFlags_c,
        .aData = (uint8_t *)adData0
    },
    {
        .length = NumberOfElements(cccDKUuid) + 1,
        .adType = gAdIncomplete16bitServiceList_c,
        .aData = (uint8_t *)cccDKUuid//adData1
    },
    {
        .length = NumberOfElements(cccDKUuidBuf) + 1,
        .adType = gAdServiceData128bit_c,
        .aData = (uint8_t *)cccDKUuidBuf//adData1
    }
#if (gHidService_c)||(gHidService_c)
    ,
	{
		.length = NumberOfElements(adData5) + 1,  // 5
		.adType = gAdIncomplete16bitServiceList_c,
		.aData = (uint8_t *)adData5
	}
#endif
};


static const gapAdStructure_t rspScanStruct[] = 
{

    {
        .length = NumberOfElements(adData2) + 1, // 17
        .adType = gAdCompleteLocalName_c,
        .aData = adData2
    },
    {
		.length = NumberOfElements(adData3) + 1, // 7
        .adType = gAdManufacturerSpecificData_c,
        .aData = adData3
	}
};


gapAdvertisingData_t gAppAdvertisingData = 
{
    NumberOfElements(advScanStruct),
    (void *)advScanStruct
};


gapScanResponseData_t gAppScanRspData = 
{
    NumberOfElements(rspScanStruct),
    (void *)rspScanStruct
};


/* Advertising Data change */
void advDataUpdate(u8 *device_name, u8 size)
{
	if (size == 12)
	{
#if 0//APP_DEBUG_MODE        
		core_mm_copy(&adData2[8], device_name, size);
#else        
        core_mm_copy(&adData2[6], device_name, size);
#endif	
    }
}
/* SMP Data */
gapPairingParameters_t gPairingParameters = {
    .withBonding = (bool_t)gAppUseBonding_d,
    .securityModeAndLevel = gSecurityMode_1_Level_2_c,
    .maxEncryptionKeySize = mcEncryptionKeySize_c,
    .localIoCapabilities = gIoNone_c,
    .oobAvailable = FALSE,
    .centralKeys = gIrk_c, 
    .peripheralKeys = (gapSmpKeyFlags_t) (gLtk_c | gIrk_c),
    .leSecureConnectionSupported = TRUE,
    .useKeypressNotifications = FALSE,
};

/* LTK */
static uint8_t smpLtk[gcSmpMaxLtkSize_c] =
    {0xD6, 0x93, 0xE8, 0xA4, 0x23, 0x55, 0x48, 0x99,
     0x1D, 0x77, 0x61, 0xE6, 0x63, 0x2B, 0x10, 0x8E};

/* RAND*/
static uint8_t smpRand[gcSmpMaxRandSize_c] =
    {0x26, 0x1E, 0xF6, 0x09, 0x97, 0x2E, 0xAD, 0x7E};

/* IRK */
static uint8_t smpIrk[gcSmpIrkSize_c] =
    {0x0A, 0x2D, 0xF4, 0x65, 0xE3, 0xBD, 0x7B, 0x49,
     0x1E, 0xB4, 0xC0, 0x95, 0x95, 0x13, 0x46, 0x73};

/* CSRK */
static uint8_t smpCsrk[gcSmpCsrkSize_c] =
    {0x90, 0xD5, 0x06, 0x95, 0x92, 0xED, 0x91, 0xD7,
     0xA8, 0x9E, 0x2C, 0xDC, 0x4A, 0x93, 0x5B, 0xF9};

gapSmpKeys_t gSmpKeys = {
    .cLtkSize = mcEncryptionKeySize_c,
    .aLtk = (void *)smpLtk,
    .aIrk = (void *)smpIrk,
    .aCsrk = (void *)smpCsrk,
    .aRand = (void *)smpRand,
    .cRandSize = gcSmpMaxRandSize_c,
    .ediv = smpEdiv,
    .addressType = gBleAddrTypePublic_c,
    .aAddress = NULL
};

/* Device Security Requirements */
static const gapSecurityRequirements_t        masterSecurity = gGapDefaultSecurityRequirements_d;
#if (gHidService_c)||(gHidService_c)
static const gapServiceSecurityRequirements_t serviceSecurity[4] = {
  {
    .requirements = {
        .securityModeLevel = gSecurityMode_1_Level_2_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = (uint16_t)service_hid
  },
#else
static const gapServiceSecurityRequirements_t serviceSecurity[3] = {
#endif
  {
    .requirements = {
        .securityModeLevel = gSecurityMode_1_Level_2_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = (uint16_t)service_yqdk
  },
  {
    .requirements = {
        .securityModeLevel = gSecurityMode_1_Level_2_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = (uint16_t)service_persional
  },
  {
    .requirements = {
        .securityModeLevel = gSecurityMode_1_Level_1_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = (uint16_t)service_ccc_dk
  },
};

gapDeviceSecurityRequirements_t deviceSecurityRequirements = {
    .pMasterSecurityRequirements    = (void*)&masterSecurity,
#if (gHidService_c)||(gHidService_c)
	.cNumServices                   = 4,
#else
    .cNumServices                   = 3,
#endif
    .aServiceSecurityRequirements   = (void*)serviceSecurity
};
