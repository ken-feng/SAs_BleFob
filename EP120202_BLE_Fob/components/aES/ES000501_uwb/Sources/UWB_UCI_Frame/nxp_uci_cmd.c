/*
 * nxp_uci_cmd.c
 *
 *  Created on: 2022年7月12日
 *      Author: JohnSong
 */
#include "../uwb_common_def.h"
#include "../UWB_Drivers/uwb_timer.h"
#include "./nxp_uci_cmd.h"
#include "./nxp_uci_cmd_param_table.h"


static uint8_t uwb_uci_send_buffer[UCISW_MAX_FRAME_LENGTH] = {0};
static uint8_t uwb_uci_recv_buffer[UCISW_MAX_FRAME_LENGTH] = {0};
static void nxp_uci_res_parse(ST_NXPUCIDat* pst_recv, ST_UCIFrameState* pst_state);
static void nxp_uci_cmd_parse(ST_NXPUCIDat* pst_send, ST_UCIFrameState* pst_state);
/****
 * \brief	init uci frame
 *
 */
//UWBErrCode API_uci_frame_init(st_uci_frame_state *pstate)
E_UWBErrCode API_UCI_Frame_Init(ST_UCIFrameState* pst_frame_state)
{
	E_UWBErrCode eResCode 		= UWB_Err_Success_0;

	core_mm_set((uint8_t*)&pst_frame_state->stUCISend, 0, sizeof(ST_NXPUCIDat));
	core_mm_set((uint8_t*)&pst_frame_state->stUCIRecv, 0, sizeof(ST_NXPUCIDat));

	pst_frame_state->stUCISend.bIsTheLast 		= true;
	pst_frame_state->stUCISend.u8Tot 			= 0;
	pst_frame_state->stUCISend.pbufDatHeader 	= uwb_uci_send_buffer;
	pst_frame_state->stUCISend.pbufDatBody 		= uwb_uci_send_buffer + UCISW_HEADER_LENGTH - 1; //CMD Header are 4 bytes RecvHeader are 5 bytes .

	pst_frame_state->stUCIRecv.bIsTheLast		= true;
	pst_frame_state->stUCIRecv.u8Tot			= 0;
	pst_frame_state->stUCIRecv.pbufDatHeader	= uwb_uci_recv_buffer;
	pst_frame_state->stUCIRecv.pbufDatBody		= uwb_uci_recv_buffer + UCISW_HEADER_LENGTH;

	core_mm_set(uwb_uci_send_buffer, 0, UCISW_MAX_FRAME_LENGTH);
	core_mm_set(uwb_uci_recv_buffer, 0, UCISW_MAX_FRAME_LENGTH);

	//initialize the API with precise timer .
	//timer id1 -> precise delay used .
	//timer id2 -> up to you .
	//timer id3 -> UWB module communication run time out used.
	//ResCode = API_UWB_timer_tools_init((st_timer_tool_t*)&st_timer_tool);
	//ResCode = API_UWB_timer_tools_init(pstTimerTools);
	eResCode = API_UWB_Timer_Tools_Init(&pst_frame_state->stTimerTools);

	if(UWB_Err_Success_0 != eResCode)
		return eResCode;

	//initialize the API with UWB module communication control .
	//ResCode = API_UWB_commu_module_init((st_uwb_commu_attr_t*)&st_uwb_commu_attr);
	//ResCode = API_UWB_commu_module_init(pstUWBCommu);
	eResCode = API_UWB_Commu_Module_Init(&pst_frame_state->stUWBCommu);
	if(UWB_Err_Success_0 != eResCode)
		return eResCode;

	//first call initialize uci frame
	pst_frame_state->eState = UciFrameState_Init;
	pst_frame_state->eWorkStat = 0;//added by niull
	pst_frame_state->stSession.bIsUCISessionInitDone = false;

	return eResCode;
}


E_UWBErrCode API_UCI_Frame_Entry(ST_UCIFrameState* pst_frame_state)
{
	E_UWBErrCode ResCode = UWB_Err_Success_0;

	switch(pst_frame_state->eState)
	{
		case UciFrameState_Init:
														ResCode = pst_frame_state->stUWBCommu.fpCmmuInit();
														if (UWB_Err_Success_0 == ResCode)
														{
															pst_frame_state->eState = UciFrameState_Connecting;
														}
														else
														{
															pst_frame_state->eState = UciFrameState_NotifyPending;
														}
														break;
		case UciFrameState_Connecting:
														ResCode = pst_frame_state->stUWBCommu.fpCmmuReset(
															&pst_frame_state->stTimerTools, pst_frame_state->stUCIRecv.pbufDatHeader, &pst_frame_state->stUCIRecv.u16DatBodyLens);
														if(UWB_Err_Success_0 == ResCode)
														{
															nxp_uci_res_parse(&pst_frame_state->stUCIRecv, pst_frame_state);
															switch(pst_frame_state->eOpCode)
															{
																case UWB_Err_UCI_Device_Status_NTF_RangingSession_Is_Ready:
																case UWB_Err_UCI_Device_Status_NTF_Device_Is_Test_Mode:
																case UWB_Err_UCI_Device_Status_NTF_Reboot_By_SoftWare:
																case UWB_Err_UCI_Device_Status_NTF_Reboot_From_HPD:
																case UWB_Err_UCI_Device_Status_NTF_Reboot_From_RST_Pin:
																case UWB_Err_UCI_Device_Status_NTF_Reboot_Power_Fault:
																	pst_frame_state->eState				= UciFrameState_Connected;
																	pst_frame_state->eNTFIdx			= UBW_Deviec_Status_NTF;
																	pst_frame_state->eMSGIdx			= UWB_MSG_Is_Null;
																	pst_frame_state->bIsInCmd			= false;
																	pst_frame_state->bIsUWBConnected	= true;
																	
																	//LOG_L_S(CCC_MD,"UWB Module is On-line .");
																	break;
																default:
																	pst_frame_state->eState = UciFrameState_NotConnected;
																	//LOG_L_S(CCC_MD,"UWB Module Reset Failed . UWB Module is Off-line .");
																break;
															}
														}
														else
														{
															pst_frame_state->eState = UciFrameState_NotConnected;
														}
														break;
		case UciFrameState_Connected:
														if(pst_frame_state->bIsInCmd)
														{
															if(pst_frame_state->stUWBCommu.fpCmmuIsntfcomin())
															{
																pst_frame_state->eState = UciFrameState_ReceiveNotify;
																ResCode = UWB_Err_COMMU_Send_Cancel;
															}
															else
															{
																pst_frame_state->eState = UciFrameState_TransmitCommand;
															}
														}
														else
														{
															// never in to thie here .
														}
														break;
		case UciFrameState_NotConnected:
														pst_frame_state->bIsUWBConnected = false;
														//Only the HardRest command can be accept
														API_UWB_Commu_Module_Deinit(&pst_frame_state->stUWBCommu);
														API_UWB_Timer_Tools_Deinit(&pst_frame_state->stTimerTools);
														pst_frame_state->eState = UciFrameState_NotifyPending;
														break;

		case UciFrameState_TransmitCommand:
														if((pst_frame_state->bIsInCmd)&&(UWB_MSG_Is_Null != pst_frame_state->eMSGIdx))
														{
															nxp_uci_cmd_parse(&pst_frame_state->stUCISend, pst_frame_state);
															if(UWB_Err_Success_0 != pst_frame_state->eOpCode)
															{
																pst_frame_state->eNTFIdx = UWB_MSG_Is_Null;
																pst_frame_state->eState = UciFrameState_Connected;
															}
															else
															{
																ResCode = pst_frame_state->stUWBCommu.fpCmmuSend(
																		&pst_frame_state->stTimerTools, pst_frame_state->stUCISend.pbufDatHeader, pst_frame_state->stUCISend.u16DatBodyLens);
																if(UWB_Err_Success_0 == ResCode)
																{
																	pst_frame_state->eMSGIdx = UWB_MSG_Is_Null;
																	pst_frame_state->eState = UciFrameState_ReceiveResponse;
																	pst_frame_state->stUCISend.u16DatBodyLens = 0;

																}
																else if(UWB_Err_COMMU_Send_Cancel == ResCode)
																{
																	pst_frame_state->eState = UciFrameState_ReceiveNotify;
																}
																else
																{
																	pst_frame_state->eState = UciFrameState_NotifyPending;
																}
															}
														}
														else
														{
															pst_frame_state->eState = UciFrameState_Connected;//goto check if 29d5 msg are coming .
														}
														break;
		case UciFrameState_ReceiveResponse:
														if(pst_frame_state->bIsInCmd)
														{
															ResCode = pst_frame_state->stUWBCommu.fpCmmuRecv(
																&pst_frame_state->stTimerTools, pst_frame_state->stUCIRecv.pbufDatHeader, &pst_frame_state->stUCIRecv.u16DatBodyLens);
															if(UWB_Err_Success_0 == ResCode)
															{
																nxp_uci_res_parse(&pst_frame_state->stUCIRecv, pst_frame_state);

																if(UWB_MSG_Is_Null != pst_frame_state->eMSGIdx)
																{
																	//pst_frame_state->stTimerTools.fpOSDelay(1);
																	pst_frame_state->stTimerTools.fpDelay(NS_PERIOD_BY_MS(1));
																	if(false == pst_frame_state->stUWBCommu.fpCmmuIsntfcomin())
																	{
																		pst_frame_state->bIsInCmd = false;
																		pst_frame_state->eState = UciFrameState_Connected;
																	}
																	else
																	{
																		pst_frame_state->bIsInCmd = true;
																		pst_frame_state->eState = UciFrameState_ReceiveNotify;
																		ResCode = UWB_Err_COMMU_Send_Cancel;
																	}
																}
															}
															else
															{
																pst_frame_state->eState = UciFrameState_NotifyPending;
															}
														}
														else
														{
															//do nothing .
														}
														break;
		case UciFrameState_ReceiveNotify:
														if(pst_frame_state->bIsInCmd)
														{
															ResCode = pst_frame_state->stUWBCommu.fpCmmuRecv(
																&pst_frame_state->stTimerTools, pst_frame_state->stUCIRecv.pbufDatHeader, &pst_frame_state->stUCIRecv.u16DatBodyLens);
															if(UWB_Err_Success_0 == ResCode)
															{
																nxp_uci_res_parse(&pst_frame_state->stUCIRecv, pst_frame_state);
																//pst_frame_state->stTimerTools.fpOSDelay(1);//need change to  use the os delay
																pst_frame_state->stTimerTools.fpDelay(NS_PERIOD_BY_MS(1));
																if(false == pst_frame_state->stUWBCommu.fpCmmuIsntfcomin())
																{
																	pst_frame_state->bIsInCmd = false;
																	pst_frame_state->eState = UciFrameState_Connected;
																}
																else
																{
																	pst_frame_state->bIsInCmd = true;
																	pst_frame_state->eState = UciFrameState_ReceiveNotify;
																	ResCode = UWB_Err_COMMU_Send_Cancel;
																}
															}
															else
															{
																pst_frame_state->eState = UciFrameState_NotifyPending;
															}
														}
														else
														{
															//do nothing .
														}
														break;
		case UciFrameState_NotifyPending:
														if(pst_frame_state->bIsUWBConnected)
														{
															pst_frame_state->eState = UciFrameState_Connected;
//															if((UWB_MSG_Is_Null != pstate->MSGIdx) &&
//															   (UWB_Sys_Notice_NTF != pstate->NTFIdx))
//															{
//																pst_frame_state->eState = UciFrameState_Connected;
//																pstate->IsInCmd = false;
//																pstate->MSGIdx = pstate->MSGIdx+1;//from RQ goto RS
//																break;
//															}
//															else
//															{
//																pst_frame_state->eState = UciFrameState_Connected;
//																//pstate->IsInCmd = false; do not going to next round . keep state at current command while for .
//																pstate->NTFIdx = UWB_Sys_Notice_NTF;//Unknown err
//															}
														}
														else
														{//keep the Hardrest continue .
															if(5 >= pst_frame_state->u8RSTCnt)
															{
																API_UCI_Frame_Init(pst_frame_state);
																pst_frame_state->eState = UciFrameState_Init;
																pst_frame_state->bIsInCmd = true;
																pst_frame_state->u8RSTCnt = pst_frame_state->u8RSTCnt + 1UL;
															}
															else
															{
																pst_frame_state->bIsInCmd = false;
															}

														}
														break;

		case UciFrameState_Error:
		default:
														ResCode = UWB_Err_UnKnowErr;
														pst_frame_state->bIsInCmd = false;
														pst_frame_state->eNTFIdx = UWB_Sys_Notice_NTF;
														break;
	}

	return ResCode;
}


