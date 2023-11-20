/*
 * uwb_responder_api.c
 *
 *  Created on: 2022-6-30
 *      Author: JohnSong
 */

#include "../uwb_common_def.h"
#if defined(UWB_RESPONDER)
#include <components/aES/ES000501_uwb/Sources/UWBResponder/uwb_responder_api.h>
#include <components/aES/ES000501_uwb/Sources/UWB_UCI_Frame/nxp_uci_cmd.h>
//#include "../aEM/EM00040101_log/EM000401.h"
//#include "../UWB_UCI_Frame/nxp_uci_cmd_param_table.h"

E_UWBErrCode API_UWB_Responder_Init(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;

	//core_mm_set((uint8_t*)&pst_uwb_source->stUCIState, 		0, sizeof(ST_UCIFrameState));
	//core_mm_set((uint8_t*)&pst_uwb_source->stUCIState.stTimerTools, 	0, sizeof(ST_TimerTools));
	//core_mm_set((uint8_t*)&pst_uwb_source->stUCIState.stUWBCommu, 		0, sizeof(ST_UWBCommuAttr));

	pst_uwb_source->stUCIState.stSession.u8UCIRangSessionMode 	= 0xA0;//Default CCC mode
	pst_uwb_source->stUCIState.eMSGIdx 							= UWB_Hard_Reset_RQ;
	pst_uwb_source->stUCIState.bIsInCmd 						= true;
	pst_uwb_source->stUCIState.eState 							= UciFrameState_Init;
	pst_uwb_source->stUCIState.bIsResponderUWBDeviceType		= true;
	API_UCI_Frame_Init(&pst_uwb_source->stUCIState);				//st_uci_state.IsInCmd = true;
	return ResCode;
}
E_UWBErrCode API_UWB_Responder_WorkProcedure(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode		ResCode 		= UWB_Err_Success_0;
	uint16_t			sendlens 		= 0;

	int seq_cnt = 0;
	//step 4. PreProcessing
	if(true == pst_uwb_source->stUCIState.bIsInCmd)
	{
//		switch(pst_uwb_source->stUCIState.eMSGIdx)
//		{
//	    	case UWB_Anchor_WakeUp_RQ:break;
//	    	case UWB_Timer_Sync_RQ:break;
//	    	case UWB_Ranging_Session_Init_RQ:break;
//	    	case UWB_Ranging_Session_App_Config_RQ:break;
//	    	case UWB_Ranging_Session_Start_RQ:break;
//	    	case UWB_Ranging_Session_Suspend_RQ:break;
//	    	case UWB_Ranging_Session_Recover_RQ:break;
//	    	case UWB_Ranging_Session_Stop_RQ:break;
//	    	case UWB_Ranging_Session_Deinit_RQ:break;
//			default:
//                	break;
//		}

		//step 5. Call the uci frame main function go in to the UWB control entry
		while(pst_uwb_source->stUCIState.bIsInCmd)//when the work flow is INComing , mean the 29d5 transcve isn't done . add by JonSong
		{
			ResCode = API_UCI_Frame_Entry(&pst_uwb_source->stUCIState);

			if(UWB_Err_COMMU_Send_Cancel == ResCode)
			{
				continue;
			}
			if(true == pst_uwb_source->stUCIState.bIsNTFCache)
			{
				pst_uwb_source->stUCIState.bIsInCmd = true;
				pst_uwb_source->stUCIState.bIsNTFCache = false;
				continue;
			}
			if(UWB_Err_Success_0 == ResCode)
			{
				if(UWB_MSG_Is_Null != pst_uwb_source->stUCIState.eNTFIdx)
				{
					UWB_NTF_Check(&pst_uwb_source->stUCIState);
				}

				if(false == pst_uwb_source->stUCIState.bIsInCmd)
				{
					break;
				}
			}
			else
			{
				//LOG_L_S(CCC_MD,"UCI Frame Report Error !!!!");
				//set process to reset 29D5
				//break;
			}
		}
	}

	//step 6. PostProcessing
	if((false == pst_uwb_source->stUCIState.bIsInCmd) && (UWB_MSG_Is_Null != pst_uwb_source->stUCIState.eMSGIdx))
	{
		switch(pst_uwb_source->stUCIState.eMSGIdx)
		{
		case UWB_Anchor_WakeUp_RS:
			//LOG_L_S(CCC_MD," UWB Anchor Wake up is Success .");
			break;
		case UWB_Timer_Sync_RS:
			//LOG_L_S(CCC_MD," UWB Time [SYNC] is Success.");
			break;
		case UWB_Ranging_Session_Init_RS:
			//LOG_L_S(CCC_MD," RANGING INIT is Success .");
			break;
		case UWB_Ranging_Session_App_Config_RS:
			//LOG_L_S(CCC_MD," RANGING APP Config is Success .");
			break;
		case UWB_Ranging_Session_Start_RS:
			//LOG_L_S(CCC_MD," RANGING [Start] is Success .");
			break;
		case UWB_Ranging_Session_Suspend_RS:
			//LOG_L_S(CCC_MD," RANGING [Suspend] is Success .");
			break;
		case UWB_Ranging_Session_Recover_RS:
			//LOG_L_S(CCC_MD," RANGING [Recover] is Success .");
			break;
		case UWB_Ranging_Session_Stop_RS:
			//LOG_L_S(CCC_MD," RANGING [Stop] is Success .");
			break;
		case UWB_Ranging_Session_Deinit_RS:
			//LOG_L_S(CCC_MD," RANGING [Deinit] is Success .");
			break;
		case UWB_Ranging_Result_Notice:
			if(pst_uwb_source->stUCIState.stSession.stRangingResult.u8ResultCnt >= 10)
			{
				//RANGING_RESULT_OUTPUT(pst->st_uci_state.RangSession.stRangingResult.ptrResult);
			}
			break;
		case UWB_Core_Config_Setting_RS:
			//LOG_L_S(CCC_MD," CORE_SET_CONFIG is Success .");
			break;
		case UWB_Hard_Reset_RS:
			//LOG_L_S(CCC_MD," UWB Module Reset is Success .");
			break;
			//The notification message is sent out, either CAN or Uart, and the next step is determined by the outside.
		default:
			//LOG_L_S(CCC_MD," Un defined CMD RS .");
			break;
		}
		pst_uwb_source->stUCIState.bIsInCmd = true;
		pst_uwb_source->stUCIState.eMSGIdx 	= UWB_MSG_Is_Null;
	}
	//step 7. Send CAN or Uart msg to BUS .

	return ResCode;
}


