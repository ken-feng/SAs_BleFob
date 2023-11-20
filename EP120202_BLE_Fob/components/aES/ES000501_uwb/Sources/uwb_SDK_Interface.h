#ifndef SOURCES_UWB_SDK_INTERFACE_H_
#define SOURCES_UWB_SDK_INTERFACE_H_

#define _UWB_DEBUG

#include <stdint.h>
#include <stdbool.h>

#include "./uwb_common_def.h"
#if defined(UWB_RESPONDER)
	#include "./UWBResponder/uwb_responder_api.h"
#elif defined(UWB_INITIATOR)
#include "./UWBInitiator/uwb_initiator_api.h"
#else
#include " you need make a choose ."
#endif
/* ------------------------------------------------------------------------------------------------------------------*/
// UQ UWB SDK 协议栈
//#define stat_Success STATUS_SUCCESS
/* ------------------------------------------------------------------------------------------------------------------*/

typedef enum enumAnchorRegion_t
{
	AR_LeftFront 	=0x01,	//：左前；
	AR_RightFront 	=0x02,	//：右前；
	AR_Centre		=0x04,	//：正中；
	AR_Leftback		=0x08,	//：左后；
	AR_Rightback	=0x10,	//：右后，
	AR_RFU1			=0x20,	//：RFU,
	AR_RFU2			=0x40,	//：RFU,
	//AR_RFU3			=0x80,	//：RFU,
	AR_Fob			=0x80,
}E_AnchorRegion;

typedef enum enumUQUWBCANMsg_t
{
	UQCANMSG_Wakeup_RS = 0,
	UQCANMSG_TimeSync_RS = 1,
	UQCANMSG_RANGING_Start_RS = 2,
	UQCANMSG_RANGING_Result_RS = 3,

}E_UQUWBCANMsg;

typedef struct struct_UWB_SDK_Interface_t
{
	//------------------------------------------------------
	//SDK API
	ST_UWBProtocol					stProtocol;
	E_AnchorRegion					eRegion;

	/*<! device opt*/
	fp_UQ_Device_Init_t				fpUQDeviceInit;					/*!< device init */
	fp_UQ_Device_Reset_t			fpUQDeviceReset;				/*!< hardware reset */
	
	fp_UQ_Get_Caps_t				fpUQGetCaps;					/*!< From Flash		|	get uwb device CCC protocol caps */
	fp_UQ_Set_Caps_t				fpUQSetCaps;					/*!< Set to Flash	|	set uwb device CCC protocol caps */

	/*<! uwb opt*/
	fp_UQ_Anchor_Wakup_t			fpUQAnchorWakup;
	fp_UQ_Time_Sync_t				fpUQTimeSync;
	fp_UQ_RangingSessionSetup_t		fpUQRangingSessionSetup;
	fp_UQ_RangingCtrl_t				fpUQRangingCtrl;
	fp_UQ_RangingResult_t			fpUQRangingResult;
	fp_UQ_RangingNTFCache_t			fpUQRangingNTFCache;
	fp_UQ_CustCMD_t					fpUQCustCMD;
	fp_UQ_SetNTFCacheFlag_t			fpUQSetNTFCacheFlag;

	//------------------------------------------------------
	//Foreign API
	fp_UQ_MSGSend_t					fpUQSendMSG;
	fp_UQ_ConfigWrite_t				fpUQConfigW;
	fp_UQ_ConfigRead_t				fpUQConfigR;

}ST_UWBSDKInterface;

extern ST_UWBSDKInterface 	stUWBSDK;
extern ST_UWBSource 		stSource;

//----------------------------------------------------------
//Debug code
int UQ_UWB_SDK_Interface_init(ST_UWBSDKInterface* pstSDK);

//extern int Debug_Main(void);
#endif /* SOURCES_UWB_SDK_INTERFACE_H_ */

#if 0 
abandoned code


#endif
