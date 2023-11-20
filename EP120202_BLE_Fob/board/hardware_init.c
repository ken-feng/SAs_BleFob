/*
 * Copyright 2019-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
/*${header:start}*/
#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "PWR_Interface.h"
/*${header:end}*/

/*${function:start}*/
void BOARD_InitHardware(void)
{
    BOARD_BootClockRUN();

    /* Configure the serial wakeup pins. It shall be done before PWRLib_init call */
    // PWR_SetWakeupPins(BOARD_WkupPin_PTC4_c_FallingDetect);
    // PWR_SetWakeupPins(BOARD_WkupPin_PTC16_c_FallingDetect);
    // PWR_SetWakeupPins(BOARD_WkupPin_PTC2_c_FallingDetect);

}
/*${function:end}*/