#endif/* UWB_RESPONDER */

#if 0 
abandoned code
const uint8_t TEST_CMD_SEQU_1_REST [] 				= {0x20, 0x00, 0x00, 0x01, 0x00};
const uint8_t TEST_CMD_SEQU_4_SET_CORE_CONFIG_1[]	= {0x20,0x04,0x00,0x0A,0x02,0xF6,0x04,0x0C,0x04,0x00,0x00,0xF4,0x01,0x01};
const uint8_t TEST_CMD_SEQU_5_INIT_SESSION[] 		= {0x21,0x00,0x00,0x05,0x03,0x00,0x00,0x00,0xA0};
const uint8_t TEST_CMD_SEQU_6_APP_SESSION_CONFIG[]	= {0x21,0x03,0x00,0x97,0x03,0x00,0x00,0x00,0x1B,0x11,0x01,0x00,0x04,0x01,0x09,
		0x05,0x01,0x05,/*Reponder 数量，与 Initiator 要一致*/
		0x1E,0x01,0x04,/*RESPONDER_SLOT_INDEX Responder 编号 2*/
		0x14,0x01,0x0A,0x0A,0x04,0x00,0x00,0x00,0x00,0xE9,0x12,0x04,0x00,0x49,0x43,0x54,0x53,0x53,0x54,0x41,0x54,0x49,0x43,0x54,0x53,0x53,0x54,0x41,0x54,0xE9,0x12,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x49,0x43,0x54,0x53,0x53,0x54,0x41,0x54,0x06,0x02,0x00,0x01,0x08,0x02,0xb0,0x04,0x09,0x04,0x60,0x00,0x00,0x00,0xF2,0x01,0x14,0x2A,0x02,0x0F,0x00,0x2C,0x01,0x00,0x15,0x01,0x00,0x1B,0x01,0x0C,0x23,0x01,0x03,0xA0,0x10,0x73,0x05,0x8F,0xE5,0x50,0x3C,0x1D,0x4A,0x17,0x54,0x1D,0x51,0x21,0xBB,0x7F,0x9E,0x32,0x02,0xFF,0xFF,0xA4,0x02,0x00,0x00,0xA5,0x01,0x11,0xA6,0x02,0xD0,0x02,0xF0,0x01,0x00,0xFB,0x04,0x02,0x03,0x12,0x13,0xFD,0x02,0xFF,0x01,0X25,0X01,0X60,0xf9,0x01,0x01};
