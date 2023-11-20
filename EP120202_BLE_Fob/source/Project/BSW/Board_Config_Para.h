//===========================================================================//
//文件说明：芯片KW36的驱动配置相关参数定义
//基于环境MCUXpresso IDE V11.2.1，版本SDK_2.2.5_FRDM-KW36


#ifndef _BOARD_CFG_PARA_H
#define _BOARD_CFG_PARA_H


//文件包含
/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
//driver
#include "fsl_port.h"
#include "GPIO_Adapter.h"
#include "fsl_flexcan.h"
#include "fsl_dspi.h"
#include "board.h"
#include "pin_mux.h"
#include "fsl_pit.h"
#include "SerialManager.h"
//#include "fsl_debug_console.h"
#include "SecLib.h"
#include "fsl_lin.h"
#include "fsl_rtc.h"
#include "FreeRTOS.h"
#include "Flash_Adapter.h"
#include "FunctionLib.h"
//#include "fsl_lpuart.h"
#include "fsl_lin_lpuart.h"
#include "fsl_adc16.h"
//#include "wireless_uart.h"
#include "Flash_Adapter.h"
#include "task.h"
#include "PWR_Interface.h"
//===========================================================================//
//宏定义
// //SPI
// #define DSPI_MASTER_CLK_SRC DSPI0_CLK_SRC
// #define DSPI_MASTER_CLK_FREQ CLOCK_GetFreq(DSPI0_CLK_SRC)
// #define EXAMPLE_DSPI_MASTER_PCS_FOR_INIT kDSPI_Pcs0
// #define EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER kDSPI_MasterPcs0
// #define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 4M */
// //#define TRANSFER_BAUDRATE 1000000U /*! Transfer baudrate - 4M */


//PIT timer
#define KW36_Timer_ISR PIT_IRQHandler
#define PIT_IRQ_ID PIT_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)


//CAN
#define RX_MESSAGE_BUFFER_NUM (8)
#define TX_MESSAGE_BUFFER_NUM (0)
#define EXAMPLE_CAN CAN0
#define DEMO_FORCE_CAN_SRC_OSC (1)
#define SET_CAN_QUANTUM     1


// //RTC
// #define EXAMPLE_CAP_LOAD_VALUE RTC_CR_SC8P_MASK


// //BLE
// extern bleResult_t Wireless_Uart_SendData (uint8_t *p_data, uint16_t len);
// extern gapAdvertisingData_t gAppAdvertisingData;
// extern gapScanResponseData_t gAppScanRspData;
// extern unsigned char g_BLE_adv_name[26];
// extern unsigned char g_BLE_adv_name_len;

//Assembly
#define nop() __asm("NOP")


#endif