/*
*
*/
static E_UWBErrCode nxp_uci_ccc_cmd_package(uint32_t cmd, ST_NXPUCIDat* pst_send, ST_UCIFrameState* pst_state)
{//此处有速度优化空间，应改用静态数组存放，枚举索引直接取值。但不利于调试目视阅读。后期调完再优化


	//ST_NXPUCIMsg* pstMsg = (ST_NXPUCIMsg*)pst_send->pbufDatHeader;
	//ST_NXPUCIMsg stMsg;
	E_UWBErrCode ResCode = UWB_Err_Success_0;

	//core_mm_set(&stMsg, 0, sizeof(ST_NXPUCIMsg));

	if(		(CORE_CMD_DEVICE_RESET == cmd)||\
		(CORE_CMD_GET_DEVICE_INFO == cmd)||\
		(CORE_CMD_GET_CAPS_INFO == cmd)||\
		(CORE_CMD_SET_CONFIG == cmd)||\
		(CORE_CMD_GET_CONFIG == cmd))
	{
		pst_send->pbufDatHeader[0] = (uint8_t)NXP_UCI_MSG_HEADER_OCT0_GID_UCI_CORE_GROUP(NXP_UCI_MSG_MIT_CTRL_CMD);
		switch(cmd)
		{
			case CORE_CMD_DEVICE_RESET:		pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_DEVICE_RESET;
											pst_send->pbufDatBody[0] = 0x00;
											pst_send->u16DatBodyLens = 1;
											break;
			case CORE_CMD_GET_DEVICE_INFO:  pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_GET_DEVICE_INFO;
											pst_send->u16DatBodyLens = 0;
											break;
			case CORE_CMD_GET_CAPS_INFO:	pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_GET_CAPS_INFO;
											pst_send->u16DatBodyLens = 0;
											break;
			case CORE_CMD_SET_CONFIG:		pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_SET_CONFIG;
											//pst_send->u16DatBodyLens = 0;
											break;
			case CORE_CMD_GET_CONFIG:		pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_GET_CONFIG;
											pst_send->u16DatBodyLens = 0;
											break;
			default:						pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_CORE_CMD_RFU;
											pst_send->u16DatBodyLens = 0;
											ResCode = UWB_Err_CMD_Undefine;
											break;
		}
	}
	else if((SESSION_CONFIG_CMD_INIT == cmd)||\
		(SESSION_CONFIG_CMD_DEINIT == cmd)||\
		(SESSION_CONFIG_CMD_SET_APP_CONFIG == cmd)||\
		(SESSION_CONFIG_CMD_GET_APP_CONFIG == cmd)||\
		(SESSION_CONFIG_CMD_GET_COUNT == cmd)||\
		(SESSION_CONFIG_CMD_GET_STATE == cmd)||\
		(SESSION_CONFIG_CMD_GET_POSSIBLE_RAN_MULTIPLIER_VALUE == cmd))
	{
		//SESSION CONFIG GROUP
		//stMsg.unioOCT0.u8BytePart = NXP_UCI_MSG_HEADER_OCT0_GID_UWB_SESS_CONFIG_GROUP(NXP_UCI_MSG_MIT_CTRL_CMD);
		//pst_send->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_state->stSession.u32CurrSessionID));
		//pst_send->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_state->stSession.u32CurrSessionID));
		//pst_send->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_state->stSession.u32CurrSessionID));
		//pst_send->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_state->stSession.u32CurrSessionID));
		pst_send->pbufDatHeader[0] = NXP_UCI_MSG_HEADER_OCT0_GID_UWB_SESS_CONFIG_GROUP(NXP_UCI_MSG_MIT_CTRL_CMD);
		switch(cmd)
		{
			case SESSION_CONFIG_CMD_INIT:			pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_INIT;
													//pst_send->pbufDatBody[4] = pst_state->stSession.u8UCIRangSessionMode;
													break;
			case SESSION_CONFIG_CMD_DEINIT:			pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_DEINIT;
													break;
			case SESSION_CONFIG_CMD_SET_APP_CONFIG:	pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_SET_APP_CONFIG;
													break;
			case SESSION_CONFIG_CMD_GET_APP_CONFIG:	pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_GET_APP_CONFIG;
													pst_send->pbufDatBody[4] = 0;
													break;//Get All session APP configure .
			case SESSION_CONFIG_CMD_GET_COUNT:		pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_GET_COUNT;
													break;
			case SESSION_CONFIG_CMD_GET_STATE:		pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_GET_STATE;
													break;
			case SESSION_CONFIG_CMD_GET_POSSIBLE_RAN_MULTIPLIER_VALUE:			
													pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_SESSION_CONFIG_CMD_GET_POSSIBLE_RAN_MULTIPLIER_VALUE;
													break;
			default:
				ResCode = UWB_Err_CMD_Undefine;
				break;
		}
	}
	else if((RANGE_SESSION_CMD_START == cmd)||\
		(RANGE_SESSION_CMD_STOP == cmd)||\
		(RANGE_SESSION_CMD_GET_RANGING_COUNT == cmd)||\
		(RANGE_SESSION_CMD_RESUME == cmd))
	{
		//SESSION CTRL GROUP
		//pstMsg->unioOCT0.u8BytePart = NXP_UCI_MSG_HEADER_OCT0_GID_UWB_RANGING_SESSION_CTRL_GROUP(NXP_UCI_MSG_MIT_CTRL_CMD);
		pst_send->pbufDatHeader[0] = NXP_UCI_MSG_HEADER_OCT0_GID_UWB_RANGING_SESSION_CTRL_GROUP(NXP_UCI_MSG_MIT_CTRL_CMD);
		switch(cmd)
		{
		case RANGE_SESSION_CMD_START:
			pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_START;
			break;
		case RANGE_SESSION_CMD_STOP:
			pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_STOP;
			pst_send->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_state->stSession.u32CurrSessionID));
			break;
		case RANGE_SESSION_CMD_GET_RANGING_COUNT:
			pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_GET_RANGING_COUNT;
			pst_send->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_state->stSession.u32CurrSessionID));
			break;
		case RANGE_SESSION_CMD_RESUME:
			pst_send->pbufDatHeader[1] = NXP_UCI_MSG_HEADER_OCT1_RANGE_SESSION_CMD_RESUME;
			pst_send->pbufDatBody[3] = core_dcm_u16_hi(core_dcm_u32_hi(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[2] = core_dcm_u16_lo(core_dcm_u32_hi(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[1] = core_dcm_u16_hi(core_dcm_u32_lo(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[0] = core_dcm_u16_lo(core_dcm_u32_lo(pst_state->stSession.u32CurrSessionID));
			pst_send->pbufDatBody[7] = core_dcm_u16_hi(core_dcm_u32_hi(pst_state->stSession.u32STSIndex));
			pst_send->pbufDatBody[6] = core_dcm_u16_lo(core_dcm_u32_hi(pst_state->stSession.u32STSIndex));
			pst_send->pbufDatBody[5] = core_dcm_u16_hi(core_dcm_u32_lo(pst_state->stSession.u32STSIndex));
			pst_send->pbufDatBody[4] = core_dcm_u16_lo(core_dcm_u32_lo(pst_state->stSession.u32STSIndex));
			break;
		default:
			ResCode = UWB_Err_CMD_Undefine;
			break;
		}
	}

	if(UWB_Err_Success_0 != ResCode)
	{
		pst_send->u16DatBodyLens = 0;
		return ResCode;
	}

	if(pst_send->u16DatBodyLens < 0xFF)
	{
		pst_send->pbufDatHeader[2] = 0;
		pst_send->pbufDatHeader[3] = (uint8_t)(pst_send->u16DatBodyLens&0x00FF);
	}
	else
	{
		//msg.OCT1.BitPart.rfu1 = 1;
		pst_send->pbufDatHeader[1] |= 0x80;
		pst_send->pbufDatHeader[3] = (uint8_t)core_dcm_u16_hi(pst_send->u16DatBodyLens);
		pst_send->pbufDatHeader[2] = (uint8_t)core_dcm_u16_lo(pst_send->u16DatBodyLens);
	}
	pst_send->u16DatBodyLens += 4;
	return ResCode;
}
//uint8_t SessionSetupCANBuffer[64] = {0};
//Control status, assemble instructions, send instructions
//
static void nxp_uci_cmd_parse(ST_NXPUCIDat* pst_send, ST_UCIFrameState* pst_state)
{
	pst_state->eOpCode =  UWB_Err_Success_0;

	if(Module_Stat_Ready == pst_state->eWorkStat)
	{
		switch(pst_state->eMSGIdx)
		{
		case UWB_Anchor_WakeUp_RQ: 								nxp_uci_ccc_cmd_package(CORE_CMD_GET_CAPS_INFO, pst_send, pst_state);break;
		case UWB_Timer_Sync_RQ: 								break;//没做
		case UWB_Ranging_Session_Init_RQ:
			if(false == pst_state->stSession.bIsUCISessionInitDone)
			{
				pst_state->eOpCode = nxp_uci_ccc_cmd_package(SESSION_CONFIG_CMD_INIT, pst_send, pst_state);
			}
			else
			{
				// Never into this here .
			}
			break;
		case UWB_Hard_Reset_RQ:					 		nxp_uci_ccc_cmd_package(CORE_CMD_DEVICE_RESET, pst_send, pst_state);break;
		case UWB_Core_Config_Setting_RQ:				nxp_uci_ccc_cmd_package(CORE_CMD_SET_CONFIG, pst_send, pst_state);break;
		case UWB_Core_Get_Device_Info_RQ:				nxp_uci_ccc_cmd_package(CORE_CMD_GET_DEVICE_INFO, pst_send, pst_state);break;

		default:
			pst_state->eOpCode = UWB_Err_Incorrect_CMD_With_Status;
			break;
		}
	}
	else//Module_Stat_Active
	{
		switch(pst_state->stSession.eSesionStat)
		{
		case Session_Stat_Init:
			if(UWB_Anchor_WakeUp_RQ == pst_state->eMSGIdx)
			{
				pst_state->eOpCode = nxp_uci_ccc_cmd_package(CORE_CMD_GET_CAPS_INFO, pst_send, pst_state);
			}
			else if(UWB_Ranging_Session_App_Config_RQ == pst_state->eMSGIdx)
			{
				if(false == pst_state->stSession.bIsUCISessionAPPConfigDone)
				{
					pst_state->eOpCode = nxp_uci_ccc_cmd_package(SESSION_CONFIG_CMD_SET_APP_CONFIG, pst_send, pst_state);
				}
				else
				{

				}
			}
			else if(UWB_Ranging_Session_Deinit_RQ == pst_state->eMSGIdx)
			{
				pst_state->eOpCode = nxp_uci_ccc_cmd_package(SESSION_CONFIG_CMD_DEINIT, pst_send, pst_state);
			}
			else
			{

			}
			break;
		case Session_Stat_Idle:
			switch(pst_state->eMSGIdx)
			{
			case UWB_Anchor_WakeUp_RQ: 				pst_state->eOpCode = nxp_uci_ccc_cmd_package(CORE_CMD_GET_CAPS_INFO, pst_send, pst_state);break;
			case UWB_Ranging_Session_Start_RQ:		pst_state->eOpCode = nxp_uci_ccc_cmd_package(RANGE_SESSION_CMD_START, pst_send, pst_state);break;
			case UWB_Ranging_Session_Recover_RQ:	pst_state->eOpCode = nxp_uci_ccc_cmd_package(RANGE_SESSION_CMD_RESUME, pst_send, pst_state);break;
			case UWB_Ranging_Session_Stop_RQ:		pst_state->eOpCode = nxp_uci_ccc_cmd_package(RANGE_SESSION_CMD_STOP, pst_send, pst_state);break;
			case UWB_Ranging_Session_Deinit_RQ:		pst_state->eOpCode = nxp_uci_ccc_cmd_package(SESSION_CONFIG_CMD_DEINIT, pst_send, pst_state);break;
			default:
				break;
			}
			break;
		case Session_Stat_Aciv:
			if(UWB_Anchor_WakeUp_RQ == pst_state->eMSGIdx)
			{
				pst_state->eOpCode = nxp_uci_ccc_cmd_package(CORE_CMD_GET_CAPS_INFO, pst_send, pst_state);
			}
			else if(UWB_Ranging_Session_Stop_RQ == pst_state->eMSGIdx)
			{
				pst_state->eOpCode = nxp_uci_ccc_cmd_package(RANGE_SESSION_CMD_STOP, pst_send, pst_state);
			}
			else if(UWB_Ranging_Session_Deinit_RQ == pst_state->eMSGIdx)
			{
				pst_state->eOpCode = nxp_uci_ccc_cmd_package(SESSION_CONFIG_CMD_DEINIT, pst_send, pst_state);
			}
			else
			{

			}
			break;
		case Session_Stat_Deinit:
			if(UWB_Anchor_WakeUp_RQ == pst_state->eMSGIdx)
			{
				pst_state->eOpCode = nxp_uci_ccc_cmd_package(CORE_CMD_GET_CAPS_INFO, pst_send, pst_state);
			}
			else if(UWB_Ranging_Session_GetSeionCnt_RQ == pst_state->eMSGIdx)
			{
				pst_state->eOpCode = nxp_uci_ccc_cmd_package(SESSION_CONFIG_CMD_GET_COUNT, pst_send, pst_state);
			}
			else
			{
				//do nothing .
			}
			break;

		default:
			pst_state->eOpCode = UWB_Err_Incorrect_CMD_With_Status;
			break;

		}
	}
}

static void nxp_uci_ccc_res_analyze(ST_NXPUCIDat* pst_recv, ST_UCIFrameState* pst_state)
{
	ST_NXPUCIMsg			stMsg;
	const uint8_t 			nxp_uci_stat_success	= 0;

	core_mm_set((uint8_t*)&stMsg, 0, sizeof(ST_NXPUCIMsg));

	//uci_data[0] is padding 0x00
	stMsg.u8PaddingByte 				= pst_recv->pbufDatHeader[0];
	stMsg.unioOCT0.u8BytePart 			= pst_recv->pbufDatHeader[1];
	stMsg.unioOCT1.BytePart 			= pst_recv->pbufDatHeader[2];
	stMsg.u8OCT2 						= pst_recv->pbufDatHeader[3];
	stMsg.u8OCT3 						= pst_recv->pbufDatHeader[4];
	if(0 == (uint8_t)stMsg.unioOCT1.stBitPart.rfu1)
	{
		stMsg.u16PaylaodLens				= (uint16_t)stMsg.u8OCT3;
	}
	else
	{
		stMsg.u16PaylaodLens				= (uint16_t)core_dcm_mku16(stMsg.u8OCT2, stMsg.u8OCT3);
	}
	stMsg.pbufPaylaod 				= pst_recv->pbufDatBody;

	/* ********************
	* Type 2:
	* Response
	* Notification
	*
	* Group 3:
	* CORE_GROUP
	* SESS_CONFIG_GROUP
	* SESSION_CTRL_GROUP
	*
	* add by John song
	*/
	//if(NXP_UCI_MSG_MIT_CTRL_RES == stMsg.OCT0.BitPart.mt)
	if(NXP_UCI_MSG_MIT_CTRL_RES == pst_recv->pbufDatHeader[1]>>5)
	{
		//switch(stMsg.OCT0.BitPart.gid)
		switch(pst_recv->pbufDatHeader[1]&0x0F)
		{
		case NXP_UCI_MSG_GID_UCI_CORE_GROUP :
			//switch(stMsg.OCT1.BitPart.oid)
			switch(pst_recv->pbufDatHeader[2]&0x3F)
			{
			case NXP_UCI_MSG_OID_CORE_DEVICE_RESET_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					pst_state->bIsInCmd = false;
					pst_state->eMSGIdx = UWB_Hard_Reset_RS;
				}
				else
				{
					pst_state->eOpCode = UWB_Err_UCI_Rest_Failed;
					pst_state->bIsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_CORE_GET_DEVICE_INFO_RES:						//没做
				pst_state->bIsInCmd = false;
				pst_state->eMSGIdx = UWB_Core_Get_Device_Info_RS;
				break;
			case NXP_UCI_MSG_OID_CORE_GET_CAPS_INFO_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					uint8_t* t 			= stMsg.pbufPaylaod + 2;
					ST_UWBCaps* pstCaps = &pst_state->stProtocol.stCCCCaps;
					for(uint8_t i=0,tlvCnt = stMsg.pbufPaylaod[1]; i < tlvCnt; i++)
					{
						switch(*t)
						{
						case 0xA0:	pstCaps->u8SlotBitMask = *(t + 2);
									break;
						case 0xA1:	pstCaps->u32SyncCodeBitMask = core_dcm_mku32(t[5], t[4], t[3], t[2]);
									break;
						case 0xA2:	pstCaps->u8HopingConfigBitMask = *(t + 2);
									break;
						case 0xA3:	pstCaps->u8ChannelBitMaskMap = *(t + 2);
									break;
						case 0xA4:	
									if (*(t + 1) >sizeof(pstCaps->bufProtocolVersion))
									{
										pstCaps->u8CCCProtocolVersionLens = sizeof(pstCaps->bufProtocolVersion);
									}
									else
									{
										pstCaps->u8CCCProtocolVersionLens = *(t + 1);
									}
									core_mm_copy(pstCaps->bufProtocolVersion, t + 2, pstCaps->u8CCCProtocolVersionLens);
									core_array_reversed_u8(pstCaps->bufProtocolVersion, pstCaps->u8CCCProtocolVersionLens);

									break;
						case 0xA5: 
									if (*(t + 1) > sizeof(pstCaps->bufCCCConfigID ))
									{
										pstCaps->u8CCCConfigIDLens = sizeof(pstCaps->bufCCCConfigID);
									}
									else
									{
										pstCaps->u8CCCConfigIDLens = *(t + 1);
									}
									core_mm_copy(pstCaps->bufCCCConfigID, t + 2, pstCaps->u8CCCConfigIDLens);
									core_array_reversed_u8(pstCaps->bufCCCConfigID, pstCaps->u8CCCConfigIDLens);

									break;
						case 0xA6: 

									if (*(t + 1) > sizeof(pstCaps->bufCCCShapeCombo))
									{
										pstCaps->u8PulseShapeCombolens = sizeof(pstCaps->bufCCCShapeCombo);
									}
									else
									{
										pstCaps->u8PulseShapeCombolens = *(t + 1);
									}
									core_mm_copy(pstCaps->bufCCCShapeCombo, t + 2, pstCaps->u8PulseShapeCombolens);
									break;
						default: //29d5 return more parameter analyze failed ， need more document to description this part add by JohnSong
							break;
						}
						t = t+(*(t+1)+2);
					}
					pst_state->eOpCode = UWB_Err_Success_0;
				}
				else
				{
					pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
				}
				pst_state->bIsInCmd = false;
				pst_state->eMSGIdx = UWB_Anchor_WakeUp_RS;
				break;
			case NXP_UCI_MSG_OID_CORE_SET_CONFIG_RES:
				pst_state->eMSGIdx = UWB_Core_Config_Setting_RS;
				pst_state->bIsInCmd = false;
				break;

			case NXP_UCI_MSG_OID_CORE_GET_CONFIG_RES:						break;//没做
			default:
				pst_state->eOpCode = UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect;
				pst_state->bIsInCmd = false;
				break;
			}
			break;//CORE_GROUP END

			//SESS_CONFIG_GROUP
		case NXP_UCI_MSG_GID_UWB_SESS_CONFIG_GROUP :
			//switch(stMsg.OCT1.BitPart.oid)
			switch(pst_recv->pbufDatHeader[2]&0x3F)
			{
			case NXP_UCI_MSG_OID_SESSION_INIT_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					pst_state->stSession.bIsUCISessionInitDone = true;
					pst_state->bIsInCmd = false;
					pst_state->eMSGIdx = UWB_Ranging_Session_Init_RS;
					pst_state->eWorkStat = Module_Stat_Active;
					pst_state->stSession.eSesionStat = Session_Stat_Init;
				}
				else
				{
					pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
					pst_state->bIsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_DEINIT_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					pst_state->stSession.bIsUCISessionInitDone = false;
					pst_state->stSession.bIsUCISessionAPPConfigDone = false;
					pst_state->stSession.eSesionStat = Session_Stat_Deinit;
					pst_state->eWorkStat = Module_Stat_Ready;
					pst_state->stSession.u32CurrSessionID = 0;
					pst_state->bIsInCmd = false;
				}
				else
				{
					pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
					pst_state->bIsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_SET_APP_CONFIG_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{//????????8怎么处理？
					pst_state->stSession.bIsUCISessionAPPConfigDone = true;
					pst_state->stSession.eSesionStat = Session_Stat_Idle;
					pst_state->eMSGIdx = UWB_Ranging_Session_App_Config_RS;
					pst_state->bIsInCmd = false;
				}
				else
				{
					pst_state->stSession.bIsUCISessionAPPConfigDone = false;
					pst_state->stSession.eSesionStat = Session_Stat_Init;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_GET_APP_CONFIG_RES:break;
			case NXP_UCI_MSG_OID_SESSION_GET_COUNT_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					pst_state->u8UWBSessionCnt = stMsg.pbufPaylaod[1];
					if(0 == pst_state->u8UWBSessionCnt)
					{
						//when the current session count is ZERO , must be gouing to Ready mode .
						pst_state->eWorkStat = Module_Stat_Ready;
					}
					else
					{
						//do nothing .
					}
				}
				else
				{
					pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
					pst_state->bIsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_GET_STATE_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					if(Session_Stat_RFU <= stMsg.pbufPaylaod[1])
					{
						pst_state->stSession.eSesionStat = Session_Stat_RFU;
					}
					else
					{
						pst_state->stSession.eSesionStat = stMsg.pbufPaylaod[1];
					}
				}
				else
				{
					pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
					pst_state->bIsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_GET_POSSIBLE_RAN_MULTIPLIER_VALUE_RES:break;//没做
			default:
				pst_state->eOpCode = UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect;
				pst_state->bIsInCmd = false;
				break;
			}
			break;//SESS_CONFIG_GROUP END

			//SESSION_CTRL_GROUP
		case NXP_UCI_MSG_GID_UWB_RANGING_SESSION_CTRL_GROUP:
			//switch(stMsg.OCT1.BitPart.oid)
			switch(pst_recv->pbufDatHeader[2]&0x3F)
			{
			case NXP_UCI_MSG_OID_RANGE_START_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					pst_state->stSession.eSesionStat = Session_Stat_Aciv;
					pst_state->bIsInCmd = false;
					pst_state->eMSGIdx = UWB_Ranging_Session_Start_RS;
				}
				else
				{
					pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
					pst_state->bIsInCmd = false;
				}

				break;
			case NXP_UCI_MSG_OID_RANGE_STOP_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					pst_state->stSession.eSesionStat = Session_Stat_Idle;
					pst_state->bIsInCmd = true;
					pst_state->eMSGIdx = UWB_Ranging_Session_Deinit_RQ;
				}
				else
				{
					pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
					pst_state->bIsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_RANGE_GET_RANGING_COUNT_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					pst_state->stSession.u32CurrentRangingCnt = core_dcm_mku32(stMsg.pbufPaylaod[4],stMsg.pbufPaylaod[3],stMsg.pbufPaylaod[2],stMsg.pbufPaylaod[1]);
					pst_state->bIsInCmd = false;
					pst_state->eMSGIdx = UWB_Ranging_Session_GetRangCnt_RS;
				}
				break;
			case NXP_UCI_MSG_OID_RANGE_RESUME_RES:
				if(nxp_uci_stat_success == stMsg.pbufPaylaod[0])
				{
					pst_state->stSession.eSesionStat = Session_Stat_Idle;
					pst_state->bIsInCmd = false;
					pst_state->eMSGIdx = UWB_Ranging_Session_Recover_RS;
				}
				else
				{
					pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
					pst_state->bIsInCmd = false;
				}
				break;
			default:
				pst_state->eOpCode = UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect;
				pst_state->bIsInCmd = false;
				break;
			}
			break;//SESSION_CTRL_GROUP END
		case NXP_UCI_MSG_PROPRIETARY_GROUP_0:
			//switch(stMsg.OCT1.BitPart.oid)
			switch(pst_recv->pbufDatHeader[2]&0x3F)
			{
			case NXP_UCI_MSG_OID_PROPRIETARY_RADIO_CONFIG_DOWNLOAD_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_ACTIVATE_SWUP_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_TEST_START_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_TEST_STOP_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_QUERY_UWB_TIMESTAMP_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_DEVICE_SUSPEND_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_SET_TRIM_VALUES_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_GET_ALL_UWB_SESSIONS_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_GET_TRIM_VALUES_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_STORE_PROTECTION_KEY_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_SET_EPOCH_ID_RSP: break;
			default:
				break;
			}
			break;//NXP_UCI_MSG_PROPRIETARY_GROUP_0 END
		default:
			//GID Error
			pst_state->eOpCode = UWB_Err_Session_GID_UnSupported;
			pst_state->bIsInCmd = false;
			break;
			//RESPONSE END
		}
	}
	//else if(NXP_UCI_MSG_MIT_CTRL_NOTIFICATION == stMsg.OCT0.BitPart.mt)
	else if(NXP_UCI_MSG_MIT_CTRL_NOTIFICATION == pst_recv->pbufDatHeader[1]>>5)
	{

		//switch(stMsg.OCT0.BitPart.gid)
		switch(pst_recv->pbufDatHeader[1]&0x0F)
		{
		case NXP_UCI_MSG_GID_UCI_CORE_GROUP :
			if(NXP_UCI_MSG_OID_CORE_DEVICE_STATUS_NTF == stMsg.unioOCT1.stBitPart.oid)
			{
				pst_state->eNTFIdx = UBW_Deviec_Status_NTF;
				//success:006001000101
				//if((nxp_uci_stat_success ==stMsg.payload_lens0)&&(1 == stMsg.payload_lens1)&&(1 == stMsg.paylaod[0]))
				pst_state->eOpCode = UWB_Err_UCI_Device_Status_Ok | stMsg.pbufPaylaod[0];
				//					switch(recv_buff_ptr[5])
				//					{
				//						case 0x01: ResCode = UWB_Err_Success_0;break;
				//						case 0x02: ResCode = UWB_Err_UCI_Device_Status_NTF_RangingSession_Is_Bussy;break;
				//						case 0xFF: ResCode = UWB_Err_UCI_Device_Status_NTF_Status_Error;break;
				//						default:
				//							ResCode = UWB_Err_UCI_Device_Status_NTF_Status_RFU_Error;
				//							break;
				//					}
				//					if(1 == stMsg.paylaod[0] && 1 == stMsg.paylaodLens)
				//					{
				//						ResCode = UWB_Err_Success_0;
				//					}
				//					else
				//					{
				//						ResCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.paylaod[0];
				//						pst_state->bIsInCmd = false;
				//					}

			}
			else if(NXP_UCI_MSG_OID_CORE_GENERIC_ERROR_NTF == stMsg.unioOCT1.stBitPart.oid)
			{
				pst_state->eOpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)stMsg.pbufPaylaod[0];
				pst_state->eNTFIdx = UBW_Deviec_Status_NTF;
			}
			else
			{
				pst_state->eOpCode = UWB_Err_UCI_Status_Unknown_GID;
				pst_state->bIsInCmd = false;
				pst_state->eNTFIdx = UBW_Deviec_Status_NTF;
			}
			break;//CORE_GROUP END

		case NXP_UCI_MSG_GID_UWB_SESS_CONFIG_GROUP :
			//if(NXP_UCI_MSG_OID_SESSION_STATUS_NTF == stMsg.OCT1.BitPart.oid)
			if(NXP_UCI_MSG_OID_SESSION_STATUS_NTF == (pst_recv->pbufDatHeader[2]&0x3F))
			{
				if(pst_state->stSession.u32CurrSessionID == core_dcm_mku32(stMsg.pbufPaylaod[3],stMsg.pbufPaylaod[2],stMsg.pbufPaylaod[1],stMsg.pbufPaylaod[0]))
				{
					switch(stMsg.pbufPaylaod[4])
					{
					case 0:		pst_state->stSession.eSesionStat = Session_Stat_Init;break;
					case 1:		pst_state->stSession.eSesionStat = Session_Stat_Deinit;break;
					case 2:		pst_state->stSession.eSesionStat = Session_Stat_Aciv;break;
					case 3:		pst_state->stSession.eSesionStat = Session_Stat_Idle;break;
					case 0xFF:	pst_state->stSession.eSesionStat = Session_Stat_Err;
						if(Session_Stat_Idle == pst_state->stSession.eSesionStat)
						{
							pst_state->stSession.eSesionStat = Session_Stat_Init;
						}
						else
						{

						}
						break;
					default:
						pst_state->stSession.eSesionStat = Session_Stat_RFU;
						break;
					}

					switch(stMsg.pbufPaylaod[5])
					{
					case 0:		pst_state->eOpCode = UWB_Err_Session_Status_NTF_Reason_Code_STATE_CHANGE_WITH_SESSION_MANAGEMENT_COMMANDS;break;
					case 1:		pst_state->eOpCode = UWB_Err_Session_Status_NTF_Reason_Code_MAX_RANGING_ROUND_RETRY_COUNT_REACHED;break;
					case 2:		pst_state->eOpCode = UWB_Err_Session_Status_NTF_Reason_Code_MAX_RANGING_BLOCKS_REACHED;break;
					case 3:		pst_state->eOpCode = UWB_Err_Session_Status_NTF_Reason_Code_URSK_EXPIRED_URSK;break;
					case 4:		pst_state->eOpCode = UWB_Err_Session_Status_NTF_Reason_Code_TERMINATION_ON_MAX_STS_MAX_STS;break;
					default:
						pst_state->eOpCode = UWB_Err_Session_Status_NTF_Reason_Code_RFU;break;
					}
					pst_state->bIsInCmd = false;
					pst_state->eNTFIdx = UWB_Session_Status_NTF;
				}
				else
				{
					//do nothing .
					pst_state->eOpCode = UWB_Err_Session_Status_SessionID_Incorrect;
					pst_state->eNTFIdx = UWB_Session_Status_NTF;
					pst_state->bIsInCmd = false;
				}
			}
			else
			{
				pst_state->eOpCode = UWB_Err_RANGING_SESSION_CONFIG_GROUP_OID_Incorrect;
				pst_state->eNTFIdx = UWB_Session_Status_NTF;
				pst_state->bIsInCmd = false;
			}
			break;//CONFIG_GROUP END

		case NXP_UCI_MSG_GID_UWB_RANGING_SESSION_CTRL_GROUP:
			//if(NXP_UCI_MSG_OID_RANGE_CCC_DATA_NTF == stMsg.OCT1.BitPart.oid)
			if(NXP_UCI_MSG_OID_RANGE_CCC_DATA_NTF == (pst_recv->pbufDatHeader[2]&0x3F))
			{
				if(pst_state->stSession.u32CurrSessionID == core_dcm_mku32(stMsg.pbufPaylaod[3],stMsg.pbufPaylaod[2],stMsg.pbufPaylaod[1],stMsg.pbufPaylaod[0]))
				{
					uint8_t status = 0;
					if(pst_state->bIsResponderUWBDeviceType)
					{//Responder
						status = (stMsg.pbufPaylaod[4] & 0xF0) >> 4;
					}
					else
					{//Initiator
						status = (stMsg.pbufPaylaod[4] & 0x0F);
					}

					if(0xA0 == pst_state->stSession.u8UCIRangSessionMode)//CCC
					{
						switch(status)
						{
						case 0: pst_state->eOpCode = UWB_Err_UCI_Device_Status_Ok;break;	//Good news
							//Bad news
						case 1: pst_state->eOpCode = 	UWB_Err_Ranging_NTF_Status_Transaction_Overflow;break;
						case 2:	pst_state->eOpCode = 	UWB_Err_Ranging_NTF_Status_Transaction_invalid;break;
						case 3:	pst_state->eOpCode = 	UWB_Err_Ranging_NTF_Status_Transaction_Frame_Incorrect;break;
						case 4:	pst_state->eOpCode = 	UWB_Err_Ranging_NTF_Status_Loss_of_ranging_control_information;break;
						default:
							break;
						}
						core_mm_copy(pst_state->stSession.bufCCMTag, stMsg.pbufPaylaod + 15, 8);//big ? lit?
#if defined(UWB_INITIATOR)
						pst_state->stSession.u32ResponderBitMap			= core_dcm_mku32(stMsg.pbufPaylaod[14],stMsg.pbufPaylaod[13],stMsg.pbufPaylaod[12],stMsg.pbufPaylaod[11]);
#elif defined(UWB_RESPONDER)
						pst_state->stSession.u8UncertaintyInitiator 	= stMsg.pbufPaylaod[14];
						pst_state->stSession.u8UncertaintyAnchor 		= stMsg.pbufPaylaod[13];
						pst_state->stSession.u16CurrentDistance 		= core_dcm_mku16(stMsg.pbufPaylaod[12],stMsg.pbufPaylaod[11]);
#else
						This area will be Never be enable .
#endif
						pst_state->stSession.u16NextRRIndex	 			= core_dcm_mku16(stMsg.pbufPaylaod[10],stMsg.pbufPaylaod[9]);
						pst_state->stSession.u32STSIndex 				= core_dcm_mku32(stMsg.pbufPaylaod[8],stMsg.pbufPaylaod[7],stMsg.pbufPaylaod[6],stMsg.pbufPaylaod[5]);
						pst_state->stSession.u32CurrSessionID			= core_dcm_mku32(stMsg.pbufPaylaod[3],stMsg.pbufPaylaod[2],stMsg.pbufPaylaod[1],stMsg.pbufPaylaod[0]);

					}
					else
					{
						//Unsupport session mode .
						pst_state->eOpCode = UWB_Err_Session_Status_Unsupported_Session_Mode;
					}
#if defined(_COMMUNI_TYPE_IS_UART)
					//// change buffer when current used is full .
					//if(pst_state->stSession.stRangingResult.u8ResultCnt >=10)
					//{
					//	pst_state->stSession.stRangingResult.bIsResultXSlot1Full = !pst_state->stSession.stRangingResult.bIsResultXSlot1Full;
					//	pst_state->stSession.stRangingResult.u8ResultCnt = 0;
					//}

					//if(pst_state->stSession.stRangingResult.bIsResultXSlot1Full)
					//{
					//	pst_state->stSession.stRangingResult.pbufResult = pst_state->stSession.stRangingResult.bufResultXSlot1;
					//	core_mm_copy(pst_state->stSession.stRangingResult.bufResultXSlot2 + (pst_state->stSession.stRangingResult.u8ResultCnt * stMsg.u16PaylaodLens),
					//		stMsg.pbufPaylaod,
					//		stMsg.u16PaylaodLens);
					//}
					//else
					//{
					//	pst_state->stSession.stRangingResult.pbufResult = pst_state->stSession.stRangingResult.bufResultXSlot2;
					//	core_mm_copy(pst_state->stSession.stRangingResult.bufResultXSlot1 + (pst_state->stSession.stRangingResult.u8ResultCnt * stMsg.u16PaylaodLens),
					//		stMsg.pbufPaylaod,
					//		stMsg.u16PaylaodLens);
					//}
					//pst_state->stSession.stRangingResult.u8ResultCnt += 1;
#endif
					pst_state->eMSGIdx = UWB_Ranging_Result_Notice;
					pst_state->bIsInCmd = false;
				}
				else
				{
					pst_state->eOpCode = UWB_Err_Session_Status_SessionID_Incorrect;//session error
					pst_state->eNTFIdx = UWB_Session_Status_NTF;
					pst_state->bIsInCmd = false;
				}
			}
			else
			{
				pst_state->eOpCode = UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect;
				pst_state->eNTFIdx = UWB_Session_Status_NTF;
				pst_state->bIsInCmd = false;
			}
			break;//RANGING_SESSION_CTRL_GROUP END

		default:
			pst_state->eOpCode = UWB_Err_Session_GID_UnSupported;
			pst_state->eNTFIdx = UWB_Session_Status_NTF;
			pst_state->bIsInCmd = false;
			//GID Error
			break;
			//NOTIFICATION END
		}
	}
	else
	{
		//Never in to this here .
	}
}

