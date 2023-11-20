/*
 * uwb_initiator_api.c
 *
 *  Created on: 2022-6-30
 *      Author: JohnSong
 */

#include "../uwb_common_def.h"
#if defined(UWB_INITIATOR)
#include "../UWBinitiator/uwb_initiator_api.h"
#include "../UWB_UCI_Frame/nxp_uci_cmd.h"
//#include "../aEM/EM00040101_log/EM000401.h"
//#include "../UWB_UCI_Frame/nxp_uci_cmd_param_table.h"

E_UWBErrCode API_UWB_Initiator_Init(ST_UWBSource* pst_uwb_source)
{
	E_UWBErrCode			ResCode 		= UWB_Err_Success_0;

	//core_mm_set((uint8_t*)&pst_uwb_source->stUCIState, 		0, sizeof(ST_UCIFrameState));
	//core_mm_set((uint8_t*)&pst_uwb_source->stUCIState.stTimerTools, 	0, sizeof(ST_TimerTools));
	//core_mm_set((uint8_t*)&pst_uwb_source->stUCIState.stUWBCommu, 		0, sizeof(ST_UWBCommuAttr));

	pst_uwb_source->stUCIState.stSession.u8UCIRangSessionMode 	= 0xA0;//Default CCC mode
	pst_uwb_source->stUCIState.eMSGIdx 							= UWB_Hard_Reset_RQ;
	pst_uwb_source->stUCIState.bIsInCmd 						= true;
	pst_uwb_source->stUCIState.eState 							= UciFrameState_Init;
	pst_uwb_source->stUCIState.bIsResponderUWBDeviceType		= false;
	API_UCI_Frame_Init(&pst_uwb_source->stUCIState);				//st_uci_state.IsInCmd = true;

	return ResCode;
}

E_UWBErrCode API_UWB_Initiator_WorkProcedure(ST_UWBSource* pst_uwb_source)
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
				LOG_L_S(CCC_MD," UWB Anchor Wake up is Success .\r\n");
				break;
			case UWB_Timer_Sync_RS:
				//LOG_L_S(CCC_MD," UWB Time [SYNC] is Success.");
				break;
			case UWB_Ranging_Session_Init_RS:
				LOG_L_S(CCC_MD," RANGING INIT is Success .\r\n");
				break;
			case UWB_Ranging_Session_App_Config_RS:
				LOG_L_S(CCC_MD," RANGING APP Config is Success .\r\n");
				break;
			case UWB_Ranging_Session_Start_RS:
				LOG_L_S(CCC_MD," RANGING [Start] is Success .\r\n");
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
				//if(pst_uwb_source->stUCIState.stSession.stRangingResult.u8ResultCnt >= 10)
				//{

					//LOG_L_S_HEX(CCC_MD,"FOB",&pst_uwb_source->stUCIState.stSession.u32ResponderBitMap, 4);
					//RANGING_RESULT_OUTPUT(pst->st_uci_state.RangSession.stRangingResult.ptrResult);
				//}
				break;
			case UWB_Core_Config_Setting_RS:
				//LOG_L_S(CCC_MD," CORE_SET_CONFIG is Success .");
				break;
			case UWB_Hard_Reset_RS:
				//LOG_L_S(CCC_MD," UWB Module Reset is Success .");
				break;
			case UWB_Core_Get_Device_Info_RS:
				LOG_S_HEX("UWB Version : ", pst_uwb_source->stUCIState.stUCIRecv.pbufDatBody, pst_uwb_source->stUCIState.stUCIRecv.u16DatBodyLens);
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
#endif //@denfined UWB_INITIATOR

#if 0
abandoned code

