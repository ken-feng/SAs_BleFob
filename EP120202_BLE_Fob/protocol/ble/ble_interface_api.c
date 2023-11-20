
#include "gap_interface.h"
#include "ble_constants.h"
#include "otap_client_att.h"

//extern appPeerInfo_t* curPeerInformation;

uint32_t Ble_SendData(uint32_t uuid, uint8_t *pdata, uint16_t length)
{
	if(uuid == 0xFFE1)
	{
//		BleApp_NotifyDKData(curPeerInformation->deviceId, pdata,length);
	}
	else if(uuid == 0xFFE3)
	{
//		BleApp_NotifyPersionalData(curPeerInformation->deviceId, pdata,length);
	}
	return 0;
}
