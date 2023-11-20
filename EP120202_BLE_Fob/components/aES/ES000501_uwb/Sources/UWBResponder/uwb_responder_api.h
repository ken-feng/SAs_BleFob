/*
 * uwb_responder_api.h
 *
 *  Created on: 2022��6��30��
 *      Author: JohnSong
 */

#ifndef SOURCES_UWBRESPONDER_UWB_RESPONDER_API_H_
#define SOURCES_UWBRESPONDER_UWB_RESPONDER_API_H_

#include "../uwb_common_def.h"

#if defined(UWB_RESPONDER)

extern E_UWBErrCode API_UWB_Responder_Init (ST_UWBSource* pst_uwb_source);
extern E_UWBErrCode API_UWB_Responder_WorkProcedure(ST_UWBSource* pst_uwb_source);

#endif /* UWB_RESPONDER */
#endif /* SOURCES_UWBRESPONDER_UWB_RESPONDER_API_H_ */

#if 0 
abandoned code


#endif
