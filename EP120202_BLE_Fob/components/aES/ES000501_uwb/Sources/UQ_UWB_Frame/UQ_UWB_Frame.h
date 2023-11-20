/*
 * UQ_UWB_Frame.h
 *
 *  Created on: 2022年8月15日
 *      Author: JohnSong
 */

#ifndef SOURCES_UQ_UWB_FRAME_UQ_UWB_FRAME_H_
#define SOURCES_UQ_UWB_FRAME_UQ_UWB_FRAME_H_
#include <components/aES/ES000501_uwb/Sources/uwb_common_def.h>



#if defined(UWB_RESPONDER)
#include <components/aES/ES000501_uwb/Sources/UWBResponder/uwb_responder_api.h>
E_UWBErrCode UQUWBAnchorInit(ST_UWBSource* pst_uwb_source);
E_UWBErrCode UQUWBAnchorReset(ST_UWBSource* pst_uwb_source);
E_UWBErrCode UQUWBAnchorGetCapblity(ST_UWBSource* pst_uwb_source);
E_UWBErrCode UQUWBAnchorRangingSetup(ST_UWBSource* pst_uwb_source, uint8_t* cmd, uint16_t* lens);
E_UWBErrCode UQUWBAnchorRangingControl(E_RangingOPType type, ST_UWBSource* pst_uwb_source, uint8_t* cmd, uint16_t* lens);//input param cmdtype:0- start 1-suspend 2-close ranging session 3-recover
E_UWBErrCode UQUWBAnchorRangingDataNTF(ST_UWBSource* pst_uwb_source);
#elif defined(UWB_INITIATOR)
#include <components/aES/ES000501_uwb/Sources/UWBInitiator/uwb_initiator_api.h>
E_UWBErrCode UQUWBFobInit(ST_UWBSource* pst_uwb_source);
E_UWBErrCode UQUWBFobReset(ST_UWBSource* pst_uwb_source);
E_UWBErrCode UQUWBFobGetCapblity(ST_UWBSource* pst_uwb_source);
E_UWBErrCode UQUWBFobRangingSetup(ST_UWBSource* pst_uwb_source, uint8_t* cmd, uint16_t* lens);
E_UWBErrCode UQUWBFobRangingControl(E_RangingOPType type, ST_UWBSource* pst_uwb_source, uint8_t* cmd, uint16_t* lens);//input param cmdtype:0- start 1-suspend 2-close ranging session 3-recover
E_UWBErrCode UQUWBFobRangingDataNTF(ST_UWBSource* pst_uwb_source);
#else
?
#endif //



E_UWBErrCode UQUWBCustCMD(ST_CustCmd* pst_cust_cmd);

#endif /* SOURCES_UQ_UWB_FRAME_UQ_UWB_FRAME_H_ */
#if 0
abandoned code


#endif
