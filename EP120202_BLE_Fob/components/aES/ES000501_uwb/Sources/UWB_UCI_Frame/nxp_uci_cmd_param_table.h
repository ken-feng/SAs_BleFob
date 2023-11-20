/*
 * nxp_uci_cmd_param_table.h
 *
 *  Created on: 2022年7月26日
 *      Author: JohnSong
 */

#ifndef SOURCES_UWB_UCI_FRAME_NXP_UCI_CMD_PARAM_TABLE_H_
#define SOURCES_UWB_UCI_FRAME_NXP_UCI_CMD_PARAM_TABLE_H_
/**************************************************************************************************************************
 * Device Info TLV
 **************************************************************************************************************************/
#include <stdint.h>

const uint8_t APP_CONFIG_CMD_Template[] ={
	0x21,0x03,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x09,
	//Fixed Parameter
	0x06,0x02,0x00,0x00,
	0x09,0x04,0x60,0x00,0x00,0x00,
	0x2A,0x02,0x00,0x00,
	0x23,0x01,0x03,
	0x32,0x02,0xFF,0xFF,
	0xA6,0x02,0xD0,0x02,
	0xF0,0x01,0x00,
	0xF2,0x01,0x14,
	0xFB,0x04,0x02,0x03,0x12,0x13
};

//const uint8_t DEBUG_FLASH_CONFIG_BYETS[128] = {
const uint8_t DEBUG_FLASH_CONFIG_BYETS[] = {
	0x00,//当前使用的设备配置编号
	0x00,//当前使用的应用配置编号
	0x00,//设备有效配置 bit map
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x01,//应用有效配置 bit map
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00																//unused
};

const uint8_t DEBUGE_FLASH_CONFIG_DEVICE_1[0x65] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00																								//unused
};



//const uint8_t DEBUGE_FLASH_CONFIG_APP_1[0x12E] = {
const uint8_t DEBUGE_FLASH_CONFIG_APP_1[] = {
	//0 - 99
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused

	//100 - 199
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused

	//200 - 299
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//unused

	//300 - 301
	0x00,0x00
};





//CCC
#define DIP_T_A0_LENS 2
#define DIP_T_A1_LENS 8

#define OFST_DeviceInfoParams									0
#define OFST_DIP_A0_UCI_CCC_VERSION								OFST_DeviceInfoParams
#define OFST_DIP_A1_CCC_VERSION									OFST_DIP_A0_UCI_CCC_VERSION		+ TLSIZ_11 + DIP_T_A0_LENS