const uint8_t TEST_CMD_SEQU_7_START_RANGING[] 		= {0x22,0x00,0x00,0x04,0x03,0x00,0x00,0x00};
UWBErrCode API_uwb_responder_ranging_task(void)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;
	st_uci_frame_state*	pst_uci_state 	= NULL;
	uint16_t			sendlens 		= 0;
	st_timer_tool_t		st_timer_tools;
	st_uwb_commu_attr_t	st_uwb_commu;

	//init
	core_mm_set((uint8_t*)&st_uci_state, 	0, sizeof(st_uci_frame_state));
	core_mm_set((uint8_t*)&st_timer_tools, 	0, sizeof(st_timer_tool_t));
	core_mm_set((uint8_t*)&st_uwb_commu, 	0, sizeof(st_uwb_commu_attr_t));

	pst_uci_state = &st_uci_state;

	pst_uci_state->RangSession.UCI_Rang_Session_Mode 	= 0xA0;//Default CCC mode
	pst_uci_state->MSGIdx 								= UWB_Hard_Reset_RQ;
	//pst_uci_state->ptrNoticBuffer 					= UWB_Task_buffer;//CommonBuffer
	pst_uci_state->IsInCmd 								= true;
	pst_uci_state->FrameState 							= UciFrameState_Init;

	pstTimerTools = &st_timer_tools;
	pstUWBCommu	  = &st_uwb_commu;

	pst_uci_state->UWBDeviceTypeIsResponder = true;

	API_uci_frame_init(pst_uci_state);			//st_uci_state.IsInCmd = true;

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	*
	*  Debug Code Being .
	*
	* */

	int seq_cnt = 0;

	while(true)
	{
		//step 1.LP --> goto sleep RTOS global value

		//step 2. CAN/UART -> recv msg RTOS global value

		//step 3. Check MSG coming data ?CAN?UART

		if((UWB_MSG_Is_Null == pst_uci_state->MSGIdx)&&(seq_cnt<=4))//recv msg
		{
			switch(seq_cnt)
			{
			case 0:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_1_REST, sizeof(TEST_CMD_SEQU_1_REST));
				st_uci_send.unLens = sizeof(TEST_CMD_SEQU_1_REST) -4;
				pst_uci_state->MSGIdx = UWB_Hard_Reset_RQ;
				pst_uci_state->IsInCmd = true;
				seq_cnt ++;
				break;
			case 1:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_4_SET_CORE_CONFIG_1, sizeof(TEST_CMD_SEQU_4_SET_CORE_CONFIG_1));
				st_uci_send.unLens = sizeof(TEST_CMD_SEQU_4_SET_CORE_CONFIG_1) -4;
				pst_uci_state->MSGIdx = UWB_Core_Config_Setting_RQ;
				pst_uci_state->IsInCmd = true;
				seq_cnt ++;
				break;
			case 2:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_5_INIT_SESSION, sizeof(TEST_CMD_SEQU_5_INIT_SESSION));
				st_uci_send.unLens = sizeof(TEST_CMD_SEQU_5_INIT_SESSION) - 4;
				pst_uci_state->RangSession.CurrSessionID = 3;
				pst_uci_state->RangSession.UCI_Rang_Session_Mode = TEST_CMD_SEQU_5_INIT_SESSION[8];
				pst_uci_state->MSGIdx = UWB_Ranging_Session_Init_RQ;
				pst_uci_state->IsInCmd = true;
				seq_cnt ++;
				break;
			case 3:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_6_APP_SESSION_CONFIG, sizeof(TEST_CMD_SEQU_6_APP_SESSION_CONFIG));
				st_uci_send.unLens = sizeof(TEST_CMD_SEQU_6_APP_SESSION_CONFIG) - 4;
				pst_uci_state->RangSession.CurrSessionID = 3;
				pst_uci_state->MSGIdx = UWB_Ranging_Session_App_Config_RQ;
				pst_uci_state->IsInCmd = true;
				seq_cnt ++;
				break;
			case 4:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_7_START_RANGING, sizeof(TEST_CMD_SEQU_7_START_RANGING));
				st_uci_send.unLens = sizeof(TEST_CMD_SEQU_7_START_RANGING) - 4;
				pst_uci_state->MSGIdx = UWB_Ranging_Session_Start_RQ;
				pst_uci_state->IsInCmd = true;
				seq_cnt ++;
				break;
			default:
				break;
			}
		}
		else
		{
			// do nothing .
		}

		//step 4. PreProcessing
		if(true == pst_uci_state->IsInCmd)
		{
			switch(pst_uci_state->MSGIdx)
			{
			case UWB_Anchor_WakeUp_RQ:break;
			case UWB_Timer_Sync_RQ:break;
			case UWB_Ranging_Session_Init_RQ:break;
			case UWB_Ranging_Session_App_Config_RQ:break;
			case UWB_Ranging_Session_Start_RQ:break;
			case UWB_Ranging_Session_Suspend_RQ:break;
			case UWB_Ranging_Session_Recover_RQ:break;
			case UWB_Ranging_Session_Stop_RQ:break;
			case UWB_Ranging_Session_Deinit_RQ:break;
			default:
				break;
			}

			//step 5. Call the uci frame main function go in to the UWB control entry
			while(pst_uci_state->IsInCmd)//when the work flow is INComing , mean the 29d5 transcve isn't done . add by JonSong
			{
				ResCode = API_uci_frame_entry(pst_uci_state);

				if(UWB_Err_COMMU_Send_Cancel == ResCode)
				{
					continue;
				}

				if(UWB_Err_Success_0 == ResCode)
				{
					if(UWB_MSG_Is_Null != pst_uci_state->NTFIdx)
					{
						UWB_NTF_Check(pst_uci_state);
					}

					if(false == pst_uci_state->IsInCmd)
					{
						break;
					}
				}
				else
				{
					LOG_L_S(CCC_MD,"UCI Frame Report Error !!!!");
					//set process to reset 29D5
					//break;
				}
			}
		}

		//step 6. PostProcessing
		if((false == pst_uci_state->IsInCmd) && (UWB_MSG_Is_Null != pst_uci_state->MSGIdx))
		{
			switch(pst_uci_state->MSGIdx)
			{
			case UWB_Anchor_WakeUp_RS:
				LOG_L_S(CCC_MD," UWB Anchor Wake up is Success .");
				break;
			case UWB_Timer_Sync_RS:
				LOG_L_S(CCC_MD," UWB Time [SYNC] is Success.");
				break;
			case UWB_Ranging_Session_Init_RS:
				LOG_L_S(CCC_MD," RANGING INIT is Success .");
				break;
			case UWB_Ranging_Session_App_Config_RS:
				LOG_L_S(CCC_MD," RANGING APP Config is Success .");
				break;
			case UWB_Ranging_Session_Start_RS:
				LOG_L_S(CCC_MD," RANGING [Start] is Success .");
				break;
			case UWB_Ranging_Session_Suspend_RS:
				LOG_L_S(CCC_MD," RANGING [Suspend] is Success .");
				break;
			case UWB_Ranging_Session_Recover_RS:
				LOG_L_S(CCC_MD," RANGING [Recover] is Success .");
				break;
			case UWB_Ranging_Session_Stop_RS:
				LOG_L_S(CCC_MD," RANGING [Stop] is Success .");
				break;
			case UWB_Ranging_Session_Deinit_RS:
				LOG_L_S(CCC_MD," RANGING [Deinit] is Success .");
				break;
			case UWB_Ranging_Result_Notice:
				if(pst_uci_state->RangSession.stRangingResult.ResultCnt >= 10)
				{
					RANGING_RESULT_OUTPUT(pst_uci_state->RangSession.stRangingResult.ptrResult);
				}
				break;
			case UWB_Core_Config_Setting_RS:
				LOG_L_S(CCC_MD," CORE_SET_CONFIG is Success .");
				break;
			case UWB_Hard_Reset_RS:
				LOG_L_S(CCC_MD," UWB Module Reset is Success .");
				break;
				//The notification message is sent out, either CAN or Uart, and the next step is determined by the outside.
			default:
				LOG_L_S(CCC_MD," Un defined CMD RS .");
				break;
			}
			pst_uci_state->IsInCmd = true;
			pst_uci_state->MSGIdx = UWB_MSG_Is_Null;
		}
		//step 7. Send CAN or Uart msg to BUS .

	}
	return ResCode;
}