/****
 * \brief uci recv data analyze
 * //analyze command and change the direction .
 *
 */
static void nxp_uci_res_parse(ST_NXPUCIDat* pst_recv, ST_UCIFrameState* pst_state)
{
	nxp_uci_ccc_res_analyze(pst_recv, pst_state);
}



#if 0 
abandoned code
static UWBErrCode nxp_uci_proprietary_cmd_package(uint32_t cmd, st_nxp_uci_dat* pst_send, st_uci_frame_state* pstate)
{
	UWBErrCode ResCode = UWB_Err_Success_0;
	return ResCode;
}

static void nxp_uci_ccc_res_analyze(ST_NXPUCIDat* pst_recv, ST_UCIFrameState *pst_state)
{
	st_nxp_uci_msg			msg;
	const uint8_t 			nxp_uci_stat_success	= 0;

	core_mm_set((uint8_t*)&msg, 0, sizeof(st_nxp_uci_msg));

	//uci_data[0] is padding 0x00
	msg.PaddingByte				= pst_recv->pDatHeader[0];
	msg.OCT0.BytePart 			= pst_recv->pDatHeader[1];
	msg.OCT1.BytePart 			= pst_recv->pDatHeader[2];
	msg.OCT2 					= pst_recv->pDatHeader[3];
	msg.OCT3 					= pst_recv->pDatHeader[4];
	if(0 == (uint8_t)msg.OCT1.BitPart.rfu1 )
	{
		msg.paylaodLens				= (uint16_t)msg.OCT3;
	}
	else
	{
		msg.paylaodLens				= (uint16_t)core_dcm_mku16(msg.OCT2, msg.OCT3);
	}
	msg.paylaod 				= pst_recv->pbufDatBody;

	/* ********************
	* Type 2:
	* Response
	* Notification
	*
	* Group 3:
	* CORE_GROUP
	* SESS_CONFIG_GROUP
	* SESSION_CTRL_GROUP
	*
	* add by John song
	*/
	//if(NXP_UCI_MSG_MIT_CTRL_RES == msg.OCT0.BitPart.mt)
	if(NXP_UCI_MSG_MIT_CTRL_RES == pst_recv->pDatHeader[1]>>5)
	{
		//switch(msg.OCT0.BitPart.gid)
		switch(pst_recv->pDatHeader[1]&0x0F)
		{
		case NXP_UCI_MSG_GID_UCI_CORE_GROUP :
			//switch(msg.OCT1.BitPart.oid)
			switch(pst_recv->pDatHeader[2]&0x3F)
			{
			case NXP_UCI_MSG_OID_CORE_DEVICE_RESET_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					pstate->IsInCmd = false;
					pstate->MSGIdx = UWB_Hard_Reset_RS;
				}
				else
				{
					pstate->OpCode = UWB_Err_UCI_Rest_Failed;
					pstate->IsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_CORE_GET_DEVICE_INFO_RES:						//没做
				break;
			case NXP_UCI_MSG_OID_CORE_GET_CAPS_INFO_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					uint8_t* t 		= msg.paylaod + 2;
					uint8_t* t1		= NULL;
					for(uint8_t i=0,tlvCnt = msg.paylaod[1]; i < tlvCnt; i++)
					{
						switch(*t)
						{
						case 0xA0: t1 = ArrDeviceCapabilityParames + OFST_DCP_A0_SLOT_BITMASK;break;
						case 0xA1: t1 = ArrDeviceCapabilityParames + OFST_DCP_A1_SYNC_CODE_INDEX_BITMASK;break;
						case 0xA2: t1 = ArrDeviceCapabilityParames + OFST_DCP_A2_HOPPING_CONFIG_BITMASK;break;
						case 0xA3: t1 = ArrDeviceCapabilityParames + OFST_DCP_A3_CHANNEL_BITMASK;break;
						case 0xA4: t1 = ArrDeviceCapabilityParames + OFST_DCP_A4_SUPPORTED_PROTOCOL_VERSION;break;
						case 0xA5: t1 = ArrDeviceCapabilityParames + OFST_DCP_A5_SUPPORTED_UWB_CONFIG_ID;break;
						case 0xA6: t1 = ArrDeviceCapabilityParames + OFST_DCP_A6_SUPPORTED_PULSESHAPE_COMBO;break;
						default:
							break;
						}
						if(NULL != t1)
						{
							core_mm_copy(t1,t,*(t+1)+2);
						}
						else
						{

						}
						t = t+(*(t+1)+2);
					}
					pstate->OpCode = UWB_Err_Success_0;
				}
				else
				{
					pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
				}
				pstate->IsInCmd = false;
				pstate->MSGIdx = UWB_Anchor_WakeUp_RS;
				break;
			case NXP_UCI_MSG_OID_CORE_SET_CONFIG_RES:
				pstate->MSGIdx = UWB_Core_Config_Setting_RS;
				pstate->IsInCmd = false;
				break;

			case NXP_UCI_MSG_OID_CORE_GET_CONFIG_RES:						break;//没做
			default:
				pstate->OpCode = UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect;
				pstate->IsInCmd = false;
				break;
			}
			break;//CORE_GROUP END

			//SESS_CONFIG_GROUP
		case NXP_UCI_MSG_GID_UWB_SESS_CONFIG_GROUP :
			//switch(msg.OCT1.BitPart.oid)
			switch(pst_recv->pDatHeader[2]&0x3F)
			{
			case NXP_UCI_MSG_OID_SESSION_INIT_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					pstate->RangSession.UCI_Session_Init_Is_Done = true;
					pstate->IsInCmd = false;
					pstate->MSGIdx = UWB_Ranging_Session_Init_RS;
					pstate->WorkStat = Module_Stat_Active;
					pstate->RangSession.SesionStat = Session_Stat_Init;
				}
				else
				{
					pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
					pstate->IsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_DEINIT_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					pstate->RangSession.UCI_Session_Init_Is_Done = false;
					pstate->RangSession.UCI_Session_APP_Config_Is_Done = false;
					pstate->RangSession.SesionStat = Session_Stat_Deinit;
					pstate->RangSession.CurrSessionID = 0;
					pstate->IsInCmd = false;
				}
				else
				{
					pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
					pstate->IsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_SET_APP_CONFIG_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{//????????8怎么处理？
					pstate->RangSession.UCI_Session_APP_Config_Is_Done = true;
					pstate->RangSession.SesionStat = Session_Stat_Idle;
					pstate->MSGIdx = UWB_Ranging_Session_App_Config_RS;
					pstate->IsInCmd = false;
				}
				else
				{
					pstate->RangSession.UCI_Session_APP_Config_Is_Done = false;
					pstate->RangSession.SesionStat = Session_Stat_Init;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_GET_APP_CONFIG_RES:break;
			case NXP_UCI_MSG_OID_SESSION_GET_COUNT_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					pstate->UWBSessionCnt = msg.paylaod[1];
					if(0 == pstate->UWBSessionCnt)
					{
						//when the current session count is ZERO , must be gouing to Ready mode .
						pstate->WorkStat = Module_Stat_Ready;
					}
					else
					{
						//do nothing .
					}
				}
				else
				{
					pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
					pstate->IsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_GET_STATE_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					if(Session_Stat_RFU <= msg.paylaod[1])
					{
						pstate->RangSession.SesionStat = Session_Stat_RFU;
					}
					else
					{
						pstate->RangSession.SesionStat = msg.paylaod[1];
					}
				}
				else
				{
					pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
					pstate->IsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_SESSION_GET_POSSIBLE_RAN_MULTIPLIER_VALUE_RES:break;//没做
			default:
				pstate->OpCode = UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect;
				pstate->IsInCmd = false;
				break;
			}
			break;//SESS_CONFIG_GROUP END

			//SESSION_CTRL_GROUP
		case NXP_UCI_MSG_GID_UWB_RANGING_SESSION_CTRL_GROUP:
			//switch(msg.OCT1.BitPart.oid)
			switch(pst_recv->pDatHeader[2]&0x3F)
			{
			case NXP_UCI_MSG_OID_RANGE_START_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					pstate->RangSession.SesionStat = Session_Stat_Aciv;
					pstate->IsInCmd = false;
					pstate->MSGIdx = UWB_Ranging_Session_Start_RS;
				}
				else
				{
					pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
					pstate->IsInCmd = false;
				}

				break;
			case NXP_UCI_MSG_OID_RANGE_STOP_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					pstate->RangSession.SesionStat = Session_Stat_Idle;
					pstate->IsInCmd = true;
					pstate->MSGIdx = UWB_Ranging_Session_Deinit_RQ;
				}
				else
				{
					pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
					pstate->IsInCmd = false;
				}
				break;
			case NXP_UCI_MSG_OID_RANGE_GET_RANGING_COUNT_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					pstate->RangSession.CurrentRangingCnt = core_dcm_mku32(msg.paylaod[4],msg.paylaod[3],msg.paylaod[2],msg.paylaod[1]);
					pstate->IsInCmd = false;
					pstate->MSGIdx = UWB_Ranging_Session_GetRangCnt_RS;
				}
				break;
			case NXP_UCI_MSG_OID_RANGE_RESUME_RES:
				if(nxp_uci_stat_success == msg.paylaod[0])
				{
					pstate->RangSession.SesionStat = Session_Stat_Idle;
					pstate->IsInCmd = false;
					pstate->MSGIdx = UWB_Ranging_Session_Recover_RS;
				}
				else
				{
					pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
					pstate->IsInCmd = false;
				}
				break;
			default:
				pstate->OpCode = UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect;
				pstate->IsInCmd = false;
				break;
			}
			break;//SESSION_CTRL_GROUP END
		case NXP_UCI_MSG_PROPRIETARY_GROUP_0:
			//switch(msg.OCT1.BitPart.oid)
			switch(pst_recv->pDatHeader[2]&0x3F)
			{
			case NXP_UCI_MSG_OID_PROPRIETARY_RADIO_CONFIG_DOWNLOAD_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_ACTIVATE_SWUP_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_TEST_START_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_TEST_STOP_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_QUERY_UWB_TIMESTAMP_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_DEVICE_SUSPEND_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_SET_TRIM_VALUES_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_GET_ALL_UWB_SESSIONS_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_GET_TRIM_VALUES_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_STORE_PROTECTION_KEY_RSP: break;
			case NXP_UCI_MSG_OID_PROPRIETARY_SET_EPOCH_ID_RSP: break;
			default:
				break;
			}
			break;//NXP_UCI_MSG_PROPRIETARY_GROUP_0 END
		default:
			//GID Error
			pstate->OpCode = UWB_Err_Session_GID_UnSupported;
			pstate->IsInCmd = false;
			break;
			//RESPONSE END
		}
	}
	//else if(NXP_UCI_MSG_MIT_CTRL_NOTIFICATION == msg.OCT0.BitPart.mt)
	else if(NXP_UCI_MSG_MIT_CTRL_NOTIFICATION == pst_recv->pDatHeader[1]>>5)
	{

		//switch(msg.OCT0.BitPart.gid)
		switch(pst_recv->pDatHeader[1]&0x0F)
		{
		case NXP_UCI_MSG_GID_UCI_CORE_GROUP :
			if(NXP_UCI_MSG_OID_CORE_DEVICE_STATUS_NTF == msg.OCT1.BitPart.oid)
			{
				pstate->NTFIdx = UBW_Deviec_Status_NTF;
				//success:006001000101
				//if((nxp_uci_stat_success ==msg.payload_lens0)&&(1 == msg.payload_lens1)&&(1 == msg.paylaod[0]))
				pstate->OpCode = UWB_Err_UCI_Device_Status_Ok | msg.paylaod[0];
				//					switch(recv_buff_ptr[5])
				//					{
				//						case 0x01: ResCode = UWB_Err_Success_0;break;
				//						case 0x02: ResCode = UWB_Err_UCI_Device_Status_NTF_RangingSession_Is_Bussy;break;
				//						case 0xFF: ResCode = UWB_Err_UCI_Device_Status_NTF_Status_Error;break;
				//						default:
				//							ResCode = UWB_Err_UCI_Device_Status_NTF_Status_RFU_Error;
				//							break;
				//					}
				//					if(1 == msg.paylaod[0] && 1 == msg.paylaodLens)
				//					{
				//						ResCode = UWB_Err_Success_0;
				//					}
				//					else
				//					{
				//						ResCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
				//						pstate->IsInCmd = false;
				//					}

			}
			else if(NXP_UCI_MSG_OID_CORE_GENERIC_ERROR_NTF == msg.OCT1.BitPart.oid)
			{
				pstate->OpCode = (uint32_t)UWB_Err_UCI_Status_Ok|(uint32_t)msg.paylaod[0];
				pstate->NTFIdx = UBW_Deviec_Status_NTF;
			}
			else
			{
				pstate->OpCode = UWB_Err_UCI_Status_Unknown_GID;
				pstate->IsInCmd = false;
				pstate->NTFIdx = UBW_Deviec_Status_NTF;
			}
			break;//CORE_GROUP END

		case NXP_UCI_MSG_GID_UWB_SESS_CONFIG_GROUP :
			//if(NXP_UCI_MSG_OID_SESSION_STATUS_NTF == msg.OCT1.BitPart.oid)
			if(NXP_UCI_MSG_OID_SESSION_STATUS_NTF == (pst_recv->pDatHeader[2]&0x3F))
			{
				if(pstate->RangSession.CurrSessionID == core_dcm_mku32(msg.paylaod[3],msg.paylaod[2],msg.paylaod[1],msg.paylaod[0]))
				{
					switch(msg.paylaod[4])
					{
					case 0:		pstate->RangSession.SesionStat = Session_Stat_Init;break;
					case 1:		pstate->RangSession.SesionStat = Session_Stat_Deinit;break;
					case 2:		pstate->RangSession.SesionStat = Session_Stat_Aciv;break;
					case 3:		pstate->RangSession.SesionStat = Session_Stat_Idle;break;
					case 0xFF:	pstate->RangSession.SesionStat = Session_Stat_Err;
						if(Session_Stat_Idle == pstate->RangSession.SesionStat)
						{
							pstate->RangSession.SesionStat = Session_Stat_Init;
						}
						else
						{

						}
						break;
					default:
						pstate->RangSession.SesionStat = Session_Stat_RFU;
						break;
					}

					switch(msg.paylaod[5])
					{
					case 0:		pstate->OpCode = UWB_Err_Session_Status_NTF_Reason_Code_STATE_CHANGE_WITH_SESSION_MANAGEMENT_COMMANDS;break;
					case 1:		pstate->OpCode = UWB_Err_Session_Status_NTF_Reason_Code_MAX_RANGING_ROUND_RETRY_COUNT_REACHED;break;
					case 2:		pstate->OpCode = UWB_Err_Session_Status_NTF_Reason_Code_MAX_RANGING_BLOCKS_REACHED;break;
					case 3:		pstate->OpCode = UWB_Err_Session_Status_NTF_Reason_Code_URSK_EXPIRED_URSK;break;
					case 4:		pstate->OpCode = UWB_Err_Session_Status_NTF_Reason_Code_TERMINATION_ON_MAX_STS_MAX_STS;break;
					default:
						pstate->OpCode = UWB_Err_Session_Status_NTF_Reason_Code_RFU;break;
					}
					pstate->IsInCmd = false;
					pstate->NTFIdx = UWB_Session_Status_NTF;
				}
				else
				{
					//do nothing .
					pstate->OpCode = UWB_Err_Session_Status_SessionID_Incorrect;
					pstate->NTFIdx = UWB_Session_Status_NTF;
					pstate->IsInCmd = false;
				}
			}
			else
			{
				pstate->OpCode = UWB_Err_RANGING_SESSION_CONFIG_GROUP_OID_Incorrect;
				pstate->NTFIdx = UWB_Session_Status_NTF;
				pstate->IsInCmd = false;
			}
			break;//CONFIG_GROUP END

		case NXP_UCI_MSG_GID_UWB_RANGING_SESSION_CTRL_GROUP:
			//if(NXP_UCI_MSG_OID_RANGE_CCC_DATA_NTF == msg.OCT1.BitPart.oid)
			if(NXP_UCI_MSG_OID_RANGE_CCC_DATA_NTF == (pst_recv->pDatHeader[2]&0x3F))
			{
				if(pstate->RangSession.CurrSessionID == core_dcm_mku32(msg.paylaod[3],msg.paylaod[2],msg.paylaod[1],msg.paylaod[0]))
				{
					uint8_t status = 0;
					if(pstate->UWBDeviceTypeIsResponder)
					{//Responder
						status = (msg.paylaod[4] & 0xF0) >> 4;
					}
					else
					{//Initiator
						status = (msg.paylaod[4] & 0x0F);
					}

					if(0xA0 == pstate->RangSession.UCI_Rang_Session_Mode)//CCC
					{
						switch(status)
						{
						case 0: pstate->OpCode = UWB_Err_UCI_Device_Status_Ok;break;	//Good news
							//Bad news
						case 1: pstate->OpCode = 	UWB_Err_Ranging_NTF_Status_Transaction_Overflow;break;
						case 2:	pstate->OpCode = 	UWB_Err_Ranging_NTF_Status_Transaction_invalid;break;
						case 3:	pstate->OpCode = 	UWB_Err_Ranging_NTF_Status_Transaction_Frame_Incorrect;break;
						case 4:	pstate->OpCode = 	UWB_Err_Ranging_NTF_Status_Loss_of_ranging_control_information;break;
						default:
							break;
						}
						core_mm_copy(pstate->RangSession.CCMTag, msg.paylaod + 15, 8);//big ? lit?
#if defined(UWB_INITIATOR)
						pstate->RangSession.ResponderBitMap			= core_dcm_mku32(msg.paylaod[14],msg.paylaod[13],msg.paylaod[12],msg.paylaod[11]);
#elif defined(UWB_RESPONDER)
						pstate->RangSession.Uncertainty_Initiator 	= msg.paylaod[14];
						pstate->RangSession.Uncertainty_Anchor 		= msg.paylaod[13];
						pstate->RangSession.Current_Distance 		= core_dcm_mku16(msg.paylaod[12],msg.paylaod[11]);
#else
						This area will be Never be enable .
#endif
							pstate->RangSession.Next_RRIndex	 		= core_dcm_mku16(msg.paylaod[10],msg.paylaod[9]);
						pstate->RangSession.STSIndex 				= core_dcm_mku32(msg.paylaod[8],msg.paylaod[7],msg.paylaod[6],msg.paylaod[5]);
						pstate->RangSession.CurrSessionID			= core_dcm_mku32(msg.paylaod[3],msg.paylaod[2],msg.paylaod[1],msg.paylaod[0]);

					}
					else
					{
						//Unsupport session mode .
						pstate->OpCode = UWB_Err_Session_Status_Unsupported_Session_Mode;
					}
#if defined(_COMMUNI_TYPE_IS_UART)
					// change buffer when current used is full .
					if(pstate->RangSession.stRangingResult.ResultCnt >=10)
					{
						pstate->RangSession.stRangingResult.IsResultXSlot1Full = !pstate->RangSession.stRangingResult.IsResultXSlot1Full;
						pstate->RangSession.stRangingResult.ResultCnt = 0;
					}

					if(pstate->RangSession.stRangingResult.IsResultXSlot1Full)
					{
						pstate->RangSession.stRangingResult.ptrResult = pstate->RangSession.stRangingResult.ResultXSlot1;
						core_mm_copy(pstate->RangSession.stRangingResult.ResultXSlot2 + (pstate->RangSession.stRangingResult.ResultCnt * msg.paylaodLens),
							msg.paylaod,
							msg.paylaodLens);
					}
					else
					{
						pstate->RangSession.stRangingResult.ptrResult = pstate->RangSession.stRangingResult.ResultXSlot2;
						core_mm_copy(pstate->RangSession.stRangingResult.ResultXSlot1 + (pstate->RangSession.stRangingResult.ResultCnt * msg.paylaodLens),
							msg.paylaod,
							msg.paylaodLens);
					}
					pstate->RangSession.stRangingResult.ResultCnt += 1;
#endif
					pstate->MSGIdx = UWB_Ranging_Result_Notice;
					pstate->IsInCmd = false;
				}
				else
				{
					pstate->OpCode = UWB_Err_Session_Status_SessionID_Incorrect;//session error
					pstate->NTFIdx = UWB_Session_Status_NTF;
					pstate->IsInCmd = false;
				}
			}
			else
			{
				pstate->OpCode = UWB_Err_RANGING_SESSION_CTRL_GROUP_OID_Incorrect;
				pstate->NTFIdx = UWB_Session_Status_NTF;
				pstate->IsInCmd = false;
			}
			break;//RANGING_SESSION_CTRL_GROUP END

		default:
			pstate->OpCode = UWB_Err_Session_GID_UnSupported;
			pstate->NTFIdx = UWB_Session_Status_NTF;
			pstate->IsInCmd = false;
			//GID Error
			break;
			//NOTIFICATION END
		}
	}
	else
	{
		//Never in to this here .
	}
}

#endif



