/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "Flash_Adapter.h"
#include "EM000101.h"

u32 flexNVM_mem_write(uint32_t address, uint32_t length, const uint8_t *buffer);
u32 flexNVM_mem_erase(uint32_t address, uint32_t length);

u8 hw_flash_write_for_ble_area(u16 id, u8 *p_data,u8 len);
u8 hw_flash_read_from_ble_area(u16 id, u8 *p_data,u8 len);

u8 hw_flash_write_for_nfc_area(u16 id, u8 *p_data,u8 len);
u8 hw_flash_read_from_nfc_area(u16 id, u8 *p_data,u8 len);