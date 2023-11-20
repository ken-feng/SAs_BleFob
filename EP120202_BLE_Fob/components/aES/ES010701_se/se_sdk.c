#include "se_sdk.h"
#include "se_apdu_api.h"
#include "RTE_se.h"
#include "EM000401.h"

#define TMP_BUFFER_MAX_SIZE     270
u8 seTmpBuffer[TMP_BUFFER_MAX_SIZE];
u16 seTmpLength = 0;

void se_sdk_init(void)
{
    api_power_on_init();
    core_mm_set(seTmpBuffer,0x00,TMP_BUFFER_MAX_SIZE);
}

u8 se_sdk_send_apdu(u8* inData, u16 length)
{
	LOG_L_S_HEX(CCC_MD,"APDU Send: ",inData,length);
#if 1  
    int ret;
    ret = api_apdu_transceive(inData,length,seTmpBuffer,TMP_BUFFER_MAX_SIZE);
    if ((ret < 0U)||(ret > 270U))
    {
	//         seTmpLength = se_sdk_recv_apdu(seTmpBuffer);
    	seTmpLength = 0;
    }
    else
	{
    	seTmpLength = ret;
	}
#endif    
    return 1;
}


u16 se_sdk_recv_apdu(u8* outData)
{
    core_mm_copy(outData,seTmpBuffer,seTmpLength);
    LOG_L_S_HEX(CCC_MD,"APDU Recv: ",outData,seTmpLength);
    return seTmpLength;
}

u8 se_sdk_recv_apdu_call_back(u8* outData, u16 length)
{
    RTE_SE_RECV_CALL_BACK(outData,length);
    return 1;
}