const uint8_t Test_MSG_CAN_Ranging_Session_Setup_RQ [] = {
		0xBA,0xAB,0xAB,0xBA,					//session id
		0x00,0x01,								//uwb config id
		0x01,									//Selected_PulseShape_Combo
		0x01,									//Selected_Session_RAN_Multiplier
		0x05,									//Selected_UWB_Channel
		0x02,									//Selected_Number_Chaps_per_Slot
		0x01,									//Selected_Number_Slots_per_Round
		0x01,									//Selected_Hopping_Config_-	Bitmask
		0x00,0x00,0x00,0x00,					//STS_Index0
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//UWB_Time0
		0x00,0x00,0x00,0x00,					//HOP_Mode_Key
		0x00,									//SYNC_Code_Index
		0x00,									//RangingFrequency
		0x01,									//Number_Responders_Nodes
		0x04,									//Responder_LOC_IND

};

const uint8_t Test_MSG_CAN_Ranging_Session_Start_RQ[]={
		0xBA,0xAB,0xAB,0xBA,					//session id
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11
};

const uint8_t Test_MSG_CAN_CAN_Ranging_Session_Stop_RQ[] = {
		0x04,									//ActionType 01: suspend		02: recover		03: configurable recover		04: stop
		0xBA,0xAB,0xAB,0xBA,					//UWB_Session_Id
		0x00,0x00,0x00,0x00,					//STS_Index0
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//UWB_Time0
		0x01									//Selected_RAN_Multiplier
};



