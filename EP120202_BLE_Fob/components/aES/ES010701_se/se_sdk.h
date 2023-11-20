#ifndef _SE_SDK_H_
#define _SE_SDK_H_
#include "se_sdk.h"
#include "EM000101.h"

void se_sdk_init(void);
u8 se_sdk_send_apdu(u8* inData, u16 length);
u16 se_sdk_recv_apdu(u8* outData);
u8 se_sdk_recv_apdu_call_back(u8* outData, u16 length);


#endif
