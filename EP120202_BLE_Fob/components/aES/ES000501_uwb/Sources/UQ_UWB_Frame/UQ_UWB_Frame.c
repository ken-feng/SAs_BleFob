/*
 * UQ_UWB_Frame.c
 *
 *  Created on: 2022年8月15日
 *      Author: JohnSong
 */

#include "../uwb_common_def.h"
#if defined(UWB_RESPONDER)
#include <components/aES/ES000501_uwb/Sources/UWBResponder/uwb_responder_api.h>
#elif defined(UWB_INITIATOR)
#include "./../UWBInitiator/uwb_initiator_api.h"
#else
" you need make a choose ."
#endif
#include "./UQ_UWB_Frame.h"

//SDK function pointer
uint8_t curr_config_bytes[0x80]		= {0};
uint8_t curr_config_device[0x65]	= {0};
uint8_t curr_config_app[0x12E]		= {0};

#define UWB_CONFIG_BYTES	curr_config_bytes
#define UWB_DEVICE_CONFIG 	curr_config_device
#define UWB_APP_CONFIG		curr_config_app

void UWB_NTF_Check(ST_UCIFrameState *pstate)
{
#if 0
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	//NTF Processing
	switch(pstate->NTFIdx)
	{
		case UWB_Sys_Notice_NTF:
										//DEBUGE_PRINT_RESCODE(pstate->OpCode);
										break;
		case UBW_Deviec_Status_NTF:
			switch(pstate->OpCode)
			{
		    	case UWB_Err_UCI_Device_Status_NTF_RangingSession_Is_Ready	:LOG_L_S(CCC_MD,"UWB Rest Success .");break;
		    	case UWB_Err_UCI_Device_Status_NTF_RangingSession_Is_Bussy	:DEBUGE_PRINT_RESCODE(UWB_Err_UCI_Device_Status_NTF_RangingSession_Is_Bussy);break;break;
		    	case UWB_Err_UCI_Device_Status_NTF_Device_IsNot_29D5		:DEBUGE_PRINT_RESCODE(UWB_Err_UCI_Device_Status_NTF_Status_RFU_Error);break;//report result and goto sleep
		    	case UWB_Err_UCI_Device_Status_NTF_Device_Is_Test_Mode		:break;
		    	case UWB_Err_UCI_Device_Status_NTF_Reboot_By_SoftWare		:break;
		    	case UWB_Err_UCI_Device_Status_NTF_Reboot_From_HPD			:break;
		    	case UWB_Err_UCI_Device_Status_NTF_Reboot_From_RST_Pin		:break;
		    	case UWB_Err_UCI_Device_Status_NTF_Reboot_Power_Fault		:break;
		    	case UWB_Err_UCI_Device_Status_NTF_Status_Error				:DEBUGE_PRINT_RESCODE(UWB_Err_UCI_Device_Status_NTF_Status_RFU_Error);break;//report result and goto sleep
		    	default:
		    		DEBUGE_PRINT_RESCODE(UWB_Err_UCI_Device_Status_NTF_Status_RFU_Error);
		    		break;//report result and goto sleep
			}
			break;
		case UWB_Session_Status_NTF:
										DEBUGE_PRINT_RESCODE(pstate->OpCode);
										break;
			default:
				break;
	}
	pstate->NTFIdx = UWB_MSG_Is_Null;
#endif
	pstate->eNTFIdx = UWB_MSG_Is_Null;
}
#if defined(UWB_RESPONDER)
ST_UQCMDSeqAnchorWakeupRQ	stUQCMDSeqAnchorWakeup;
ST_UQCMDSeqRangingStop		stUQCMDSeqRangingSTOP;
ST_UQCMDSeqRangingRecover	stUQCMDSeqRangingRecover;
ST_UQCMDSeqRangingStart		stUQCMDSeqRangingStart;

const uint8_t CORE_CONFIG_CMD_Template[] = {
	0x20, 0x04, 0x00, 0x08,
	0x02,
	0x01, 0x01, 0x00,//停用低功耗
	0xEA, 0x02, 0x10, 0x27//从低功耗退出到运行10ms
};

