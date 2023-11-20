//===========================================================================//
//文件说明：芯片KW36的驱动配置模块头文件声明
//基于环境MCUXpresso IDE V11.2.1，版本SDK_2.2.5_FRDM-KW36


#ifndef _BOARD_CFG_H
#define _BOARD_CFG_H


//文件包含
#include "Board_Config_Para.h"
#include "alg.h"
#include "fsl_gpio.h"
//DEBUG信息开关
#define DEBUG_PRINT

#define KW_Print(x) //关闭打印函数



//宏定义
#define KW_TRUE 1
#define KW_FALSE 0
#define LIN_UART_enable_LIN

//唤醒源定义
#define KW38_WAKEUP_SRC_COUNT	2


#define KW36_WakeUp_Src_CAN (1 << 0)
#define KW36_WakeUp_Src_GPIOxx (1 << 1)
#define KW36_WakeUp_Src_Timerxx (1 << 2)

// #define SE_SPI_CS_TOG()					GPIO_PortToggle(GPIOC, (1U << 19))
// #define SE_SPI_CS_High()     			GPIO_PinWrite (GPIOC, 19, 1)
// #define SE_SPI_CS_Low()      			GPIO_PinWrite (GPIOC, 19, 0)
// #define SE_Reset_Set_High()   			GPIO_PinWrite (GPIOB, 18, 1) //PB18
// #define SE_Reset_Set_Low()   			GPIO_PinWrite (GPIOB, 18, 0) //PB18
#define SE_PowerOn()					GPIO_PinWrite (PORTC, 5, 1) //PA17
#define SE_PowerOff()					GPIO_PinWrite (PORTC, 5, 0) //PA17
#define CAN_tranceiver_ON()				//GPIO_PinWrite (GPIOC, 2, 0) //PC2
#define CAN_tranceiver_OFF()			//GPIO_PinWrite (GPIOC, 2, 1) //PC2
#define BLE_ANT_LIN_tranceiver_ON()		GPIO_PinWrite (GPIOC, 5, 1) //PC5
#define BLE_ANT_LIN_tranceiver_OFF()	GPIO_PinWrite (GPIOC, 5, 0) //PC5
#define RF_Request_DK()					GPIO_PinRead (GPIOB, (1U << 2)) //PB2
#define ADC_BAT							0

#define ButtonReadPB0()		            GPIO_PinRead (GPIOB, 0) //PB0
#define ButtonReadPB1()		            GPIO_PinRead (GPIOB, 1) //PB1
#define ButtonReadPB2()		            GPIO_PinRead (GPIOB, 2) //PB2
#define ButtonReadPB3()		            GPIO_PinRead (GPIOB, 3) //PB3

#endif


