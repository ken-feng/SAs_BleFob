/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2019-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

#define APP_VERSION_H	0x05
#define APP_VERSION_L	0x01


#define FEATRUE_UART_USE_PTC67  1   /*1：PTC67  0:串口日志使用PTA17 18*/
#define gKeyBoardSupported_d                0
#define gCccL2Cap_d     1/*支持ccc l2cap*/

#define gCcc_SPSM_c  0x004F

#define FIT_FOR_TEST       
#define gDCDC_3p3V_Output_d		1
// #define gKeyEventNotificationMode_d         gKbdEventShortLongPressMode_c
 /* Defines the number of required keys for the keyboard module */
#define gKBD_KeysCount_c        0//2
#define FIT_SUPPORT

#define DISABLE_WDOG    0  /*0: 开启开门狗   1: 关闭开门狗*/

#define APP_FOR_AUTO_TEST   /*自动化测试打开*/

#define APP_DEBUG_MODE

#define USE_PIT_TIMER       0


#define PTB0_IRQ_PIN		1/*	1: PTB0   0: PTC16*/

//Modify (Ken):FOB-V0501 NO.1 -20231225
#define	__FIT_Aeon_H

//#define CAN_DEBUG_MODE

//#define EEPROM_BASE_ADDRESS             0x78000

#define BL_APP_VECTOR_TABLE_ADDRESS     0x10000u
#define APP_VECTOR_TABLE                ((uint32_t *)BL_APP_VECTOR_TABLE_ADDRESS)
#define kBootloaderConfigAreaAddress    ((uint32_t)(APP_VECTOR_TABLE) - 0x800u)

/*!
 *  Application specific configuration file only
 *  Board Specific Configuration shall be added to board.h file directly such as :
 *  - Number of button on the board,
 *  - Number of LEDs,
 *  - etc...
 */
/*! *********************************************************************************
 *     Board Configuration
 ********************************************************************************** */


/* Specifies the number of required LEDs for the LED module */
#define gLEDsOnTargetBoardCnt_c 0//4

/* Specifies if the LED operation is inverted. LED On = GPIO Set */
#define gLED_InvertedMode_d     0

#define gSerialManagerMaxInterfaces_c       0


/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
 /*! Maximum number of connections supported for this application */
#define gAppMaxConnections_c           3
/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d   1

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d   1

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d        0

#define gPasskeyValue_c    999999



/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0
/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    1

/* Defines Size for Serial Manager Task*/
#define gSerialTaskStackSize_c  500

/* Defines Size for TMR Task*/
#define gTmrTaskStackSize_c     600

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define AppPoolsDetails_c \
         _block_size_  88  _number_of_blocks_    7 _eol_  \
         _block_size_ 248  _number_of_blocks_    2 _eol_  \
         _block_size_ 312  _number_of_blocks_    2 _eol_  \
         _block_size_ 392  _number_of_blocks_    1 _eol_

/* Defines number of timers needed by the application */
#if gRepeatedAttempts_d
#define gTmrApplicationTimers_c         15
#else
#define gTmrApplicationTimers_c         14
#endif

/* Set this define TRUE if the PIT frequency is an integer number of MHZ */
#define gTMR_PIT_FreqMultipleOfMHZ_d    0

/* Enables / Disables the precision timers platform component */
#define gTimestamp_Enabled_d            0

/* Check Low Power Timer */
#define cPWR_CheckLowPowerTimers        1

/*! Enable/Disable Low Power Timer */
#define gTMR_EnableLowPowerTimers_d     1

/* Enable/Disable PowerDown functionality in PwrLib */
#define cPWR_UsePowerDownMode           0

/* Enable/Disable BLE Link Layer DSM */
#define cPWR_BLE_LL_Enable              1

/* Default Power Mode: MCU=VLLS3, LL=IDLE*/
#define cPWR_DeepSleepMode              5

/* Enable/Disable MCU Sleep During BLE Events */
#define cMCU_SleepDuringBleEvents       1

/* Default deep sleep duration in ms */
#define cPWR_DeepSleepDurationMs        30000

/* Number of slots(625us) before the wake up instant before which the hardware needs to exit from deep sleep mode. */
#define cPWR_BLE_LL_OffsetToWakeupInstant 3

/* Application Connection sleep mode */
#define gAppDeepSleepMode_c             8

/* Disable unused LowPower modes */
#define cPWR_EnableDeepSleepMode_1      1
#define cPWR_EnableDeepSleepMode_2      0
#define cPWR_EnableDeepSleepMode_3      0
#define cPWR_EnableDeepSleepMode_4      0
#define cPWR_EnableDeepSleepMode_5      1
#define cPWR_EnableDeepSleepMode_7      0
#define cPWR_EnableDeepSleepMode_8      1

/* Enables / Disables MWS coexistence */
#define gMWS_UseCoexistence_d           0

/*! Enable / Disable the DCDC SW component
      Shall be enabled in buck mode to have optimized DCDC settings during active and lowpower modes
      Not required in DCDC bypass mode                                                   */
#define gDCDC_Enabled_d                 1

/*! Default DCDC Mode used by the application gDCDC_Buck_c or gDCDC_Bypass_c
      Warning :
     - in Buck mode , DCDC module shall be enabled (gDCDC_Enabled_d= 1)
     - in bypass mode , It is advised to disable DCDC module  (not needed)   */