const uint8_t base_device_config[] = {
	0x01,//config index
	0x01,//config type
	0x02,//config NUM
	0x07,//lens
	0x01, 0x01, 0x00,
	0xEA, 0x02, 0x10, 0x27
};
//const uint8_t base_app_config[] = {
//	0x01,//config index
//	0x02,//config type
//	0x19,//config NUM
//	0x8C, 0x00,//lens
//	0x04, 0x01, 0x09,
//	0x05, 0x01, 0x08,
//	0x06, 0x02, 0x00, 0x00,
//	0x09, 0x04, 0x60, 0x00, 0x00, 0x00,
//	0x08, 0x02, 0xB0, 0x04,
//	0x0A, 0x04, 0x00, 0x00, 0x00, 0x00,
//	0x11, 0x01, 0x01,
//	0x14, 0x01, 0x0A,
//	0x15, 0x01, 0x00,
//	0x1B, 0x01, 0x0C,
//	0x2A, 0x02, 0x00, 0x00,
//	0x2C, 0x01, 0x00,
//	0x23, 0x01, 0x03,
//	0x32, 0x02, 0xFF, 0xFF,
//	0xE9, 0x12, 0x04, 0x00, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54,
//	0xE9, 0x12, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54,
//	0xF2, 0x01, 0x14,
//	0xA0, 0x10, 0x73, 0x05, 0x8F, 0xE5, 0x50, 0x3C, 0x1D, 0x4A, 0x17, 0x54, 0x1D, 0x51, 0x21, 0xBB, 0x7F, 0x9E,
//	0xA4, 0x02, 0x00, 0x00,
//	0xA5, 0x01, 0x11,
//	0xA6, 0x02, 0xD0, 0x02,
//	0xF0, 0x01, 0x00,
//	0xFB, 0x04, 0x02, 0x03, 0x12, 0x13,
//	0xFD, 0x02, 0xFF, 0x01,
//	0xF9, 0x01,0X01
//};
E_UWBErrCode UQUWBAnchorInit(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ResCode = API_UWB_Responder_Init(pst_uwb_source);
	return ResCode;
}

E_UWBErrCode UQUWBAnchorReset(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	pst_uwb_source->stUCIState.bIsInCmd = true;
	pst_uwb_source->stUCIState.eMSGIdx = UWB_Hard_Reset_RQ;
	ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);
	return ResCode;
}

E_UWBErrCode UQUWBAnchorGetCapblity(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;

	pst_uwb_source->stUCIState.bIsInCmd = true;

	pst_uwb_source->stUCIState.eMSGIdx = UWB_Anchor_WakeUp_RQ;

	ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);

	return ResCode;
}

