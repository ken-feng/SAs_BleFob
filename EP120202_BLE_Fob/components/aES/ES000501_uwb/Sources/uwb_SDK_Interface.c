/*
 * uwb_api_interface.c
 *
 *  Created on: 2022年7月1日
 *      Author: JohnSong
 */
#include "./uwb_common_def.h"
#include "./uwb_SDK_Interface.h"
#include "./UQ_UWB_Frame/UQ_UWB_Frame.h"
#include "./FilterOptimization/uwb_filter_optimization.h"
#if defined (UWB_RESPONDER)
#include "../../../aES/ES010901_ccc_sdk/ccc_protocol/ccc_can.h"
#include "../../ES010601_ble_ccc/ble_ccc.h"
#endif
static int API_UQ_Device_Init			(fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Device_Reset			(fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Get_Caps				(ST_UWBProtocol* pst_ptl, fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Set_Caps				(ST_UWBProtocol* pst_ptl, fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Anchor_Wakup			(uint8_t* cmd, uint16_t cmdlens, fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Time_Sync				(uint8_t* cmd, uint16_t cmdlens, fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Ranging_Session_Setup	(uint8_t* cmd, uint16_t cmdlens, fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Ranging_Ctrl			(E_RangingOPType type, uint8_t* cmd, uint16_t cmdlens, fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_CustCMD				(ST_CustCmd* pst_cust_cmd, fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Ranging_NTF_Cache		(fp_UQ_MSGSend_t fp_send_msg);
static int API_UQ_Ranging_Result		(fp_UQ_MSGSend_t fp_send_msg);
#ifdef _DEBUG

static int DEBUG_MSG_SEND(E_UWBControlMessageIndex msgtype, uint8_t* pcmd, uint16_t* pcmdlens);
static int DEBUG_CONFIG_W(uint8_t CfgIdx, uint8_t* pBuf, const uint16_t BufLens);
static int DEBUG_CONFIG_R(uint8_t CfgIdx, uint8_t* pBuf, uint16_t* pMaxBufLens);
#endif

ST_UWBSDKInterface 	stUWBSDK;
ST_UWBSource 		stSource;
uint8_t FAnchor_Version[3] = {0x80, 0x01, 0x00};
static E_UWBErrCode get_local_index(uint8_t* local_index)
{
#if defined(UWB_RESPONDER)
	//const uint8_t bufLocidxInitiator[1] = {0};
	uint8_t bitlocal = 0;

	bitlocal = (uint8_t)(0x01 + UWB_ANCHOR_INDEX);
	switch(bitlocal)
	{
		case 1 : *local_index = 0x01;break;
		case 2 : *local_index = 0x02;break;
		case 3 : *local_index = 0x04;break;
		case 4 : *local_index = 0x08;break;
		case 5 : *local_index = 0x10;break;
		default:
			*local_index = 0xFF;break;
	}
	//const uint8_t bufLocidxInitiator[1] = {(0x01+UWB_ANCHOR_INDEX)};
#elif defined(UWB_INITIATOR)
	const uint8_t bufLocidxInitiator[1] = {0x80};
	*local_index = bufLocidxInitiator[0];
#else
?
#endif

	return UWB_Err_Success_0;
}

static E_UWBErrCode get_hardware_sn(uint8_t* ptr_buffer_sn, uint8_t lens)
{
	const uint8_t bufHardwareSN[0x10] = {   0xA1,
		0xB2,
		0xC2,
		0xD2,
		0xE2,
		0xF2,
		0x02,
		0x02,
		0x02,
		0x02,
		0x02,
		0x02,
		0x02,
		0x02,
		0x02,
		0xEE};

	core_mm_copy(ptr_buffer_sn, bufHardwareSN, lens);
}

#if defined(UWB_RESPONDER)
int API_UQ_Device_Init(fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ResCode = UQUWBAnchorInit(&stSource);
#if defined(UWB_RESPONDER)
	get_local_index(&stSource.stUCIState.u8UWBLocalcIndex);
#endif
	return ResCode;
}

int API_UQ_Device_Reset(fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ResCode = UQUWBAnchorReset(&stSource);
#if defined(UWB_RESPONDER)
	FAnchor_Version[0] = (uint8_t)UWB_ANCHOR_INDEX + 1;
#endif
	LOG_S("UCI Specification Version : 01 05\n");
	LOG_S("CCC Specification Version : 30 2e 32 2e 36 00 00 00\n");
	LOG_S("Device Name : 52 41 4e 47 45 52 34 00\n");
	LOG_S("Firmware Version : 02 00 00\n");
	LOG_S("Dsp Version : 02 12 00\n");
	LOG_S("Ranger4 Version : 04 2c \n");
	LOG_S_HEX("UQUWB SDK Version : ", FAnchor_Version, sizeof(FAnchor_Version));
	return ResCode;
}

int API_UQ_Anchor_Wakup(uint8_t* cmdin, uint16_t cmdinlens, fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode = UWB_Err_Success_0;
	uint8_t*				canbuf	= stSource.bufCANOut;
	uint16_t				sendlen = 0;

	if(Module_Stat_Active == stSource.stUCIState.eWorkStat)
	{
		ResCode = UQUWBAnchorRangingControl(UWBRangingOPType_Stop, &stSource, cmdin, cmdinlens);
		if(UWB_Err_Success_0 != ResCode)
		{
			core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
			*canbuf = UQRESC_WakeupFailed;
			fp_send_msg(UWBRangingOPType_Stop, stSource.bufCANOut, &sendlen);
			return ResCode;
		}
		stSource.stUCIState.stTimerTools.fpOSDelay(180);//need change to  use the os delay
		LOG_L_S(CAN_MD,"RANGING STOP DONE . \r\n");

	}
	else
	{
		ResCode = UQUWBAnchorGetCapblity(&stSource);
		LOG_L_S(CAN_MD,"RANGING WAKEUP DONE . \r\n");
		if (UWB_Err_Success_0 == ResCode)
		{
			ST_UWBCaps*				pstCaps = &stSource.stUCIState.stProtocol.stCCCCaps;
			//ofst 0x0  res
			*canbuf = UQRESC_Success_0;														canbuf += 1L;

			get_hardware_sn(canbuf, 0x10);  												canbuf += 16L;

			get_local_index(canbuf);		 												canbuf += 1L;

			*canbuf = 1;																	canbuf += 1L;

			*canbuf = pstCaps->u8CCCConfigIDLens; 											canbuf += 1L;

			core_mm_copy(canbuf, pstCaps->bufCCCConfigID, pstCaps->u8CCCConfigIDLens);		canbuf += pstCaps->u8CCCConfigIDLens;

			*canbuf = pstCaps->u8PulseShapeCombolens; 										canbuf +=1L;

			core_mm_copy(canbuf, pstCaps->bufCCCShapeCombo, pstCaps->u8PulseShapeCombolens);canbuf += pstCaps->u8PulseShapeCombolens;

			*canbuf = pstCaps->u8ChannelBitMaskMap; 										canbuf += 1L;

			*canbuf = 1; 																	canbuf += 1L;// Mini RAN Multiplier

			*canbuf = pstCaps->u8SlotBitMask;												canbuf += 1L;

			*canbuf = core_dcm_u16_hi(core_dcm_u32_hi(pstCaps->u32SyncCodeBitMask)); 		canbuf += 1L;
			*canbuf = core_dcm_u16_lo(core_dcm_u32_hi(pstCaps->u32SyncCodeBitMask)); 		canbuf += 1L;
			*canbuf = core_dcm_u16_hi(core_dcm_u32_lo(pstCaps->u32SyncCodeBitMask)); 		canbuf += 1L;
			*canbuf = core_dcm_u16_lo(core_dcm_u32_lo(pstCaps->u32SyncCodeBitMask)); 		canbuf += 1L;

			*canbuf = pstCaps->u8HopingConfigBitMask;										canbuf += 1L;

			sendlen = canbuf - stSource.bufCANOut;
			fp_send_msg(UWB_Anchor_WakeUp_RS, stSource.bufCANOut, &sendlen);
		}
		else
		{
			core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
			*canbuf = UQRESC_WakeupFailed;
			fp_send_msg(UWB_Anchor_WakeUp_RS, stSource.bufCANOut, &sendlen);
		}
	}


	return ResCode;
}
int API_UQ_Ranging_Session_Setup(uint8_t* cmdin, uint16_t cmdinlens, fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	core_mm_set(stSource.bufCANIn_1, 0, DEF_UWB_CAN_BUF_SIZE);
	core_mm_copy(stSource.bufCANIn_1, cmdin, cmdinlens);
	stSource.bIsCanBuffLink = true;
	return ResCode;
}

int API_UQ_Ranging_Ctrl(E_RangingOPType type, uint8_t* cmdin, uint16_t cmdinlens, fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	uint8_t*				canbuf			= stSource.bufCANOut;
	uint16_t				sendlens		= 0;

	if (UWBRangingOPType_Start == type)
	{
		if (true == stSource.bIsCanBuffLink)
		{
			ResCode = UQUWBAnchorRangingControl(type, &stSource, cmdin, &cmdinlens);
			if (UWB_Err_Success_0 == ResCode)
			{
				*canbuf = UQRESC_Success_0; canbuf += 1;
				*canbuf = core_dcm_u16_hi(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
				*canbuf = core_dcm_u16_lo(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
				*canbuf = core_dcm_u16_hi(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
				*canbuf = core_dcm_u16_lo(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
				get_local_index(canbuf);
				sendlens = 6;
				fp_send_msg(UWB_Ranging_Session_Start_RS, stSource.bufCANOut, &sendlens);
				stSource.u16HistoryPos = 0;
				stSource.u16HistoryNeg = 0;
				stSource.u16LastDistan = 0xFFFF;
				stSource.stUCIState.stSession.u32STSIndex = 0;


			}
			else
			{
				*canbuf = UQRESC_RangingStartFailed;
			}
		}
		else
		{
			core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
			*canbuf = UQRESC_RangingStartFailed;
			sendlens = 1;
			fp_send_msg(UWB_Ranging_Session_Start_RS, stSource.bufCANOut, &sendlens);
		}
		stSource.bIsCanBuffLink = false;
	}
	else if ((UWBRangingOPType_Suspend == type) \
		|| (UWBRangingOPType_Recover == type) \
		|| (UWBRangingOPType_RecoverAndConfig == type) \
		|| (UWBRangingOPType_Stop == type))
	{
		if(UWBRangingOPType_Stop == type)
		{
			if(Module_Stat_Active == stSource.stUCIState.eWorkStat)
			{
				ResCode = UQUWBAnchorRangingControl(UWBRangingOPType_Stop, &stSource, cmdin, &cmdinlens);
				if(UWB_Err_Success_0 == ResCode)
				{
					core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
					*canbuf = UQRESC_Success_0;			canbuf = canbuf + 1UL;
					*canbuf = UWBRangingOPType_Stop;	canbuf = canbuf + 1UL;
					*canbuf = core_dcm_u16_hi(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
					*canbuf = core_dcm_u16_lo(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
					*canbuf = core_dcm_u16_hi(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
					*canbuf = core_dcm_u16_lo(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
					*canbuf = stSource.stUCIState.u8UWBLocalcIndex;
					sendlens = canbuf - stSource.bufCANOut;
					fp_send_msg(UWB_Ranging_Session_Stop_RS, stSource.bufCANOut, &sendlens);
					return ResCode;
				}
				else
				{
					//...
				}
			}
			else
			{
				//...
			}
			core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
			*canbuf = UQRESC_RangingStopFailed;
			fp_send_msg(UWB_Anchor_WakeUp_RS, stSource.bufCANOut, &sendlens);
			return ResCode;
		}
		else if(UWBRangingOPType_Suspend == type)
		{
			return ResCode;
		}
		else if(UWBRangingOPType_RecoverAndConfig == type)
		{
			return ResCode;
		}

		//.....
	}
	return ResCode;
}

static int API_UQ_Ranging_NTF_Cache(fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode = UWB_Err_Success_0;
	//uint8_t*				canbuf	= stSource.bufCANOut;

	if(Session_Stat_Aciv != stSource.stUCIState.stSession.eSesionStat)
	{
		return UWB_Err_UCI_Status_Rejected;
	}

	core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
	ResCode = UQUWBAnchorRangingDataNTF(&stSource);


	if (UWB_Err_Success_0 == ResCode)
	{

		if (2 == stSource.stUCIState.stSession.u16FilterCnt)
		{
			stSource.stUCIState.stSession.u16FilterCnt = 0;


			if(stSource.stUCIState.stSession.u32STSIndex < 5)
			{
				return ResCode;
			}

			ResCode = API_UQ_Ranging_Result(fp_send_msg);
			distance_queue_refresh(&stSource.stUCIState.stRaningDat, stSource.stUCIState.stSession.u16CurrentDistance);
			stSource.stUCIState.stSession.u16FilterCnt = stSource.stUCIState.stSession.u16FilterCnt  + 1UL;
		}
		else
		{
			distance_queue_refresh(&stSource.stUCIState.stRaningDat, stSource.stUCIState.stSession.u16CurrentDistance);
			stSource.stUCIState.stSession.u16FilterCnt = stSource.stUCIState.stSession.u16FilterCnt   + 1UL;

		}
	}
	else
	{
		//do nothing .
	}
	return ResCode;
}

int API_UQ_Ranging_Result(fp_UQ_MSGSend_t callbackfun)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	uint8_t*				canbuf			= stSource.bufCANOut;
	uint16_t				sendlens		= 0;
	uint16_t 				u16tempbuf[5]	= {0};
	int 					i				= 0;

	core_mm_copy(u16tempbuf, stSource.stUCIState.stRaningDat.bufDistQueue, 10);

	if(0 == stSource.u16HistoryPos)
	{
		for(i = 0; i < 5; i++)
		{
			if((0 != stSource.stUCIState.stRaningDat.bufDistQueue[i]) && (0xFFFF != stSource.stUCIState.stRaningDat.bufDistQueue[i]))
			{
				stSource.u16HistoryPos = stSource.stUCIState.stRaningDat.bufDistQueue[i] + AllowedStepWithE;
				stSource.u16HistoryNeg = stSource.stUCIState.stRaningDat.bufDistQueue[i] > AllowedStepWithE ?  stSource.stUCIState.stRaningDat.bufDistQueue[i] - AllowedStepWithE : 0;
				invalid_data_kick_out_with_5_data(&stSource.u16HistoryPos, &stSource.u16HistoryNeg, stSource.stUCIState.stRaningDat.bufDistQueue);
				distance_check_and_fixup_with_5_data(&stSource.u16LastDistan, stSource.stUCIState.stRaningDat.bufDistQueue);
				break;
			}
		}
	}
	else
	{
		invalid_data_kick_out_with_5_data(&stSource.u16HistoryPos, &stSource.u16HistoryNeg, stSource.stUCIState.stRaningDat.bufDistQueue);

		distance_check_and_fixup_with_5_data(&stSource.u16LastDistan, stSource.stUCIState.stRaningDat.bufDistQueue);
	}

	LOG_L_S_HEX(CCC_MD,"Anchor-Distance List : ", stSource.stUCIState.stRaningDat.bufDistQueue, 10);
	*canbuf = 0x00; canbuf += 1;

	*canbuf = core_dcm_u16_hi(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
	*canbuf = core_dcm_u16_lo(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
	*canbuf = core_dcm_u16_hi(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
	*canbuf = core_dcm_u16_lo(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;

	*canbuf = core_dcm_u16_hi(core_dcm_u32_hi(stSource.stUCIState.stSession.u32STSIndex)); canbuf += 1;
	*canbuf = core_dcm_u16_lo(core_dcm_u32_hi(stSource.stUCIState.stSession.u32STSIndex)); canbuf += 1;
	*canbuf = core_dcm_u16_hi(core_dcm_u32_lo(stSource.stUCIState.stSession.u32STSIndex)); canbuf += 1;
	*canbuf = core_dcm_u16_lo(core_dcm_u32_lo(stSource.stUCIState.stSession.u32STSIndex)); canbuf += 1;

	*canbuf = stSource.stUCIState.u8UWBLocalcIndex;canbuf += 1;

	*canbuf = core_dcm_u16_hi(stSource.stUCIState.stRaningDat.bufDistQueue[0]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(stSource.stUCIState.stRaningDat.bufDistQueue[0]); canbuf += 1;

	*canbuf = core_dcm_u16_hi(stSource.stUCIState.stRaningDat.bufDistQueue[1]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(stSource.stUCIState.stRaningDat.bufDistQueue[1]); canbuf += 1;

	*canbuf = core_dcm_u16_hi(stSource.stUCIState.stRaningDat.bufDistQueue[2]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(stSource.stUCIState.stRaningDat.bufDistQueue[2]); canbuf += 1;


	*canbuf = core_dcm_u16_hi(stSource.stUCIState.stRaningDat.bufDistQueue[3]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(stSource.stUCIState.stRaningDat.bufDistQueue[3]); canbuf += 1;

	*canbuf = core_dcm_u16_hi(stSource.stUCIState.stRaningDat.bufDistQueue[4]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(stSource.stUCIState.stRaningDat.bufDistQueue[4]); canbuf += 1;


	//debug data

	*canbuf = core_dcm_u16_hi(u16tempbuf[0]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(u16tempbuf[0]); canbuf += 1;

	*canbuf = core_dcm_u16_hi(u16tempbuf[1]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(u16tempbuf[1]); canbuf += 1;

	*canbuf = core_dcm_u16_hi(u16tempbuf[2]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(u16tempbuf[2]); canbuf += 1;


	*canbuf = core_dcm_u16_hi(u16tempbuf[3]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(u16tempbuf[3]); canbuf += 1;

	*canbuf = core_dcm_u16_hi(u16tempbuf[4]); canbuf += 1;
	*canbuf = core_dcm_u16_lo(u16tempbuf[4]); canbuf += 1;


	//*canbuf = bufDistan[0];canbuf += 1;
	//*canbuf = bufDistan[1];canbuf += 1;

	//sendlens = 12;
	//sendlens = 20;
	sendlens = 20 + 10;
	callbackfun(UWB_Ranging_Result_Notice, stSource.bufCANOut, &sendlens);
	stSource.stUCIState.stTimerTools.fpOSDelay(50);//need change to  use the os delay
	return ResCode;
}
int API_UQ_CustCMD(ST_CustCmd* pstCustCmd, fp_UQ_MSGSend_t callbackfun)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ResCode = UQUWBCustCMD(pstCustCmd);
	return ResCode;
}
int API_UQ_Get_Caps(ST_UWBProtocol* pst_ptl, fp_UQ_MSGSend_t callbackfun)
{
	//pst_ptl->stCCCCaps.
	//UWBErrCode			ResCode 		= UWB_Err_Success_0;
	//return ResCode;

	return UWB_Err_Success_0;
}
int API_UQ_Set_Caps(ST_UWBProtocol* pst_ptl, fp_UQ_MSGSend_t callbackfun)
{
	//UWBErrCode			ResCode 		= UWB_Err_Success_0;
	//return ResCode;
	return UWB_Err_Success_0;
}

int API_UQ_Time_Sync(uint8_t* cmd, uint16_t cmdlens, fp_UQ_MSGSend_t callbackfun)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	uint8_t*				canbuf			= stSource.bufCANOut;
	uint16_t				sendlens		= 0;
	*canbuf = 0x00; canbuf += 1;
	*canbuf = stSource.stUCIState.u8UWBLocalcIndex;canbuf += 1;
	sendlens = 1;
	callbackfun(UWB_Timer_Sync_RS, stSource.bufCANOut, &sendlens);

	return ResCode;
}
#elif defined(UWB_INITIATOR)

int API_UQ_Device_Init(fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	core_mm_set((u8*)&stSource,0x00,sizeof(stSource));
	ResCode = UQUWBFobInit(&stSource);
#if defined(UWB_RESPONDER)
	get_local_index(&stSource.stUCIState.u8UWBLocalcIndex);
#endif
	return ResCode;
}

int API_UQ_Device_Reset(fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ResCode = UQUWBFobReset(&stSource);
#if defined(UWB_RESPONDER)
	FAnchor_Version[0] = (uint8_t)UWB_ANCHOR_INDEX + 1;
#endif
	LOG_S("UCI Specification Version : 01 05\n");
	LOG_S("CCC Specification Version : 30 2e 32 2e 36 00 00 00\n");
	LOG_S("Device Name : 52 41 4e 47 45 52 34 00\n");
	LOG_S("Firmware Version : 02 00 00\n");
	LOG_S("Dsp Version : 02 12 00\n");
	LOG_S("Ranger4 Version : 04 2c\n");
	LOG_S_HEX("UQUWB SDK Version : ", FAnchor_Version, sizeof(FAnchor_Version));
	return ResCode;
}

int API_UQ_Anchor_Wakup(uint8_t* cmdin, uint16_t cmdinlens, fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode = UWB_Err_Success_0;
	uint8_t*				canbuf	= stSource.bufCANOut;
	uint16_t				sendlen = 0;

	if(Module_Stat_Active == stSource.stUCIState.eWorkStat)
	{
		ResCode = UQUWBFobRangingControl(UWBRangingOPType_Stop, &stSource, cmdin, cmdinlens);
		if(UWB_Err_Success_0 != ResCode)
		{
			core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
			*canbuf = UQRESC_WakeupFailed;
			fp_send_msg(UWBRangingOPType_Stop, stSource.bufCANOut, &sendlen);
			return ResCode;
		}
		stSource.stUCIState.stTimerTools.fpOSDelay(180);//need change to  use the os delay
		LOG_L_S(CAN_MD,"RANGING STOP DONE . \r\n");
	}
//	else
//	{
		ResCode = UQUWBFobGetCapblity(&stSource);
		if (UWB_Err_Success_0 == ResCode)
		{
			ST_UWBCaps*				pstCaps = &stSource.stUCIState.stProtocol.stCCCCaps;
			//ofst 0x0  res
			*canbuf = UQRESC_Success_0;														canbuf += 1L;

			get_hardware_sn(canbuf, 0x10);  												canbuf += 16L;

			get_local_index(canbuf);		 												canbuf += 1L;

			*canbuf = 1;																	canbuf += 1L;

			*canbuf = pstCaps->u8CCCConfigIDLens; 											canbuf += 1L;

			core_mm_copy(canbuf, pstCaps->bufCCCConfigID, pstCaps->u8CCCConfigIDLens);		canbuf += pstCaps->u8CCCConfigIDLens;

			*canbuf = pstCaps->u8PulseShapeCombolens; 										canbuf +=1L;

			core_mm_copy(canbuf, pstCaps->bufCCCShapeCombo, pstCaps->u8PulseShapeCombolens);canbuf += pstCaps->u8PulseShapeCombolens;

			*canbuf = pstCaps->u8ChannelBitMaskMap; 										canbuf += 1L;

			*canbuf = 1; 																	canbuf += 1L;// Mini RAN Multiplier

			*canbuf = pstCaps->u8SlotBitMask;												canbuf += 1L;
			*canbuf = core_dcm_u16_hi(core_dcm_u32_hi(pstCaps->u32SyncCodeBitMask)); 		canbuf += 1L;
			*canbuf = core_dcm_u16_lo(core_dcm_u32_hi(pstCaps->u32SyncCodeBitMask)); 		canbuf += 1L;
			*canbuf = core_dcm_u16_hi(core_dcm_u32_lo(pstCaps->u32SyncCodeBitMask)); 		canbuf += 1L;
			*canbuf = core_dcm_u16_lo(core_dcm_u32_lo(pstCaps->u32SyncCodeBitMask)); 		canbuf += 1L;

			*canbuf = pstCaps->u8HopingConfigBitMask;										canbuf += 1L;

			sendlen = canbuf - stSource.bufCANOut;
			fp_send_msg(UQCANMSG_Wakeup_RS, stSource.bufCANOut, &sendlen);
		}
		else
		{
			core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
			*canbuf = UQRESC_WakeupFailed;
			fp_send_msg(UQCANMSG_Wakeup_RS, stSource.bufCANOut, &sendlen);
		}
//	}

	return ResCode;
}
int API_UQ_Ranging_Session_Setup(uint8_t* cmdin, uint16_t cmdinlens, fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	core_mm_set(stSource.bufCANIn_1, 0, DEF_UWB_CAN_BUF_SIZE);
	core_mm_copy(stSource.bufCANIn_1, cmdin, cmdinlens);
	stSource.bIsCanBuffLink = true;
	return ResCode;
}

int API_UQ_Ranging_Ctrl(E_RangingOPType type, uint8_t* cmdin, uint16_t cmdinlens, fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	uint8_t*				canbuf			= stSource.bufCANOut;
	uint16_t				sendlens		= 0;
	/*
	if(UWBRangingOPType_Stop == type)//added by niull 0428
	{
			KW38_INT_Stop();
			INT_Clr_Flag();
			API_UQ_Device_Init(fp_send_msg);
			API_UQ_Device_Reset(fp_send_msg);
			return UWB_Err_Success_0;
	}
	*/
	if (UWBRangingOPType_Start == type)
	{
		if (true == stSource.bIsCanBuffLink)
		{
			ResCode = UQUWBFobRangingControl(type, &stSource, cmdin, &cmdinlens);
			if (UWB_Err_Success_0 == ResCode)
			{
				*canbuf = UQRESC_Success_0; canbuf += 1;
				*canbuf = core_dcm_u16_hi(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
				*canbuf = core_dcm_u16_lo(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
				*canbuf = core_dcm_u16_hi(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
				*canbuf = core_dcm_u16_lo(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
				get_local_index(canbuf);
				sendlens = 6;
				fp_send_msg(UWB_Ranging_Session_Start_RS, stSource.bufCANOut, &sendlens);
			}
			else
			{
				*canbuf = UQRESC_RangingStartFailed;
			}
		}
		else
		{
			core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
			*canbuf = UQRESC_RangingStartFailed;
			sendlens = 1;
			fp_send_msg(UWB_Ranging_Session_Start_RS, stSource.bufCANOut, &sendlens);
		}
		stSource.bIsCanBuffLink = false;
	}
	else if ((UWBRangingOPType_Suspend == type) \
		|| (UWBRangingOPType_Recover == type) \
		|| (UWBRangingOPType_RecoverAndConfig == type) \
		|| (UWBRangingOPType_Stop == type))
	{
		if(UWBRangingOPType_Stop == type)
		{
			if(Module_Stat_Active == stSource.stUCIState.eWorkStat)
			{
				ResCode = UQUWBFobRangingControl(UWBRangingOPType_Stop, &stSource, cmdin, &cmdinlens);
				if(UWB_Err_Success_0 == ResCode)
				{
					core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
					*canbuf = UQRESC_Success_0;			canbuf = canbuf + 1UL;
					*canbuf = UWBRangingOPType_Stop;	canbuf = canbuf + 1UL;
					*canbuf = core_dcm_u16_hi(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
					*canbuf = core_dcm_u16_lo(core_dcm_u32_hi(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
					*canbuf = core_dcm_u16_hi(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
					*canbuf = core_dcm_u16_lo(core_dcm_u32_lo(stSource.stUCIState.stSession.u32CurrSessionID)); canbuf += 1;
					*canbuf = stSource.stUCIState.u8UWBLocalcIndex;
					sendlens = canbuf - stSource.bufCANOut;
					fp_send_msg(UWB_Ranging_Session_Stop_RS, stSource.bufCANOut, &sendlens);
					return ResCode;
				}
				else
				{
					//...
				}
			}
			else
			{
				//...
			}
			core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
			*canbuf = UQRESC_RangingStopFailed;
			fp_send_msg(UWB_Anchor_WakeUp_RS, stSource.bufCANOut, &sendlens);
			return ResCode;
		}
		else if(UWBRangingOPType_Suspend == type)
		{
			return ResCode;
		}
		else if(UWBRangingOPType_RecoverAndConfig == type)
		{
			return ResCode;
		}

		//.....
	}
	return ResCode;
}
static int API_UQ_Ranging_NTF_Cache(fp_UQ_MSGSend_t fp_send_msg)
{
	E_UWBErrCode			ResCode = UWB_Err_Success_0;
	uint8_t*				canbuf	= stSource.bufCANOut;

	if(Session_Stat_Aciv != stSource.stUCIState.stSession.eSesionStat)
	{
		return UWB_Err_UCI_Status_Rejected;
	}

	core_mm_set(stSource.bufCANOut, 0, DEF_UWB_CAN_BUF_SIZE);
	ResCode = UQUWBFobRangingDataNTF(&stSource);
	if (UWB_Err_Success_0 == ResCode)
	{
#ifdef _DEBUG
		//stSource.stUCIState.stSession.u32STSIndex ++;
#endif
		if (2 == stSource.stUCIState.stSession.u16FilterCnt)
		{
			if(stSource.stUCIState.stSession.u32STSIndex < 5)
			{
				return ResCode;
			}
			ResCode = API_UQ_Ranging_Result(fp_send_msg);
			stSource.stUCIState.stSession.u16FilterCnt = stSource.stUCIState.stSession.u16FilterCnt  = 0;
		}
		else
		{
			stSource.stUCIState.stSession.u16FilterCnt = stSource.stUCIState.stSession.u16FilterCnt   + 1UL;
		}
	}
	else
	{
		//do nothing .
	}
	return ResCode;
}
int API_UQ_Ranging_Result(fp_UQ_MSGSend_t callbackfun)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	//ResCode = UQUWBFobRangingDataNTF(callbackfun);
	LOG_L_S_HEX(CCC_MD,"FOB",&stSource.stUCIState.stSession.u32ResponderBitMap, 4);
	stSource.stUCIState.stTimerTools.fpOSDelay(50);
	return ResCode;
}
int API_UQ_CustCMD(ST_CustCmd* pstCustCmd, fp_UQ_MSGSend_t callbackfun)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	ResCode = UQUWBCustCMD(pstCustCmd);
	return ResCode;
}
int API_UQ_Get_Caps(ST_UWBProtocol* pst_ptl, fp_UQ_MSGSend_t callbackfun)
{
	//pst_ptl->stCCCCaps.
	//UWBErrCode			ResCode 		= UWB_Err_Success_0;
	//return ResCode;

	return UWB_Err_Success_0;
}
int API_UQ_Set_Caps(ST_UWBProtocol* pst_ptl, fp_UQ_MSGSend_t callbackfun)
{
	//UWBErrCode			ResCode 		= UWB_Err_Success_0;
	//return ResCode;
	return UWB_Err_Success_0;
}

int API_UQ_Time_Sync(uint8_t* cmd, uint16_t cmdlens, fp_UQ_MSGSend_t callbackfun)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;
	uint8_t*				canbuf			= stSource.bufCANOut;
	uint16_t				sendlens		= 0;
	*canbuf = 0x00; canbuf += 1;
	*canbuf = stSource.stUCIState.u8UWBLocalcIndex;canbuf += 1;
	sendlens = 1;
	callbackfun(UWB_Timer_Sync_RS, stSource.bufCANOut, &sendlens);

	return ResCode;
}



#else
?
#endif

void API_UQ_Set_NTF_Cache_Flag(ST_UWBSource* pst_uwb_source)
{
	pst_uwb_source->stUCIState.bIsNTFCache = true;
}


#ifdef _DEBUG
int DEBUG_MSG_SEND(E_UWBControlMessageIndex msgtype, uint8_t* pcmd, uint16_t* pcmdlens)
{
#if defined(UWB_Responder)
	u8 tmpBuf[65];
	core_mm_set(tmpBuf,0x00,65);

	switch (msgtype)
	{
	case UWB_Anchor_WakeUp_RS:
		tmpBuf[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
		break;
	case UWB_Timer_Sync_RS:
		tmpBuf[0] = CAN_PKG_ID_TIME_SYNC_RS;
		break;
	case UWB_Ranging_Session_Start_RS:
		tmpBuf[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
		break;
	case UWB_Ranging_Result_Notice:

#if defined(UWB_BLE_ANCHOR_AND_SA_CONTROLL)// only 1 BLE + 4Anchor + SA Control mode
		core_mm_copy(tmpBuf,pcmd,*pcmdlens);
		BCanPdu_Set_BLE133_Data(tmpBuf,64U);
		return 0U;
#else
		tmpBuf[0] = CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE;
		break;
#endif

	default:
		break;
	}
	core_mm_copy(tmpBuf+1,pcmd,*pcmdlens);
	ble_ccc_send_evt(CCC_EVT_RECV_UWB_DATA,0U,tmpBuf,65U);
#endif
	return 0;
}


int DEBUG_CONFIG_W(uint8_t CfgIdx, uint8_t* pBuf, const uint16_t BufLens)
{
	return 0;
}

int DEBUG_CONFIG_R(uint8_t CfgIdx, uint8_t* pBuf, uint16_t* pMaxBufLens)
{
	return 0;
}






















































#endif


int UQ_UWB_SDK_Interface_init(ST_UWBSDKInterface* pstSDK)
{
	E_UWBErrCode	ResCode 		= UWB_Err_Success_0;
	pstSDK->fpUQDeviceInit			= (fp_UQ_Device_Init_t)			(&API_UQ_Device_Init);
	pstSDK->fpUQDeviceReset			= (fp_UQ_Device_Reset_t)		(&API_UQ_Device_Reset);
	pstSDK->fpUQGetCaps				= (fp_UQ_Get_Caps_t)			(&API_UQ_Get_Caps);
	pstSDK->fpUQSetCaps				= (fp_UQ_Set_Caps_t)			(&API_UQ_Set_Caps);
	pstSDK->fpUQAnchorWakup			= (fp_UQ_Anchor_Wakup_t)		(&API_UQ_Anchor_Wakup);
	pstSDK->fpUQTimeSync			= (fp_UQ_Time_Sync_t)			(&API_UQ_Time_Sync);
	pstSDK->fpUQRangingSessionSetup	= (fp_UQ_RangingSessionSetup_t)	(&API_UQ_Ranging_Session_Setup);
	pstSDK->fpUQRangingCtrl			= (fp_UQ_RangingCtrl_t)			(&API_UQ_Ranging_Ctrl);
	pstSDK->fpUQCustCMD				= (fp_UQ_CustCMD_t)				(&API_UQ_CustCMD);
	pstSDK->fpUQRangingResult		= (fp_UQ_RangingResult_t)		(&API_UQ_Ranging_Result);
	pstSDK->fpUQRangingNTFCache 	= (fp_UQ_RangingNTFCache_t)		(&API_UQ_Ranging_NTF_Cache);
	pstSDK->fpUQSetNTFCacheFlag		= (fp_UQ_SetNTFCacheFlag_t)		(&API_UQ_Set_NTF_Cache_Flag);
#ifdef _DEBUG
	pstSDK->fpUQSendMSG	= (fp_UQ_MSGSend_t)			(&DEBUG_MSG_SEND);
	pstSDK->fpUQConfigW	= (fp_UQ_ConfigWrite_t)		(&DEBUG_CONFIG_W);
	pstSDK->fpUQConfigR	= (fp_UQ_ConfigRead_t)		(&DEBUG_CONFIG_R);
#endif

	return ResCode;
}



#if 0 
abandoned code

#if defined(UWB_INITIATOR)

#elif defined(UWB_RESPONDER)
int API_UQ_Device_Init(void)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}
int API_UQ_Device_Reset(void)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}
int API_UQ_Get_Caps(st_uwb_protocol* pst_ptl)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}
int API_UQ_Set_Caps(st_uwb_protocol* pst_ptl)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}
int API_UQ_Anchor_Wakup(uint8_t* cmd, uint16_t cmdlens)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}
int API_UQ_Time_Sync(uint8_t* cmd, uint16_t cmdlens)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}
int API_UQ_RangingSessionSetup(uint8_t* cmd, uint16_t cmdlens)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}
int API_UQ_RangingCtrl(uint8_t* cmd, uint16_t cmdlens)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}

int API_UQ_CustCMD(st_custcmd* pstCustCmd)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;

	return ResCode;

}
#else
?
#endif

#endif