const uint8_t TEST_CMD_SEQU_1_REST1 [] 				= {0x20, 0x00, 0x00, 0x01, 0x00};
const uint8_t TEST_CMD_SEQU_4_SET_CORE_CONFIG_1 [] 	= {0x20, 0x04, 0x00, 0x04, 0x01, 0x01, 0x01, 0x00};
const uint8_t TEST_CMD_SEQU_4_SET_CORE_CONFIG_2 []	= {0x20, 0x04, 0x00, 0x05, 0x01, 0xEA, 0x02, 0x10, 0x27};
const uint8_t TEST_CMD_SEQU_5_INIT_SESSION[] 		= {0x21, 0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0xA0};
const uint8_t TEST_CMD_SEQU_6_APP_SESSION_CONFIG[]	= {0x21,0x03,0x00,0x91,0x03,0x00,0x00,0x00,0x19,0x11,0x01,0x01,0x04,0x01,0x09,0x05,0x01,0x05,0x14,0x01,0x0A,0x0A,0x04,0x00,0x00,0x00,0x00,0xE9,0x12,0x04,0x00,0x49,0x43,0x54,0x53,0x53,0x54,0x41,0x54,0x49,0x43,0x54,0x53,0x53,0x54,0x41,0x54,0xE9,0x12,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x49,0x43,0x54,0x53,0x53,0x54,0x41,0x54,0x06,0x02,0x00,0x00,0x08,0x02,0xB0,0x04,0x09,0x04,0x60,0x00,0x00,0x00,0xF2,0x01,0x14,0x2A,0x02,0x00,0x00,0x2C,0x01,0x00,0x15,0x01,0x00,0x1B,0x01,0x0C,0x23,0x01,0x03,0xA0,0x10,0x73,0x05,0x8F,0xE5,0x50,0x3C,0x1D,0x4A,0x17,0x54,0x1D,0x51,0x21,0xBB,0x7F,0x9E,0x32,0x02,0xFF,0xFF,0xA4,0x02,0x00,0x00,0xA5,0x01,0x11,0xA6,0x02,0xD0,0x02,0xF0,0x01,0x00,0xFB,0x04,0x02,0x03,0x12,0x13,0xFD,0x02,0xFF,0x01,0xF9,0x01,0x01};
const uint8_t TEST_CMD_SEQU_7_START_RANGING[] 		= {0x22, 0x00, 0x00, 0x04, 0x03, 0x00, 0x00, 0x00};

st_uwb_rang_ctrl_t	st_uwb_ranging;
UWBErrCode API_uwb_initiator_ranging_task(void)
{
	UWBErrCode			ResCode 		= UWB_Err_Success_0;
	uint16_t			sendlens 		= 0;
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

		if((UWB_MSG_Is_Null == pst_uci_state->MSGIdx)&&(seq_cnt<=6))//recv msg
		{
			switch(seq_cnt)
			{
				case 0:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_1_REST1, sizeof(TEST_CMD_SEQU_1_REST1));
						pst_uci_state->MSGIdx = UWB_Hard_Reset_RQ;
						pst_uci_state->IsInCmd = true;
						seq_cnt ++;
						break;
				case 1:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_1_REST1, sizeof(TEST_CMD_SEQU_1_REST1));
						pst_uci_state->MSGIdx = UWB_Hard_Reset_RQ;
						pst_uci_state->IsInCmd = true;
						seq_cnt ++;
						break;
				case 2: core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_4_SET_CORE_CONFIG_1, sizeof(TEST_CMD_SEQU_4_SET_CORE_CONFIG_1));
						pst_uci_state->MSGIdx = UWB_Core_Config_Setting_RQ;
						pst_uci_state->IsInCmd = true;
						st_uci_send.unLens = sizeof(TEST_CMD_SEQU_4_SET_CORE_CONFIG_1) -4 ;
						seq_cnt ++;
						break;
				case 3:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_4_SET_CORE_CONFIG_2, sizeof(TEST_CMD_SEQU_4_SET_CORE_CONFIG_2));
						st_uci_send.unLens = sizeof(TEST_CMD_SEQU_4_SET_CORE_CONFIG_2) -4 ;
						pst_uci_state->MSGIdx = UWB_Core_Config_Setting_RQ;
						pst_uci_state->IsInCmd = true;
						seq_cnt ++;
						break;
				case 4:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_5_INIT_SESSION, sizeof(TEST_CMD_SEQU_5_INIT_SESSION));
						st_uci_send.unLens = sizeof(TEST_CMD_SEQU_5_INIT_SESSION) -4;
						pst_uci_state->RangSession.CurrSessionID = 3;
						pst_uci_state->RangSession.UCI_Rang_Session_Mode = TEST_CMD_SEQU_5_INIT_SESSION[8];
						pst_uci_state->MSGIdx = UWB_Ranging_Session_Init_RQ;
						pst_uci_state->IsInCmd = true;
						seq_cnt ++;
						break;
				case 5:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_6_APP_SESSION_CONFIG, sizeof(TEST_CMD_SEQU_6_APP_SESSION_CONFIG));
						st_uci_send.unLens = sizeof(TEST_CMD_SEQU_6_APP_SESSION_CONFIG) -4;
						pst_uci_state->RangSession.CurrSessionID = 3;
						pst_uci_state->MSGIdx = UWB_Ranging_Session_App_Config_RQ;
						pst_uci_state->IsInCmd = true;
						seq_cnt ++;
						break;
				case 6:	core_mm_copy(st_uci_send.pDatHeader, TEST_CMD_SEQU_7_START_RANGING, sizeof(TEST_CMD_SEQU_7_START_RANGING));
						st_uci_send.unLens = sizeof(TEST_CMD_SEQU_7_START_RANGING) -4;
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

#endif