E_UWBErrCode UQUWBAnchorRangingSetup(ST_UWBSource* pst_uwb_source, uint8_t * cmd, uint16_t* lens)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ST_NXPUCIDat*			UCISend			= &pst_uwb_source->stUCIState.stUCISend;

	/* ----------------------------------------------
	DebugInfo:
	CORE_CONFIG_SET 指令配置参数应从 Flash读取，接口未完成，目前写成静态代码
	*/
	const uint8_t DEBUG_CORE_CONFIG_SET[] = {
		0x01, 0x01, 0x00, //不使能低功耗
		//0xF4, 0x01, 0x01
	};

	//step 1. CORE_CONFIG_SET
	*UCISend->pbufDatBody = 1;//param num;
	core_mm_copy(UCISend->pbufDatBody + 1, DEBUG_CORE_CONFIG_SET, sizeof(DEBUG_CORE_CONFIG_SET));
	UCISend->u16DatBodyLens = 1 + sizeof(DEBUG_CORE_CONFIG_SET);

	pst_uwb_source->stUCIState.bIsInCmd = true;
	pst_uwb_source->stUCIState.eMSGIdx = UWB_Core_Config_Setting_RQ;
	ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);
	if (UWB_Err_Success_0 == ResCode)
	{
		pst_uwb_source->stUCIState.stTimerTools.fpOSDelay(180);//need change to  use the os delay
		//step 2. INIT_RANGING_SESSION
		pst_uwb_source->stUCIState.stSession.u32CurrSessionID =	core_dcm_mku32(	pst_uwb_source->bufCANIn_1[0],
																				pst_uwb_source->bufCANIn_1[1],
																				pst_uwb_source->bufCANIn_1[2],
																				pst_uwb_source->bufCANIn_1[3]
																				); //MSB KW38
		UCISend->pbufDatBody[0] = pst_uwb_source->bufCANIn_1[3];//LSB - > 29D5
		UCISend->pbufDatBody[1] = pst_uwb_source->bufCANIn_1[2];
		UCISend->pbufDatBody[2] = pst_uwb_source->bufCANIn_1[1];
		UCISend->pbufDatBody[3] = pst_uwb_source->bufCANIn_1[0];
		UCISend->pbufDatBody[4] = pst_uwb_source->stUCIState.stSession.u8UCIRangSessionMode;
		UCISend->u16DatBodyLens = 5;
		pst_uwb_source->stUCIState.bIsInCmd = true;
		pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_Init_RQ;
		ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);
		if (UWB_Err_Success_0 == ResCode)
		{
			pst_uwb_source->stUCIState.stTimerTools.fpOSDelay(180);//need change to  use the os delay
			//step 3. RANGING_APP_CONFIG
			const uint8_t DEBUG_APP_CONFIG_SET[] = {
					0x02, 0x01, 0x01,
					0x04, 0x01, 0x09,
					0x05, 0x01, 0x08,
					0x06, 0x02, 0x00, 0x01,
					0x08, 0x02, 0xB0, 0x04,
					0x09, 0x04, 0x60, 0x00, 0x00, 0x00,
					0x0A, 0x04, 0x00, 0x00, 0x00, 0x00,
					0x11, 0x01, 0x00,
					0x14, 0x01, 0x0A,
					0x15, 0x01, 0x00,
					0x1B, 0x01, 0x0C,
					0x1E, 0x01, (0x01 + UWB_ANCHOR_INDEX),
					0x23, 0x01, 0x03,
					0x25, 0x01, 0x60,
					0x2A, 0x02, 0x00, 0x00,
					0x2C, 0x01, 0x00,
					0x32, 0x02, 0xFF, 0xFF,
					0xA0, 0x10, 0x73, 0x05, 0x8F, 0xE5, 0x50, 0x3C, 0x1D, 0x4A, 0x17, 0x54, 0x1D, 0x51, 0x21, 0xBB, 0x7F, 0x9E,
					0xA4, 0x02, 0x00, 0x00,
					0xA5, 0x01, 0x11,
					0xA6, 0x02, 0xD0, 0x02,
					0xE9, 0x12, 0x04, 0x00, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54,
					0xE9, 0x12, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54,
					0xF0, 0x01, 0x00,
					0xF2, 0x01, 0x0C,
					0xF9, 0x01, 0x00,
					0xFB, 0x04, 0x02, 0x03, 0x12, 0x13,
					0xFD, 0x02, 0xFF, 0x01,
			};
			UCISend->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
			UCISend->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
			UCISend->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
			UCISend->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
			UCISend->pbufDatBody[4] = 0x1C;//param num
			core_mm_copy(UCISend->pbufDatBody + 5, DEBUG_APP_CONFIG_SET, sizeof(DEBUG_APP_CONFIG_SET));
			UCISend->u16DatBodyLens = 5 + sizeof(DEBUG_APP_CONFIG_SET);
			pst_uwb_source->stUCIState.bIsInCmd = true;
			pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_App_Config_RQ;
			ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);
			pst_uwb_source->stUCIState.stTimerTools.fpOSDelay(180);//need change to  use the os delay
		}
		else
		{
			// return .
		}
	}
	else
	{
		// return .
	}
	return ResCode;
}

//input param cmdtype:0-suspend 1-close ranging session
E_UWBErrCode UQUWBAnchorRangingControl(E_RangingOPType type, ST_UWBSource* pst_uwb_source, uint8_t* cmdin, uint16_t* cmdinlens)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ST_NXPUCIDat*			UCISend			= &pst_uwb_source->stUCIState.stUCISend;
	if (UWBRangingOPType_Start == type)
	{
		if(pst_uwb_source->bIsCanBuffLink)
		{
			core_mm_set(pst_uwb_source->bufCANIn_2, 0, DEF_UWB_CAN_BUF_SIZE);
			core_mm_copy(pst_uwb_source->bufCANIn_2, cmdin, *cmdinlens);

			ResCode = UQUWBAnchorRangingSetup(pst_uwb_source, cmdin, *cmdinlens);
			if (UWB_Err_Success_0 == ResCode)
			{
				//step 4. Start_RANGING_SESSION	
				UCISend->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->u16DatBodyLens = 4;
				pst_uwb_source->stUCIState.bIsInCmd = true;
				pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_Start_RQ;
				ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);
			}
			else
			{
				//do nothing .
			}

		}
		else
		{

		}
		pst_uwb_source->bIsCanBuffLink = false;
	}
	else if ((UWBRangingOPType_Suspend == type) \
		|| (UWBRangingOPType_Recover == type) \
		|| (UWBRangingOPType_RecoverAndConfig == type) \
		|| (UWBRangingOPType_Stop == type))
	{
		UCISend->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
		UCISend->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
		UCISend->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
		UCISend->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
		UCISend->u16DatBodyLens = 4;
		pst_uwb_source->stUCIState.bIsInCmd = true;
		pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_Stop_RQ;
		ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);
		if(UWB_Err_Success_0 == ResCode)
		{
			if(UWBRangingOPType_Stop == type)
			{
				UCISend->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->u16DatBodyLens = 4;
				pst_uwb_source->stUCIState.bIsInCmd = true;
				pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_Deinit_RQ;
				ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);
			}
		}
		//........
	}

	return ResCode;
}