static UWBErrCode CAN_Msg_Recv(uint8_t* rcv_buf)
{
	UWBErrCode ResCode = UWB_Err_Success_0;

	return ResCode;
}

static UWBErrCode CAN_Msg_Send(uint8_t* snd_buf)
{
	UWBErrCode ResCode = UWB_Err_Success_0;

	return ResCode;
}

static void CAN_Msg_Analyze(uCAN_MSG can, st_uci_frame_state* pst)
{
	YQ_UWB_CAN_MSG index = (YQ_UWB_CAN_MSG)*can.stCanMsg.header;

	switch(index)
	{
		case CAN_UWB_Anchor_WakeUp_RQ:
													pst->UWBLocalcIndex = can.stCanMsg.body[0];
													pst->MSGIdx = UWB_Anchor_WakeUp_RQ;
													break;

		//case CAN_UWB_Anchor_WakeUp_RS:
		case CAN_Timer_Sync_RQ:
													break;
		case CAN_Ranging_Session_Setup_RQ:
													core_mm_copy(SessionSetupCANBuffer, can.stCanMsg.body,64);
													break;
		case CAN_Ranging_Session_Start_RQ:
													if(0 == core_mm_compare(SessionSetupCANBuffer,can.stCanMsg.body, 4))
													{
														//core_mm_copy(uwb_uci_send_buffer,RESPONDER_ranging_session_app_default_config,sizeof(RESPONDER_ranging_session_app_default_config));
													}
													else
													{
														pst->MSGIdx = UWB_MSG_Is_Null;
														pst->IsInCmd =false;
														*can.stCanMsg.header = CAN_Ranging_Session_Start_RS;
														can.stCanMsg.body[0] = CAN_Err_Ranging_Session_ID_Incorrect;
													}
													break;
		//case CAN_Ranging_Session_Start_RS:break;
		case CAN_Ranging_Session_Suspend_Recover_Stop_RQ:break;
		//case CAN_Ranging_Session_Suspend_Recover_Stop_RS:break;
		case CAN_Ranging_Result_Notice:break;
		case CAN_Ranging_Location_notice:break;
		case CAN_Vehicle_Status_Notice:break;
		default:
			break;
	}
}