#define APP_DCDC_MODE                   gDCDC_Buck_c

/*! Use Pswitch Mode when going to mode 4 (DCDC switched OFF) or RAM off mode
       instead of VLLS1 (DCDC must be in buck mode)
       In this case, DCDC should be set to manual startup (J38 in position 1-2)
       In manual mode, DCDC needs to be started with PSWITCH button (SW4)
       and you can only wake up the device with this button */
//#define gPWR_UsePswitchMode_d          1

/*! Default DCDC Battery Level Monitor interval in Ms - Set the flag to higher value, you are unlikely requested
       having Vbat monitor every 6 seconds. typical is 60 seconds.
    Vbat Monitoring is done in DCDC buck mode only on every wakeup. However, if the device remains active,
       then the battery measurement is done on periodic timer whose Interval value is given by this define     */
#define APP_DCDC_VBAT_MONITOR_INTERVAL  600000

/*! Store RNG seed to allow faster wakeup time from RAM OFF
    Seed location value is stored in HW parameters and then retrieved from their on each wakeup from
      POR, Pswitch, VLLS0, VLLS1
     Warning : The user may want to clear the Seed from time to time for security reason. In this case,
       an erase of the rngSeed field in gHardwareParameters is required so the SW could generate a new Seed on next
       reboot (will increase the wakeup time)*/
//#define gRngSeedHwParamStorage_d       
/*! Enable XCVR calibration storage in Flash
    Store the XCVR calibration in HW parameters to allow faster wakeup from lowpower VLLS2/3
    Not used on wakeup from POR, Pswitch, VLLS0, VLLS1
    Clear the Radio calibration by erasing the xcvrCal field in gHardwareParameters.*/
#define gControllerPreserveXcvrDacTrimValue_d     1

/*! Enable the SWD pins to be managed into low-power */
#define gBoard_ManageSwdPinsInLowPower_d    1

/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
/* Defines the RTOS used */
#define FSL_RTOS_FREE_RTOS      1

/* Defines number of OS events used */
#define osNumberOfEvents        7

/* Defines main task stack size */
#define gMainThreadStackSize_c  1024

/*! Defines Size for Idle Task  (minimum 572 is required) */
#define gAppIdleTaskStackSize_c         600

/* Defines total heap size used by the OS */
#define gTotalHeapSize_c        0x4000/* Warm-boot sequence will use the default stack which is used by ISRs on FreeRTOS */

/* Defines if RTOS systicks are used and managed by the Low-power module */
#define PWR_EnableRtosSysticks           0//1

/* when RTOS systicks are used , Idle hook and freertos tickless mode shall be enabled for lowpower */
#if defined(PWR_EnableRtosSysticks) && (PWR_EnableRtosSysticks == 1)
#define configUSE_IDLE_HOOK              1
#define configUSE_TICKLESS_IDLE          1

/* Use direct call of handle keys function */           
#define cPWR_BleAppHandleKeyDirectCall_d    0
/*! On wakeup from POR or lowpower modes where the 32KHz oscilator is disabled (Pswitch, VLLS0, eventually VLLS1), the
       32KHz crystal takes roughly 200-300 ms to set up. In this case, there are some issues when going to lowpower
       while the Link layer / Radio has already been initialized, the power consumption of the chip remains high.
       In addition to this, the Linklayer/Radio will not wakeup correctly in case 32KHz clock is not stable enough.
    By setting this Macro, the lowpower module will go to lowpower (LLS) immediatly after wakeup from
       POR, Pswitch mode, VLLS0 or VLLS1 (if 32K was disabled) until the 32KHz oscillator is running correctly.
       The first Advertisment event is delayed, but it ensures the device does not consume too high and
       Linklayer/Radio does not encounter any issue can because of this
    Note: In VLLS1, the 32khz is disabled when no LPTMR are running   */
/* Wait for 32KHz oscillator to be up and stable */
#define PWR_WaitFor32kOscRestart    1

extern void vPortSuppressTicksAndSleep( unsigned int xExpectedIdleTime );
#define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime ) vPortSuppressTicksAndSleep(xExpectedIdleTime)
#endif

#if defined(configUSE_IDLE_HOOK) && (configUSE_IDLE_HOOK == 1)
/* Freertos task stack shall be increased when App Idle task is not used (in ApplMain.c)
   Requires 600 bytes, in words. */
#define configMINIMAL_STACK_SIZE         ((unsigned short)150)
#endif

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */

/* Time between the beginning of two consecutive advertising PDU's */
#define mcAdvertisingPacketInterval_c     0x02    /* 1.25 msec */

/* Offset to the first instant register. */
#define mcOffsetToFirstInstant_c        0x00      /* 625usec */


/*! *********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/
/*  ble_ll_config.h file lists the parameters with their default values. User can override
 *    the parameter here by defining the parameter to a user defined value. */

/*
 * Specific configuration of LL pools by block size and number of blocks for this application.
 * Optimized using the MEM_OPTIMIZE_BUFFER_POOL feature in MemManager,
 * we find that the most optimized combination for LL buffers.
 *
 * If LlPoolsDetails_c is not defined, default LL buffer configuration in app_preinclude_common.h
 * will be applied.
 */

/* Include common configuration file and board configuration file */
#include "app_preinclude_common.h"
#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