E_UWBErrCode UQUWBAnchorRangingDataNTF(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	pst_uwb_source->stUCIState.bIsInCmd = true;
	pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Result_Notice;
	ResCode = API_UWB_Responder_WorkProcedure(pst_uwb_source);
	return ResCode;
}

#elif defined(UWB_INITIATOR)

ST_UQCMDSeqAnchorWakeupRQ	stUQCMDSeqAnchorWakeup;
ST_UQCMDSeqRangingStop		stUQCMDSeqRangingSTOP;
ST_UQCMDSeqRangingRecover	stUQCMDSeqRangingRecover;
ST_UQCMDSeqRangingStart		stUQCMDSeqRangingStart;

const uint8_t CORE_CONFIG_CMD_Template[] = {
	0x20, 0x04, 0x00, 0x08,
	0x02,
	0x01, 0x01, 0x00,//停用低功耗
	0xEA, 0x02, 0x10, 0x27//从低功耗退出到运行10ms
};

const uint8_t base_device_config[] = {
    0x01,//config index
    0x01,//config type
    0x02,//config NUM
    0x07,//lens
    0x01, 0x01, 0x00,
    0xEA, 0x02, 0x10, 0x27
};
const uint8_t base_app_config[] = {
    0x01,//config index
    0x02,//config type
    0x19,//config NUM
    0x8C, 0x00,//lens
    0x04, 0x01, 0x05,
    0x05, 0x01, 0x04,
    0x06, 0x02, 0x00, 0x00,
    0x09, 0x04, 0x60, 0x00, 0x00, 0x00,
    0x08, 0x02, 0xB0, 0x04,
    0x0A, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x11, 0x01, 0x01,
    0x14, 0x01, 0x0A,
    0x15, 0x01, 0x00,
    0x1B, 0x01, 0x0C,
    0x2A, 0x02, 0x00, 0x00,
    0x2C, 0x01, 0x00,
    0x23, 0x01, 0x03,
    0x32, 0x02, 0xFF, 0xFF,
    0xE9, 0x12, 0x04, 0x00, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54,
    0xE9, 0x12, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54,
    0xF2, 0x01, 0x14,
    0xA0, 0x10, 0x73, 0x05, 0x8F, 0xE5, 0x50, 0x3C, 0x1D, 0x4A, 0x17, 0x54, 0x1D, 0x51, 0x21, 0xBB, 0x7F, 0x9E,
    0xA4, 0x02, 0x00, 0x00,
    0xA5, 0x01, 0x11,
    0xA6, 0x02, 0xD0, 0x02,
    0xF0, 0x01, 0x00,
    0xFB, 0x04, 0x02, 0x03, 0x12, 0x13,
    0xFD, 0x02, 0xFF, 0x01,
    0xF9, 0x01,0X01
};


E_UWBErrCode UQUWBFobInit(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ResCode = API_UWB_Initiator_Init(pst_uwb_source);
	return ResCode;
}

E_UWBErrCode UQUWBFobReset(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	pst_uwb_source->stUCIState.bIsInCmd = true;
	pst_uwb_source->stUCIState.eMSGIdx = UWB_Hard_Reset_RQ;
	ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
	return ResCode;
}

E_UWBErrCode UQUWBFobGetCapblity(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;

	pst_uwb_source->stUCIState.bIsInCmd = true;
	
	pst_uwb_source->stUCIState.eMSGIdx = UWB_Anchor_WakeUp_RQ;
	
	ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
	
	return ResCode;
}