uint32_t API_uwb_responder_ranging_task(void)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;
	st_uci_frame_state*	pst_uci_state 	= NULL;
	uint16_t			sendlens 		= 0;
	st_timer_tool_t		st_timer_tools;
	st_uwb_commu_attr_t	st_uwb_commu;

	//init
	core_mm_set((uint8_t*)&st_uci_state, 0, sizeof(st_uci_frame_state));
	pst_uci_state = &st_uci_state;

	pst_uci_state->RangSession.UCI_Rang_Session_Mode 	= 0xA0;//Default CCC mode
	pst_uci_state->MSGIdx 								= UWB_Hard_Reset_RQ;
	//pst_uci_state->ptrNoticBuffer 						= UWB_Task_buffer;//CommonBuffer
	pst_uci_state->IsInCmd = true;
	pst_uci_state->FrameState = UciFrameState_Init;

	pstTimerTools = &st_timer_tools;
	pstUWBCommu	  = &st_uwb_commu;


#if defined(UWB_RESPONDER)
	pst_uci_state->UWBDeviceTypeIsResponder = true;
#else
	pst_uci_state->UWBDeviceTypeIsResponder =
#endif



	API_uci_frame_init(pst_uci_state);			//st_uci_state.IsInCmd = true;

	while(true)
	{
		//step 1.LP --> goto sleep RTOS global value

		//step 2. CAN/UART -> recv msg RTOS global value

		//step 3. Check MSG coming data ?CAN?UART
		if(UWB_MSG_Is_Null == pst_uci_state->MSGIdx)//recv msg
		{
			CAN_Msg_Recv(CanMsg.can_buffer);
			CAN_Msg_Analyze(CanMsg, pst_uci_state);

			//Test
			//pst_uci_state->MSGIdx = UWB_Anchor_WakeUp_RQ;
		}
		else
		{
			// do nothing .
		}

		//step 4. PreProcessing
		if(true == pst_uci_state->IsInCmd)
		{
			switch(pst_uci_state->MSGIdx)
			{
		    	case UWB_Anchor_WakeUp_RQ:break;
		    	case UWB_Timer_Sync_RQ:break;
		    	case UWB_Ranging_Session_Init_RQ:break;
		    	case UWB_Ranging_Session_Start_RQ:break;
		    	case UWB_Ranging_Session_Suspend_RQ:break;
		    	case UWB_Ranging_Session_Recover_RQ:break;
		    	case UWB_Ranging_Session_Stop_RQ:break;
		    	case UWB_Ranging_Session_Deinit_RQ:break;
				default:
                    	break;
			}

		//step 5. Call the uci frame main function go in to the UWB control entry
			while(pst_uci_state->IsInCmd)//when the work flow is INComing , mean the 29d5 transcve isn't done . add by JonSong
			{
				ResCode = API_uci_frame_entry(pst_uci_state);

				if(UWB_Err_Success_0 == ResCode)
				{

				}
				else
				{
					pst_uci_state->IsInCmd = false;
					break;
				}
				//Only the nxp_uci_res_parse function can be change the direction with work flow . add by JohnSong
			}
		}

		//step 6. PostProcessing
		if(false == pst_uci_state->IsInCmd)
		{
			switch(pst_uci_state->MSGIdx)
			{
				case UWB_Anchor_WakeUp_RS:
					CAN_Msg_pkg_UWB_Anchor_WakeUp_RS(CanMsg.can_buffer, &sendlens);
					pst_uci_state->IsInCmd = true;
					//SEND_TO_CAN
					LOG_OUT_HEX("UWB_Anchor_WakeUp-> : ",CanMsg.can_buffer, sendlens);
					break;
				case UWB_Timer_Sync_RS:				break;
				case UWB_Ranging_Session_Init_RS:	break;
				case UWB_Ranging_Session_Start_RS:	break;
				case UWB_Ranging_Session_Suspend_RS: break;
				case UWB_Ranging_Session_Recover_RS:break;
				case UWB_Ranging_Session_Stop_RS:
				case UWB_Ranging_Session_Deinit_RS: break;
				case UWB_Ranging_Result_Notice: break;
					break;
					//The notification message is sent out, either CAN or Uart, and the next step is determined by the outside.
				default:
					break;
			}

		}
		//step 7. Send CAN or Uart msg to BUS .

	}
	return ResCode;
}