const uint8_t ArrDeviceInfoParams[]=
{
	0xA0,0x02,0x00,0x00,
	0xA1,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

//Proprietary
#define PDIP_T_E3_LENS 8
#define PDIP_T_E4_LENS 3
#define PDIP_T_E5_LENS 2
#define PDIP_T_E6_LENS 8
#define PDIP_T_E7_LENS 3
#define PDIP_T_E8_LENS 2

#define	OFST_ProprietaryDeviceInfoParams					    0
#define OFST_PDIP_E3_DEVICE_NAME								OFST_ProprietaryDeviceInfoParams
#define OFST_PDIP_E4_FIRMWARE_VERSION							OFST_PDIP_E3_DEVICE_NAME 		+ TLSIZ_11 + PDIP_T_E3_LENS
#define OFST_PDIP_E5_DEVICE_VERSION								OFST_PDIP_E4_FIRMWARE_VERSION 	+ TLSIZ_11 + PDIP_T_E4_LENS
#define OFST_PDIP_E6_SERIAL_NUMBER                              OFST_PDIP_E5_DEVICE_VERSION 	+ TLSIZ_11 + PDIP_T_E5_LENS
#define OFST_PDIP_E7_DSP_VERSION                                OFST_PDIP_E6_SERIAL_NUMBER 		+ TLSIZ_11 + PDIP_T_E6_LENS
#define OFST_PDIP_E8_RANGER4_VERSION                            OFST_PDIP_E7_DSP_VERSION 		+ TLSIZ_11 + PDIP_T_E7_LENS

const uint8_t ArrProprietaryDeviceInfoParams[] = {
    0xE3,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xE4,0x03,0x00,0x00,0x00,
    0xE5,0x02,0x00,0x00,
    0xE6,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xE7,0x03,0x00,0x00,0x00,
    0xE8,0x02,0x00,0x00
};

/***************************************************************************************************************************
 * Device Config TLV
 **************************************************************************************************************************/

//CCC
#define DP_T_00_LENS	1
#define DP_T_01_LNES 	1
#define OFST_DP													0
#define OFST_DP_00_DEVICE_STATE									OFST_DP
#define OFST_DP_01_LOW_POWER_MODE								OFST_DP_00_DEVICE_STATE + TLSIZ_11 + DP_T_00_LENS

//Proprietary
#define PDCP_T_E5_LENS 2
#define PDCP_T_E6_LENS 1
#define PDCP_T_EA_LENS 2
#define PDCP_T_F4_LENS 1
#define PDCP_T_F5_LENS 1
#define PDCP_T_F6_LENS 4
#define PDCP_T_F7_LENS 2
#define PDCP_T_F8_LENS 2

#define OFST_PDCP												0
#define OFST_PDCP_E5_RESET_TIMEOUT								OFST_PDCP
#define OFST_PDCP_E6_WAKEUP_PIN									OFST_PDCP_E5_RESET_TIMEOUT       + TLSIZ_11 + PDCP_T_E5_LENS
#define OFST_PDCP_EA_HPD_ENTRY_TIMEOUT							OFST_PDCP_E6_WAKEUP_PIN          + TLSIZ_11 + PDCP_T_E6_LENS
#define OFST_PDCP_F4_RX_PHY_LOGGING_ENBL                        OFST_PDCP_EA_HPD_ENTRY_TIMEOUT   + TLSIZ_11 + PDCP_T_EA_LENS
#define OFST_PDCP_F5_TX_PHY_LOGGING_ENBL                        OFST_PDCP_F4_RX_PHY_LOGGING_ENBL + TLSIZ_11 + PDCP_T_F4_LENS
#define OFST_PDCP_F6_LOG_PARAMS_CONF                            OFST_PDCP_F5_TX_PHY_LOGGING_ENBL + TLSIZ_11 + PDCP_T_F5_LENS
#define OFST_PDCP_F7_CIR_TAP_OFFSET                             OFST_PDCP_F6_LOG_PARAMS_CONF     + TLSIZ_11 + PDCP_T_F6_LENS
#define OFST_PDCP_F8_CIR_NUM_TAPS                               OFST_PDCP_F7_CIR_TAP_OFFSET      + TLSIZ_11 + PDCP_T_F7_LENS
const uint8_t ArrProprietaryDeviceConfigurationParams[]={
	0xE5,0x02,0x00,0x00,
    0xE6,0x01,0x00,
    0xEA,0x02,0x00,0x00,
    0xF4,0x01,0x00,
    0xF5,0x01,0x00,
    0xF6,0x04,0x00,0x00,0x00,0x00,
    0xF7,0x02,0x00,0x00,
    0xF8,0x02,0x00,0x00
};

/***************************************************************************************************************************
 * Device Capability Parameters TLV
 **************************************************************************************************************************/

//CCC
/*
#define DCP_T_A0_LENS 1
#define DCP_T_A1_LENS 4
#define DCP_T_A2_LENS 1
#define DCP_T_A3_LENS 1
#define DCP_T_A4_LENS 4
#define DCP_T_A5_LENS 4
#define DCP_T_A6_LENS 9

#define	OFST_DevCapParam										0
#define OFST_DCP_A0_SLOT_BITMASK								OFST_DevCapParam
#define OFST_DCP_A1_SYNC_CODE_INDEX_BITMASK						OFST_DCP_A0_SLOT_BITMASK 				+ TLSIZ_11 + DCP_T_A0_LENS
#define OFST_DCP_A2_HOPPING_CONFIG_BITMASK						OFST_DCP_A1_SYNC_CODE_INDEX_BITMASK 	+ TLSIZ_11 + DCP_T_A1_LENS
#define OFST_DCP_A3_CHANNEL_BITMASK								OFST_DCP_A2_HOPPING_CONFIG_BITMASK 		+ TLSIZ_11 + DCP_T_A2_LENS
#define OFST_DCP_A4_SUPPORTED_PROTOCOL_VERSION					OFST_DCP_A3_CHANNEL_BITMASK 			+ TLSIZ_11 + DCP_T_A3_LENS
#define OFST_DCP_A5_SUPPORTED_UWB_CONFIG_ID						OFST_DCP_A4_SUPPORTED_PROTOCOL_VERSION 	+ TLSIZ_11 + DCP_T_A4_LENS
#define OFST_DCP_A6_SUPPORTED_PULSESHAPE_COMBO					OFST_DCP_A5_SUPPORTED_UWB_CONFIG_ID 	+ TLSIZ_11 + DCP_T_A5_LENS
const uint8_t ArrDeviceCapabilityParames[]={
	0xA0,0x01,0x00,
	0xA1,0x04,0x00,0x00,0x00,0x00,
	0xA2,0x01,0x00,
	0xA3,0x01,0x00,
	0xA4,0x04,0x00,0x00,0x00,0x00,
	0xA5,0x04,0x00,0x00,0x00,0x00,
	0xA6,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
*/
/***************************************************************************************************************************
 * Session APP Config Parameters TLV
 **************************************************************************************************************************/

//CCC
#define ACP_T_00_LENS 1
#define ACP_T_02_LENS 1
#define ACP_T_04_LENS 1
#define ACP_T_05_LENS 1
#define ACP_T_06_LENS 2
#define ACP_T_07_LENS 2
#define ACP_T_08_LENS 2
#define ACP_T_09_LENS 4
#define ACP_T_0A_LENS 4
#define ACP_T_0B_LENS 1
#define ACP_T_0E_LENS 1
#define ACP_T_0F_LENS 2
#define ACP_T_10_LENS 2
#define ACP_T_11_LENS 1
#define ACP_T_14_LENS 1
#define ACP_T_15_LENS 1
#define ACP_T_1B_LENS 1
#define ACP_T_1C_LENS 1
#define ACP_T_1E_LENS 1
#define ACP_T_23_LENS 1
#define ACP_T_25_LENS 1
#define ACP_T_2A_LENS 2
#define ACP_T_2B_LENS 4
#define ACP_T_2C_LENS 1
#define ACP_T_32_LENS 2
#define ACP_T_A0_LENS 16
#define ACP_T_A1_LENS 1
#define ACP_T_A2_LENS 1
#define ACP_T_A3_LENS 2
#define ACP_T_A4_LENS 2
#define ACP_T_A5_LENS 1
#define ACP_T_A6_LENS 2

#define OFST_AppConfigParams                                        0
#define OFST_ACP_00_DEVICE_TYPE                                     OFST_AppConfigParams
#define OFST_ACP_02_STS_CONFIG                                      OFST_ACP_00_DEVICE_TYPE                 + TLSIZ_11 + ACP_T_00_LENS
#define OFST_ACP_04_CHANNEL_ID                                      OFST_ACP_02_STS_CONFIG                  + TLSIZ_11 + ACP_T_02_LENS
#define OFST_ACP_05_NUMBER_OF_ANCHORS                               OFST_ACP_04_CHANNEL_ID                  + TLSIZ_11 + ACP_T_04_LENS
#define OFST_ACP_06_DEVICE_MAC_ADDRESS                              OFST_ACP_05_NUMBER_OF_ANCHORS           + TLSIZ_11 + ACP_T_05_LENS
#define OFST_ACP_07_DST_MAC_ADDRESS                                 OFST_ACP_06_DEVICE_MAC_ADDRESS          + TLSIZ_11 + ACP_T_06_LENS
#define OFST_ACP_08_RANGING_SLOT_LENGTH                             OFST_ACP_07_DST_MAC_ADDRESS             + TLSIZ_11 + ACP_T_07_LENS
#define OFST_ACP_09_RANGING_INTERVAL                                OFST_ACP_08_RANGING_SLOT_LENGTH         + TLSIZ_11 + ACP_T_08_LENS
#define OFST_ACP_0A_STS_INDEX0                                      OFST_ACP_09_RANGING_INTERVAL            + TLSIZ_11 + ACP_T_09_LENS
#define OFST_ACP_0B_MAC_FCS_TYPE                                    OFST_ACP_0A_STS_INDEX0                  + TLSIZ_11 + ACP_T_0A_LENS
#define OFST_ACP_0E_RNG_DATA_NTF                                    OFST_ACP_0B_MAC_FCS_TYPE                + TLSIZ_11 + ACP_T_0B_LENS
#define OFST_ACP_0F_RNG_DATA_NTF_PROXIMITY_NEAR                     OFST_ACP_0E_RNG_DATA_NTF                + TLSIZ_11 + ACP_T_0E_LENS
#define OFST_ACP_10_RNG_DATA_NTF_PROXIMITY_FAR                      OFST_ACP_0F_RNG_DATA_NTF_PROXIMITY_NEAR + TLSIZ_11 + ACP_T_0F_LENS
#define OFST_ACP_11_DEVICE_ROLE                                     OFST_ACP_10_RNG_DATA_NTF_PROXIMITY_FAR  + TLSIZ_11 + ACP_T_10_LENS
#define OFST_ACP_14_PREAMBLE_ID                                     OFST_ACP_11_DEVICE_ROLE                 + TLSIZ_11 + ACP_T_11_LENS
#define OFST_ACP_15_SFD_ID                                          OFST_ACP_14_PREAMBLE_ID                 + TLSIZ_11 + ACP_T_14_LENS
#define OFST_ACP_1B_SLOTS_PER_R                                     OFST_ACP_15_SFD_ID                      + TLSIZ_11 + ACP_T_15_LENS
#define OFST_ACP_1C_ADAPTIVE_PAYLOAD_POWER                          OFST_ACP_1B_SLOTS_PER_R                 + TLSIZ_11 + ACP_T_1B_LENS
#define OFST_ACP_1E_RESPONDER_SLOT_INDEX                            OFST_ACP_1C_ADAPTIVE_PAYLOAD_POWER      + TLSIZ_11 + ACP_T_1C_LENS
#define OFST_ACP_23_KEY_ROTATION                                    OFST_ACP_1E_RESPONDER_SLOT_INDEX        + TLSIZ_11 + ACP_T_1E_LENS
#define OFST_ACP_25_SESSION_PRIORITY                                OFST_ACP_23_KEY_ROTATION                + TLSIZ_11 + ACP_T_23_LENS
#define OFST_ACP_2A_MAX_RR_RETRY                                    OFST_ACP_25_SESSION_PRIORITY            + TLSIZ_11 + ACP_T_25_LENS
#define OFST_ACP_2B_UWB_INITIATION_TIME                             OFST_ACP_2A_MAX_RR_RETRY                + TLSIZ_11 + ACP_T_2A_LENS
#define OFST_ACP_2C_HOPPING_MODE                                    OFST_ACP_2B_UWB_INITIATION_TIME         + TLSIZ_11 + ACP_T_2B_LENS
#define OFST_ACP_32_MAX_NUMBER_OF_MEASUREMENTS                      OFST_ACP_2C_HOPPING_MODE                + TLSIZ_11 + ACP_T_2C_LENS
#define OFST_ACP_A0_HOP_MODE_KEY                                    OFST_ACP_32_MAX_NUMBER_OF_MEASUREMENTS  + TLSIZ_11 + ACP_T_32_LENS
#define OFST_ACP_A1_CCC_CONFIG_QUIRKS                               OFST_ACP_A0_HOP_MODE_KEY                + TLSIZ_11 + ACP_T_A0_LENS
#define OFST_ACP_A2_RFU                                             OFST_ACP_A1_CCC_CONFIG_QUIRKS           + TLSIZ_11 + ACP_T_A1_LENS
#define OFST_ACP_A3_RANGING_PROTOCOL_VER                            OFST_ACP_A2_RFU                         + TLSIZ_11 + ACP_T_A2_LENS
#define OFST_ACP_A4_UWB_CONFIG_ID                                   OFST_ACP_A3_RANGING_PROTOCOL_VER        + TLSIZ_11 + ACP_T_A3_LENS
#define OFST_ACP_A5_PULSESHAPE_COMBO                                OFST_ACP_A4_UWB_CONFIG_ID               + TLSIZ_11 + ACP_T_A4_LENS
#define OFST_ACP_A6_URSK_TTL                                        OFST_ACP_A5_PULSESHAPE_COMBO            + TLSIZ_11 + ACP_T_A5_LENS
//static uint8_t ArrAppConfigParams[] ={
//    0x00,0x01,0x00,
//    0x02,0x01,0x00,
//    0x04,0x01,0x00,
//    0x05,0x01,0x00,
//    0x06,0x02,0x00,0x00,
//    0x07,0x02,0x00,0x00,
//    0x08,0x02,0x00,0x00,
//    0x09,0x04,0x00,0x00,0x00,0x00,
//    0x0A,0x04,0x00,0x00,0x00,0x00,
//    0x0B,0x01,0x00,
//    0x0E,0x01,0x00,
//    0x0F,0x02,0x00,0x00,
//    0x10,0x02,0x00,0x00,
//    0x11,0x01,0x00,
//    0x14,0x01,0x00,
//    0x15,0x01,0x00,
//    0x1B,0x01,0x00,
//    0x1C,0x01,0x00,
//    0x1E,0x01,0x00,
//    0x23,0x01,0x00,
//    0x25,0x01,0x00,
//    0x2A,0x02,0x00,0x00,
//    0x2B,0x04,0x00,0x00,0x00,0x00,
//    0x2C,0x01,0x00,
//    0x32,0x02,0x00,0x00,
//    0xA0,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//    0xA1,0x01,0x00,
//    0xA2,0x01,0x00,
//    0xA3,0x02,0x00,0x00,
//    0xA4,0x02,0x00,0x00,
//    0xA5,0x01,0x00,
//    0xA6,0x02,0x00,0x00
//};


//Proprietary
#define PACP_T_E3_LENS 1
#define PACP_T_E4_LENS 2
#define PACP_T_E5_LENS 2
#define PACP_T_E6_LENS 1
#define PACP_T_E7_LENS 1
#define PACP_T_E8_LENS 33
#define PACP_T_E9_LENS 34
#define PACP_T_EA_LENS 6
#define PACP_T_EB_LENS 6
#define PACP_T_EC_LENS 1
#define PACP_T_ED_LENS 1
#define PACP_T_EF_LENS 8
#define PACP_T_F0_LENS 1
#define PACP_T_F1_LENS 2
#define PACP_T_F2_LENS 1
#define PACP_T_F4_LENS 1
#define PACP_T_F5_LENS 1
#define PACP_T_F6_LENS 4
#define PACP_T_F7_LENS 2
#define PACP_T_F8_LENS 2
#define PACP_T_F9_LENS 1
#define PACP_T_FA_LENS 3
#define PACP_T_FB_LENS 4
#define PACP_T_FD_LENS 2
#define PACP_T_FE_LENS 1

#define OFST_ProprietaryAppConfigParams                         0
#define OFST_PACP_E3_RX_START_MARGIN                            OFST_ProprietaryAppConfigParams
#define OFST_PACP_E4_RX_TIMEOUT                                 OFST_PACP_E3_RX_START_MARGIN            + TLSIZ_11 + PACP_T_E3_LENS
#define OFST_PACP_E5_ADAPTED_RANGING_INDEX                      OFST_PACP_E4_RX_TIMEOUT                 + TLSIZ_11 + PACP_T_E4_LENS
#define OFST_PACP_E6_NBIC_CONF                                  OFST_PACP_E5_ADAPTED_RANGING_INDEX      + TLSIZ_11 + PACP_T_E5_LENS
#define OFST_PACP_E7_GROUPDELAY_RECALC_ENA                      OFST_PACP_E6_NBIC_CONF                  + TLSIZ_11 + PACP_T_E6_LENS
#define OFST_PACP_E8_URSK                                       OFST_PACP_E7_GROUPDELAY_RECALC_ENA      + TLSIZ_11 + PACP_T_E7_LENS
#define OFST_PACP_E9_STATIC_KEYS                                OFST_PACP_E8_URSK                       + TLSIZ_11 + PACP_T_E8_LENS
#define OFST_PACP_EA_RCM_RX_MARGIN_TIME                         OFST_PACP_E9_STATIC_KEYS                + TLSIZ_11 + PACP_T_E9_LENS
#define OFST_PACP_EB_RCM_RX_TIMEOUT                             OFST_PACP_EA_RCM_RX_MARGIN_TIME         + TLSIZ_11 + PACP_T_EA_LENS
#define OFST_PACP_EC_DYNAMIC_PRIORITY_IN_SYNCH                  OFST_PACP_EB_RCM_RX_TIMEOUT             + TLSIZ_11 + PACP_T_EB_LENS
#define OFST_PACP_ED_TX_POWER_TEMP_COMPENSATION                 OFST_PACP_EC_DYNAMIC_PRIORITY_IN_SYNCH  + TLSIZ_11 + PACP_T_EC_LENS
#define OFST_PACP_EF_LONG_SRC_ADDRESS                           OFST_PACP_ED_TX_POWER_TEMP_COMPENSATION + TLSIZ_11 + PACP_T_ED_LENS
#define OFST_PACP_F0_N                                          OFST_PACP_EF_LONG_SRC_ADDRESS           + TLSIZ_11 + PACP_T_EF_LENS
#define OFST_PACP_F1_RR_RETRY_THR                               OFST_PACP_F0_N                          + TLSIZ_11 + PACP_T_F0_LENS
#define OFST_PACP_F2_TX_POWER_ID                                OFST_PACP_F1_RR_RETRY_THR               + TLSIZ_11 + PACP_T_F1_LENS
#define OFST_PACP_F4_RX_PHY_LOGGING_ENBL                        OFST_PACP_F2_TX_POWER_ID                + TLSIZ_11 + PACP_T_F2_LENS
#define OFST_PACP_F5_TX_PHY_LOGGING_ENBL                        OFST_PACP_F4_RX_PHY_LOGGING_ENBL        + TLSIZ_11 + PACP_T_F4_LENS
#define OFST_PACP_F6_LOG_PARAMS_CONF                            OFST_PACP_F5_TX_PHY_LOGGING_ENBL        + TLSIZ_11 + PACP_T_F5_LENS
#define OFST_PACP_F7_CIR_TAP_OFFSET                             OFST_PACP_F6_LOG_PARAMS_CONF            + TLSIZ_11 + PACP_T_F6_LENS
#define OFST_PACP_F8_CIR_NUM_TAPS                               OFST_PACP_F7_CIR_TAP_OFFSET             + TLSIZ_11 + PACP_T_F7_LENS
#define OFST_PACP_F9_STS_INDEX_RESTART                          OFST_PACP_F8_CIR_NUM_TAPS               + TLSIZ_11 + PACP_T_F8_LENS
#define OFST_PACP_FA_VENDOR_SPECIFIC_OUI                        OFST_PACP_F9_STS_INDEX_RESTART          + TLSIZ_11 + PACP_T_F9_LENS
#define OFST_PACP_FB_RADIO_CFG_IDXS                             OFST_PACP_FA_VENDOR_SPECIFIC_OUI        + TLSIZ_11 + PACP_T_FA_LENS
#define OFST_PACP_FD_CRYPTO_KEY_USAGE_FLAG                      OFST_PACP_FB_RADIO_CFG_IDXS             + TLSIZ_11 + PACP_T_FB_LENS
#define OFST_PACP_FE_SEND_FINAL_ALWAYS                          OFST_PACP_FD_CRYPTO_KEY_USAGE_FLAG      + TLSIZ_11 + PACP_T_FD_LENS

const uint8_t ArrProprietaryAppConfigParams[] ={
    0xE3,0x01,0x00,
    0xE4,0x02,0x00,0x00,
    0xE5,0x02,0x00,0x00,
    0xE6,0x01,0x00,
    0xE7,0x01,0x00,
    0xE8,0x21,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xE9,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xEA,0x06,0x00,0x00,0x00,0x00,0x00,0x00,
    0xEB,0x06,0x00,0x00,0x00,0x00,0x00,0x00,
    0xEC,0x01,0x00,
    0xED,0x01,0x00,
    0xEF,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xF0,0x01,0x00,
    0xF1,0x02,0x00,0x00,
    0xF2,0x01,0x00,
    0xF4,0x01,0x00,
    0xF5,0x01,0x00,
    0xF6,0x04,0x00,0x00,0x00,0x00,
    0xF7,0x02,0x00,0x00,
    0xF8,0x02,0x00,0x00,
    0xF9,0x01,0x00,
    0xFA,0x03,0x00,0x00,0x00,
    0xFB,0x04,0x00,0x00,0x00,0x00,
    0xFD,0x02,0x00,0x00,
    0xFE,0x01,0x00
};
#endif /* SOURCES_UWB_UCI_FRAME_NXP_UCI_CMD_PARAM_TABLE_H_ */

#if 0 
abandoned code


#endif