E_UWBErrCode UQUWBFobRangingSetup(ST_UWBSource* pst_uwb_source, uint8_t * cmd, uint16_t* lens)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ST_NXPUCIDat*			UCISend			= &pst_uwb_source->stUCIState.stUCISend;
	
	/* ----------------------------------------------
		DebugInfo:
		CORE_CONFIG_SET 指令配置参数应从 Flash读取，接口未完成，目前写成静态代码
	 */
	const uint8_t DEBUG_CORE_CONFIG_SET[] = {
		0x01, 0x01, 0x00, //不使能低功耗
		//0xEA, 0x02, 0xFF, 0xFF//设置进入HPD（硬掉电模式的时间）10000（ms)
	};
	//step 1. CORE_CONFIG_SET
	*UCISend->pbufDatBody = 1;
	core_mm_copy(UCISend->pbufDatBody + 1, DEBUG_CORE_CONFIG_SET, sizeof(DEBUG_CORE_CONFIG_SET));
	UCISend->u16DatBodyLens = 1 + sizeof(DEBUG_CORE_CONFIG_SET);

	pst_uwb_source->stUCIState.bIsInCmd = true;
	pst_uwb_source->stUCIState.eMSGIdx = UWB_Core_Config_Setting_RQ;
	ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
	if (UWB_Err_Success_0 == ResCode)
	{
		pst_uwb_source->stUCIState.stTimerTools.fpOSDelay(180);//need change to  use the os delay
		//step 2. INIT_RANGING_SESSION
		pst_uwb_source->stUCIState.stSession.u32CurrSessionID =	core_dcm_mku32(	pst_uwb_source->bufCANIn_1[0],
																				pst_uwb_source->bufCANIn_1[1],
																				pst_uwb_source->bufCANIn_1[2],
																				pst_uwb_source->bufCANIn_1[3]
																				); //MSB KW38
		UCISend->pbufDatBody[0] = pst_uwb_source->bufCANIn_1[3];//LSB - > 29D5
		UCISend->pbufDatBody[1] = pst_uwb_source->bufCANIn_1[2];
		UCISend->pbufDatBody[2] = pst_uwb_source->bufCANIn_1[1];
		UCISend->pbufDatBody[3] = pst_uwb_source->bufCANIn_1[0];
		UCISend->pbufDatBody[4] = pst_uwb_source->stUCIState.stSession.u8UCIRangSessionMode;
		UCISend->u16DatBodyLens = 5;
		pst_uwb_source->stUCIState.bIsInCmd = true;
		pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_Init_RQ;
		ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
		if (UWB_Err_Success_0 == ResCode)
		{
			pst_uwb_source->stUCIState.stTimerTools.fpOSDelay(180);//need change to  use the os delay
			//step 3. RANGING_APP_CONFIG
			const uint8_t DEBUG_APP_CONFIG_SET[] = {
					0x02, 0x01, 0x01,
					0x04, 0x01, 0x05,
					0x05, 0x01, 0x08,
					0x06, 0x02, 0x00, 0x00,
					0x08, 0x02, 0xC0, 0x12,
					//Modify (Ken):FOB-V0501 NO.1 -20231225
					#if defined __FIT_Aeon_H
					0x09, 0x04, 0xC0, 0x00, 0x00, 0x00,
					#else
					0x09, 0x04, 0x60, 0x00, 0x00, 0x00,
					#endif
					0x0A, 0x04, 0x00, 0x00, 0x00, 0x00,
					0x11, 0x01, 0x01,
					0x14, 0x01, 0x0A,
					0x15, 0x01, 0x00,
					0x1B, 0x01, 0x0C,
					0x1C, 0x01, 0x01,
					0x23, 0x01, 0x03,
					0x2A, 0x02, 0x00, 0x00,
					0x2C, 0x01, 0x00,
					0x32, 0x02, 0xFF, 0xFF,
					0xA0, 0x10, 0x73, 0x05, 0x8F, 0xE5, 0x50, 0x3C, 0x1D, 0x4A, 0x17, 0x54, 0x1D, 0x51, 0x21, 0xBB, 0x7F, 0x9E,
					0xA4, 0x02, 0x00, 0x00,
					0xA5, 0x01, 0x11,
					0xA6, 0x02, 0xD0, 0x02,
					0xE9, 0x12, 0x04, 0x00, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54,
					0xE9, 0x12, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x43, 0x54, 0x53, 0x53, 0x54, 0x41, 0x54,
					0xF0, 0x01, 0x00,
					//Modify (Ken):FOB-V0501 NO.1 -20231225
					#if defined __FIT_Aeon_H
					0xF2, 0x01, 0x38,
					#else
					0xF2, 0x01, 0x0C,
					#endif
					0xF9, 0x01, 0x00,
					0xFB, 0x04, 0x02, 0x03, 0x12, 0x13,
					0xFD, 0x02, 0xFF, 0x01,
					0xFE, 0x01, 0x01
			};
			UCISend->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
			UCISend->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
			UCISend->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
			UCISend->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
			UCISend->pbufDatBody[4] = 0x1C;//param num
			core_mm_copy(UCISend->pbufDatBody+5, DEBUG_APP_CONFIG_SET, sizeof(DEBUG_APP_CONFIG_SET));
			UCISend->u16DatBodyLens = 5 + sizeof(DEBUG_APP_CONFIG_SET);
			pst_uwb_source->stUCIState.bIsInCmd = true;
			pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_App_Config_RQ;
			ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
			pst_uwb_source->stUCIState.stTimerTools.fpOSDelay(180);//need change to  use the os delay
		}
		else
		{
			// return .
		}
	}
	else
	{
		// return .
	}
	return ResCode;
}

