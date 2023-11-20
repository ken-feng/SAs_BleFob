/*
 * uwb_initiator_api.h
 *
 *  Created on: 2022��6��30��
 *      Author: JohnSong
 */

#ifndef SOURCES_UWBINITIATOR_UWB_INITIATOR_API_H_
#define SOURCES_UWBINITIATOR_UWB_INITIATOR_API_H_
#include <components/aES/ES000501_uwb/Sources/uwb_common_def.h>

#if defined(UWB_INITIATOR)

extern E_UWBErrCode API_UWB_Initiator_Init(ST_UWBSource* pst_uwb_source);
extern E_UWBErrCode API_UWB_Initiator_WorkProcedure(ST_UWBSource* pst_uwb_source);

#endif // @defined(UWB_INITIATOR)

#endif /* SOURCES_UWBINITIATOR_UWB_INITIATOR_API_H_ */


#if 0
abandoned code
//static st_uci_frame_state		st_uci_state;
#endif
