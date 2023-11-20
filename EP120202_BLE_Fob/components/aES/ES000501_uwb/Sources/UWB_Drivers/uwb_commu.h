
#ifndef SOURCES_UWB_DRIVERS_UWB_COMMU_H_
#define SOURCES_UWB_DRIVERS_UWB_COMMU_H_

#include "../uwb_common_def.h"

#if defined(NXP_UCI_FRAME)
	#define	UWB_MAX_FRAME_LENGTH	UCISW_MAX_FRAME_LENGTH
#elif defined(NXP_RCI_FRAME)
	#define	UWB_MAX_FRAME_LENGTH	RCISW_MAX_FRAME_LENGTH
#else
	#define ?
#endif

// 1 Byte for Response data padding header
#define SPI_BUF_MAX_LENS	(UWB_MAX_FRAME_LENGTH)
#define SPI_SEND_BUF_MAX_LENS	(UWB_MAX_FRAME_LENGTH)
#define SPI_RECV_BUF_MAX_LENS	(UWB_MAX_FRAME_LENGTH)




/* ------------------------------------------------------------------------------------------------------------------*/
/**< UWB moudule communication Control*/
/* ------------------------------------------------------------------------------------------------------------------*/
typedef enum UCI_CommuState_t
{
    UciCommuState_Idle 						= 0x00u,
    UciCommuState_WaitReady 				= 0x01u,
	UciCommuState_WaitTransferHeader 		= 0x02u,
    UciCommuState_Start 					= 0x03u,
    UciCommuState_WaitTransferComplete 		= 0x04u,
    UciCommuState_Cancelled					= 0x05u,
    UciCommuState_Error 					= 0x06u,
    UciCommuState_Undefined 				= 0xFFu,
}E_UCICommuState;



//extern ST_UWBCommuAttr*     pstUWBCommu;
extern uint32_t 			API_UWB_Commu_Module_Init(ST_UWBCommuAttr* 	    pst_uwb_commu);
extern uint32_t 			API_UWB_Commu_Module_Deinit(ST_UWBCommuAttr* 	pst_uwb_commu);




#endif /* SOURCES_UWB_DRIVERS_UWB_COMMU_H_ */
#if 0 
abandoned code


#endif