//input param cmdtype:0-suspend 1-close ranging session
E_UWBErrCode UQUWBFobRangingControl(E_RangingOPType type, ST_UWBSource* pst_uwb_source, uint8_t* cmdin, uint16_t* cmdinlens)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ST_NXPUCIDat*			UCISend			= &pst_uwb_source->stUCIState.stUCISend;
	if (UWBRangingOPType_Start == type)
	{
		if(pst_uwb_source->bIsCanBuffLink)
		{
			core_mm_set(pst_uwb_source->bufCANIn_2, 0, DEF_UWB_CAN_BUF_SIZE);
			core_mm_copy(pst_uwb_source->bufCANIn_2, cmdin, *cmdinlens);

			ResCode = UQUWBFobRangingSetup(pst_uwb_source, cmdin, cmdinlens);
			if (UWB_Err_Success_0 == ResCode)
			{
				//step 4. Start_RANGING_SESSION
				UCISend->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->u16DatBodyLens = 4;
				pst_uwb_source->stUCIState.bIsInCmd = true;
				pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_Start_RQ;
				ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
			}
			else
			{
				//do nothing .
			}

		}
		else
		{

		}
		pst_uwb_source->bIsCanBuffLink = false;
	}
	else if ((UWBRangingOPType_Suspend == type) \
			|| (UWBRangingOPType_Recover == type) \
			|| (UWBRangingOPType_RecoverAndConfig == type) \
			|| (UWBRangingOPType_Stop == type))
	{
		UCISend->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
		UCISend->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
		UCISend->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
		UCISend->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
		UCISend->u16DatBodyLens = 4;
		pst_uwb_source->stUCIState.bIsInCmd = true;
		pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_Stop_RQ;
		ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
		if(UWB_Err_Success_0 == ResCode)
		{
			if(UWBRangingOPType_Stop == type)
			{
				UCISend->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_uwb_source->stUCIState.stSession.u32CurrSessionID));
				UCISend->u16DatBodyLens = 4;
				pst_uwb_source->stUCIState.bIsInCmd = true;
				pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Session_Deinit_RQ;
				ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
			}
		}
		//........
	}

	return ResCode;
}

E_UWBErrCode UQUWBFobRangingDataNTF(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	pst_uwb_source->stUCIState.bIsInCmd = true;
	pst_uwb_source->stUCIState.eMSGIdx = UWB_Ranging_Result_Notice;
	ResCode = API_UWB_Initiator_WorkProcedure(pst_uwb_source);
	return ResCode;
}
#else
?
#endif //@ if define UWB_RESPONDER/UWB_INITIATOR

E_UWBErrCode UQUWBCustCMD(ST_CustCmd* pst_CustCmd)
{
	return 0;
}

#if 0 
abandoned code
//unimplemented functionality
static int get_curr_device_confg(uint8_t* cfg);
static int get_curr_app_config(uint8_t* cfg);
//return Null -  no result
//       other - result address
static uint8_t* tools_tlv_find_byLens(uint8_t target, uint8_t* src, const uint16_t srclens)
{
	uint8_t* start = src;
	uint8_t* end = src + srclens;

	while (start < end)
	{
		if (target == *start)
			return start;

		start += 2 + *(start+1);
	}

	return NULL;
}

//return Null -  no result
//       other - result address
static uint8_t* tools_tlv_find_byNum(uint8_t target, uint8_t* src, const uint16_t TlvTotNum)
{
	uint8_t* start = src;

	for (int i = 0; i < TlvTotNum; i++)
	{
		if (target == *start)
			return start;
		start += 2 + *(start+1);
	}

	return NULL;
}

#endif
