#ifndef BLE_INTERFACE_H
#define BLE_INTERFACE_H

#include "ble_constants.h"

uint32_t Ble_SendData(uint32_t uuid, uint8_t *pdata, uint16_t length);

#endif