static void CAN_Msg_pkg_UWB_Anchor_WakeUp_RS(uint8_t* buf, uint16_t* buflens)
{
	uint16_t lens = 0;
	core_mm_copy(buf, UWB_Anchor_SN, sizeof(UWB_Anchor_SN));
	lens +=10;
	buf+=lens;

	if(UWB_Err_Success_0 == st_uci_state.OpCode)
	{
		buf[lens++] = 0x00;
	}
	else
	{
		buf[lens++] = 0x03;
	}
	lens++;

	buf[lens++] = UWB_LOC_IND;
	buf[lens++] = 0x01;

	core_mm_copy(buf, ArrDeviceCapabilityParames + OFST_DCP_A5_SUPPORTED_UWB_CONFIG_ID + 1, ArrDeviceCapabilityParames[OFST_DCP_A5_SUPPORTED_UWB_CONFIG_ID+1]+1);
	lens+= ArrDeviceCapabilityParames[OFST_DCP_A5_SUPPORTED_UWB_CONFIG_ID+1]+1;
	buf+= lens;

	core_mm_copy(buf,ArrDeviceCapabilityParames + OFST_DCP_A6_SUPPORTED_PULSESHAPE_COMBO + 1, ArrDeviceCapabilityParames[OFST_DCP_A6_SUPPORTED_PULSESHAPE_COMBO]+1);
	lens+= ArrDeviceCapabilityParames[OFST_DCP_A6_SUPPORTED_PULSESHAPE_COMBO]+1;
	buf+= lens;

	buf[lens++] = ArrDeviceCapabilityParames[OFST_DCP_A3_CHANNEL_BITMASK+2];

	buf[lens++] = 1;//Supported_RAN_Multiplier_MIN // How to get it ? add by JohnSong

	buf[lens++] = ArrDeviceCapabilityParames[OFST_DCP_A0_SLOT_BITMASK+2];

	buf[lens++] = ArrDeviceCapabilityParames[OFST_DCP_A1_SYNC_CODE_INDEX_BITMASK+2];

	buf[lens++] = ArrDeviceCapabilityParames[OFST_DCP_A2_HOPPING_CONFIG_BITMASK];

	*buflens = lens;
}
#endif

#if 0
abandoned code


#endif
