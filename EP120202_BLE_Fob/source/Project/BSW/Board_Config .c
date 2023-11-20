//===========================================================================//
//文件说明：芯片KW36的驱动配置模块
//基于环境MCUXpresso IDE V11.2.1，版本SDK_2.2.5_FRDM-KW36
//===========================================================================//
//修订版本：V0.1
//修订人：
//修订时间：20201106
//修订说明：创建初版，提供API接口和函数要求
//1. 按照硬件完成GPIO初始化
//2. 完成CAN初始化，实现CAN任意报文收发，实现报文接收中断，发送完成中断，busoff中断，被动
//错误中断
//3. 完成PIT初始化，实现2ms中断
//4. 完成RTC初始化，实现秒中断
//===========================================================================//

//===========================================================================//
//文件包含
#include "Board_Config.h"
#include "EM000401.h"
#include "fsl_gpio.h"
#include "fsl_flash.h"
#include "fsl_flexcan.h"
#include "fsl_ftfx_flash.h"
#include "TimersManager.h"
#include "rtc_handle.h"
#include "fsl_ftfx_controller.h"
#include "fsl_ftfx_flexnvm.h"
#include "hw_flash.h"
#include "app_preinclude.h"
#include "fsl_cop.h"
#include "Keyboard.h"
#include "ble_ccc_evt_notify.h"
//#include "uwb_SDK_Interface.h"
#include "../../../components/aES/ES000501_uwb/Sources/uwb_SDK_Interface.h"
//#include "SecOC_Auth.h"
//#include "app_RTE.h"
//#include "lpuart_drv.h"
//===========================================================================//
//数据结构定义

//===========================================================================//
//变量定义
//BLE
//底层测试

unsigned char g_KW36_WakeUp_Source;
extern ST_UWBSDKInterface   gStUWBSDKInterface;
extern ST_UWBSource 		stSource;

//ADC0
#define ADC16_CHANNEL_GROUP 0U
//BLE
//eeprom
extern flash_config_t gFlashConfig;
uint32_t g_flexramBlockBase = 0;
uint32_t g_eepromTotalSize = 0;
flexnvm_config_t s_flashDriver;
tmrTimerID_t mCanTimerId = gTmrInvalidTimerID_c;
#if (DISABLE_WDOG == 0U)/*开启开门狗*/
tmrTimerID_t mWTGTimerId = gTmrInvalidTimerID_c;
#endif
extern tmrTimerID_t vcPressTimer;

typedef enum
{
	Press_PB0 = 0,
	Press_PB1,
	Press_PB2,
	Press_PB3,
}vcPressButton_t;
void ButtonPressCallbackISR(vcPressButton_t vcPressButton);




// gpioInputPinConfig_t switchPins[] = {
//     {
// 		.gpioPort = gpioPort_B_c,
// 		.gpioPin = 1U,
// 		.pullSelect = pinPull_Up_c,
// 		.interruptSelect = pinInt_FallingEdge_c
//     },
// 	{
// 		.gpioPort = gpioPort_B_c,
// 		.gpioPin = 2U,
// 		.pullSelect = pinPull_Up_c,
// 		.interruptSelect = pinInt_FallingEdge_c
//     },
// 	{
// 		.gpioPort = gpioPort_B_c,
// 		.gpioPin = 3U,
// 		.pullSelect = pinPull_Up_c,
// 		.interruptSelect = pinInt_FallingEdge_c
//     },
// 	{
// 		.gpioPort = gpioPort_B_c,
// 		.gpioPin = 0U,
// 		.pullSelect = pinPull_Up_c,
// 		.interruptSelect = pinInt_FallingEdge_c
//     }
// };



static const gpioInputPinConfig_t lockIRQPin =
{
	.gpioPort = gpioPort_B_c,
	.gpioPin = 0U,
	.pullSelect = pinPull_Up_c,
	.interruptSelect = pinInt_FallingEdge_c
};
static const gpioInputPinConfig_t unlockIRQPin =
{
	.gpioPort = gpioPort_B_c,
	.gpioPin = 1U,
	.pullSelect = pinPull_Up_c,
	.interruptSelect = pinInt_FallingEdge_c
};
static const gpioInputPinConfig_t trunkIRQPin =
{
	.gpioPort = gpioPort_B_c,
	.gpioPin = 2U,
	.pullSelect = pinPull_Up_c,
	.interruptSelect = pinInt_FallingEdge_c
};
static const gpioInputPinConfig_t panicIRQPin =
{
	.gpioPort = gpioPort_B_c,
	.gpioPin = 3U,
	.pullSelect = pinPull_Up_c,
	.interruptSelect = pinInt_FallingEdge_c
};



static const gpioInputPinConfig_t intUwbIRQPin =
{
	.gpioPort = gpioPort_B_c,
	.gpioPin = 18U,
	.pullSelect = pinPull_Up_c,
	.interruptSelect = pinInt_FallingEdge_c
};
static const gpioInputPinConfig_t rdyUwbIRQPin =
{
	.gpioPort = gpioPort_C_c,
	.gpioPin = 4U,
	.pullSelect = pinPull_Down_c,
	.interruptSelect = pinInt_Disabled_c
};
static uint8_t gWakeFlag = 0;
//===========================================================================//
//函数说明：KW36时钟初始化函数
//作者：
//输入说明：无
//输出说明：无
//配置说明：需要在这里加以说明
//===========================================================================//
void KW36_Clock_Config (void)
{
	CLOCK_EnableClock(kCLOCK_PortA);
	CLOCK_EnableClock(kCLOCK_PortB);
	CLOCK_EnableClock(kCLOCK_PortC);
	CLOCK_EnableClock(kCLOCK_Spi0);
	CLOCK_EnableClock(kCLOCK_Spi1);
}
//===========================================================================//

/*中断处理例程*/
void LOCK_IRQ_ISR(void)
{
	GpioClearPinIntFlag(&lockIRQPin);
//	GpioUninstallIsr(&lockIRQPin);
	if (TMR_IsTimerActive(vcPressTimer))
	{
		TMR_StopTimer(vcPressTimer);
	}
	TMR_StartLowPowerTimer(vcPressTimer, gTmrLowPowerSingleShotMillisTimer_c, 50, ButtonPressCallbackISR, Press_PB0);
}
/*中断处理例程*/
void UNLOCK_IRQ_ISR(void)
{
	GpioClearPinIntFlag(&unlockIRQPin);
//	GpioUninstallIsr(&unlockIRQPin);
	if (TMR_IsTimerActive(vcPressTimer))
	{
		TMR_StopTimer(vcPressTimer);
	}
	TMR_StartLowPowerTimer(vcPressTimer, gTmrLowPowerSingleShotMillisTimer_c, 50, ButtonPressCallbackISR, Press_PB1);
}
/*中断处理例程*/
void TRUNK_IRQ_ISR(void)
{
	GpioClearPinIntFlag(&trunkIRQPin);
//	GpioUninstallIsr(&trunkIRQPin);
	if (TMR_IsTimerActive(vcPressTimer))
	{
		TMR_StopTimer(vcPressTimer);
	}
	TMR_StartLowPowerTimer(vcPressTimer, gTmrLowPowerSingleShotMillisTimer_c, 50, ButtonPressCallbackISR, Press_PB2);
}
/*中断处理例程*/
void PANIC_IRQ_ISR(void)
{
	GpioClearPinIntFlag(&panicIRQPin);
//	GpioUninstallIsr(&panicIRQPin);
	if (TMR_IsTimerActive(vcPressTimer))
	{
		TMR_StopTimer(vcPressTimer);
	}
	TMR_StartLowPowerTimer(vcPressTimer, gTmrLowPowerSingleShotMillisTimer_c, 50, ButtonPressCallbackISR, Press_PB3);
}

volatile u8 intIRQFlag = 0;
/*中断处理例程*/
void INT_UWB_IRQ_ISR(void)
{
	GpioClearPinIntFlag(&intUwbIRQPin);


	//if(gStUWBSDKInterface.fpCmmuIsTransmitComplate())
	if((stSource.stUCIState.stUWBCommu.fpCmmuIsTransmitComplate == NULL)||(stUWBSDK.fpUQSetNTFCacheFlag == NULL))
	{
		return;
	}
	if(stSource.stUCIState.stUWBCommu.fpCmmuIsTransmitComplate())
	{
		if(intIRQFlag == 0U)
		{
			intIRQFlag = 1U;
			ble_ccc_send_evt(UWB_EVT_INT_NOTICE,0U,NULL,0U);
		}

	}
	else
	{
		if((intIRQFlag == 0x00 )|| (intIRQFlag==0x01))
		{
			stUWBSDK.fpUQSetNTFCacheFlag(&stSource);
		}
		
	}

}
void INT_Set_Flag(void)
{
	intIRQFlag = 0x02;
}
void INT_Clr_Flag(void)
{
	intIRQFlag = 0x00;
}

void ButtonPressCallbackISR(vcPressButton_t vcPressButton)
{
    switch (vcPressButton)
    {
        case Press_PB0:
        {
			if (ButtonReadPB0() == 0U)
			{
				LOG_L_S(BASE_MD,"Lock: Press PB0 !!!\r\n");
				ble_ccc_evt_rke_cmd(CCC_FUNCTION_ID_CENTRAL_LOCKING,CCC_ACTION_ID_LOCK);
			}
//			GpioInstallIsr(LOCK_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &lockIRQPin);
            break;
        }
        case Press_PB1:
        {
			if (ButtonReadPB1() == 0U)
			{
				LOG_L_S(BASE_MD,"unLock: Press PB1 !!!\r\n");
				ble_ccc_evt_rke_cmd(CCC_FUNCTION_ID_CENTRAL_LOCKING,CCC_ACTION_ID_UNLOCK);
			}
//			GpioInstallIsr(UNLOCK_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &unlockIRQPin);
            break;
        }
        case Press_PB2:
        {
			if (ButtonReadPB2() == 0U)
			{
				LOG_L_S(BASE_MD,"Trunk: Press PB2 !!!\r\n");
				ble_ccc_evt_rke_cmd(CCC_FUNCTION_ID_MANUAL_TRUNK,CCC_ACTION_ID_MANUAL_TRUNK_RALEASE);
			}
//			GpioInstallIsr(TRUNK_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &trunkIRQPin);
            break;
        }
        case Press_PB3:
        {
			if (ButtonReadPB3() == 0U)
			{
				LOG_L_S(BASE_MD,"PANIC: Press PB3 !!!\r\n");
				ble_ccc_evt_rke_cmd(CCC_FUNCTION_ID_PANIC_ALARM,CCC_ACTION_ID_PANIC_TRIGGER_ALARM);
			}
//			GpioInstallIsr(PANIC_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &panicIRQPin);
            break;
        }
        default:
        {
            ; /* No action required */
            break;
        }
    }
}
#if (DISABLE_WDOG == 0U)/*开启开门狗*/
//uint32_t wtdogCnt = 0;
void KW38_Reset_WTG_Counter(void)
{
	//wtdogCnt++;
	COP_Refresh(SIM);
	//rtc_display_time();
	//LOG_L_S(CAN_MD,"wtdogCnt:%d\r\n",wtdogCnt);
}
void KW38_WTG_Callback (void)
{
	KW38_Reset_WTG_Counter();
}
void KW38_Watchdog_Config (void)
{
	/*long mode, 8S 超时reset,last 25%(6~8S)喂狗*/
	cop_config_t configCop;
	COP_GetDefaultConfig(&configCop);
	configCop.timeoutMode = kCOP_LongTimeoutMode;
	configCop.timeoutCycles = kCOP_2Power5CyclesOr2Power13Cycles;
	COP_Init(SIM, &configCop);
	if(mWTGTimerId == gTmrInvalidTimerID_c)
	{
		mWTGTimerId = TMR_AllocateTimer();
	}
	/*低功耗模式下会清除counter*/
	TMR_StartLowPowerTimer(mWTGTimerId, gTmrLowPowerIntervalMillisTimer_c, 2000, KW38_WTG_Callback, NULL);
}
#endif

void KW38_Int_Start(void)
{
	GpioInstallIsr(INT_UWB_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &intUwbIRQPin);
}
void KW38_INT_Stop(void)
{
	GpioUninstallIsr(&intUwbIRQPin);
}

//===========================================================================//
//函数说明：KW36端口IO初始化函数
//作者：
//输入说明：无
//输出说明：无
//配置说明：需要在这里加以说明
//===========================================================================//
void KW38_GPIO_Config (void)
{
    //CAN_InitPins();
    //管脚名称， 管脚使用资源， 管脚用途说明
    //PA0, SWD_DIO, 默认不管
    //PA1, SWD_CLK, 默认不管
    //PA2, RESET, 默认不管

    //PA16, SPI1_SOUT 	(UWB MOSI)
    //PA17, SPI1_SIN	(UWB MISO)
    //PA18, SPI1_SCK	(UWB CLK)
    //PA19, SPI1_PCS0	(UWB CS)
	/*对应 UWB芯片SPI引脚接口*/
#ifdef FIT_SUPPORT	
    PORT_SetPinMux(PORTA, 16u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTA, 17u, kPORT_MuxAlt2);
	PORT_SetPinMux(PORTA, 18u, kPORT_MuxAlt2);
//    PORT_SetPinMux(PORTA, 19u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTA, 19u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOA, 19u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
#else
    PORT_SetPinMux(PORTC, 1u, kPORT_MuxAlt8);	/*CLK*/
    PORT_SetPinMux(PORTC, 2u, kPORT_MuxAlt8);	/*MOSI*/
	PORT_SetPinMux(PORTC, 3u, kPORT_MuxAlt8);	/*MISO*/
    PORT_SetPinMux(PORTC, 4u, kPORT_MuxAlt8);	/*CS*/

	PORT_SetPinMux(PORTA, 18u, kPORT_MuxAsGpio); /*VCC*/
    GPIO_PinInit(PORTA, 18u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});
#endif
    //PB0, gpio输入低, (lock Key)
    //PB1, gpio输入低, (unlock key)
    //PB2, gpio输入低, (Trunk key)
    //PB3, gpio输入低, (PANIC)
	GpioInputPinInit(&lockIRQPin,1);
	GpioInstallIsr(LOCK_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &lockIRQPin);
	GpioInputPinInit(&unlockIRQPin,1);
	GpioInstallIsr(UNLOCK_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &unlockIRQPin);
	GpioInputPinInit(&trunkIRQPin,1);
	GpioInstallIsr(TRUNK_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &trunkIRQPin);
	GpioInputPinInit(&panicIRQPin,1);
	GpioInstallIsr(PANIC_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &panicIRQPin);

    //PB18, gpio输入, (INT_UWB)
	//PC3, gpio输出, (RST_UWB)
    //PC4, gpio输入, (RDY_UWB)
	GpioInputPinInit(&intUwbIRQPin,1);
	GpioInstallIsr(INT_UWB_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &intUwbIRQPin);
//	GpioInputPinInit(&rdyUwbIRQPin,1);
	PORT_SetPinMux(PORTC, 3u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 3u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
//    GpioInputPinInit(&rdyUwbIRQPin,1);
	PORT_SetPinMux(PORTC, 4u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 4u, &(gpio_pin_config_t){kGPIO_DigitalInput, 1U});
    //PC16, SPI0_SCK(功能2)，	(se安全芯片 sclk)
    //PC17, SPI0_SO(功能2)，	(se安全芯片MOSI)
    //PC18, SPI0_SIN(功能2)，	(se安全芯片MISO)
    //PC19, SPI0_CS，			(安全芯片SPI_CS)
    PORT_SetPinMux(PORTC, 16u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC, 17u, kPORT_MuxAlt2);
	PORT_SetPinMux(PORTC, 18u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC, 19u, kPORT_MuxAlt2);
	//PC1, RST_SE，			(安全芯片RESET)
	//PC5, SE_EN，			(安全芯片VCC控制)
#ifdef FIT_SUPPORT
	PORT_SetPinMux(PORTC, 1u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 1u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
	PORT_SetPinMux(PORTC, 5u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 5u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
#else
	PORT_SetPinMux(PORTA, 17u, kPORT_MuxAsGpio);/*VCC*/
    GPIO_PinInit(PORTA, 17u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});
#endif
    CAN_tranceiver_ON ();
}


#if 1
//===========================================================================//
//函数说明：KW38 RTC初始化函数
//输入说明：无
//输出说明：无
//配置说明：无
void KW36_RTC_Config (void)
{
	rtc_init();
}
//===========================================================================//
//函数说明：KW38 SPI始终初始化函数
//输入说明：无
//输出说明：无
//配置说明：SPI配置要求4M，模式0，8bit，主机模式，CS由GPIO控制，收发采用查询
//===========================================================================//	
void KW38_SPI1_Config(void)
{
	dspi_master_config_t masterConfig;
	u32 srcClock_Hz;

	masterConfig.whichCtar = kDSPI_Ctar0;
    masterConfig.ctarConfig.baudRate = 1000000U;/*! Transfer baudrate - 4000k *///TRANSFER_BAUDRATE;
    masterConfig.ctarConfig.bitsPerFrame = 8U;
    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveLow;
    masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
    masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
    masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 1000000000U / 4000000U;
    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 1000000000U / 4000000U;
    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000000000U / 4000000U;

    masterConfig.whichPcs = kDSPI_Pcs0;
    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

    masterConfig.enableContinuousSCK = false;
    masterConfig.enableRxFifoOverWrite = false;
    masterConfig.enableModifiedTimingFormat = false;
    masterConfig.samplePoint = kDSPI_SckToSin0Clock;

    srcClock_Hz = CLOCK_GetFreq(DSPI1_CLK_SRC);//DSPI_MASTER_CLK_FREQ;
    DSPI_MasterInit(SPI1, &masterConfig, srcClock_Hz);
}

#else
#endif

//===========================================================================//
//函数说明：KW36 DFLASH初始化
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void KW36_DFLASH_Config (void)
{
	NV_Init();//参见函数 NV_Init();在source\common\ApplMain.c中main_task完成了初始化
}

//===========================================================================//
//函数说明：KW36 DFLASH擦除扇区
//作者：
//输入说明：最小值 0
//输出说明：无
//配置说明：无
//===========================================================================//
unsigned char KW36_DFlash_Erase_Sector (unsigned short int SectorIx)
{
#if 0
#endif
    return 0;
}


//===========================================================================//
//函数说明：KW36 写DFLASH扇区
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
unsigned char KW36_DFlash_Write_Sector (unsigned short int SectorIx, unsigned char *p_data)
{
#if 0
#endif
   return 0;
}

//===========================================================================//
//函数说明：KW36 读取DFLASH扇区
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
unsigned char KW36_DFlash_Read_Sector (unsigned short int SectorIx, unsigned char *p_data)
{
#if 0
#endif
	return 0;
}
//===========================================================================//
//函数说明：KW36 ADC初始化
//作者：
//输入说明：无
//输出说明：无
//配置说明：ADC分辨率16位，参考电压VrefH引脚，ADC时钟？？
//===========================================================================//
void KW36_ADC_Config (void)
{
	adc16_config_t adc16ConfigStruct;

	//输入电压选择，VREF或者VALT
	adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceVref;
	//adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceValt;
	//时钟源可选择4种，当前选择异步时钟源
	adc16ConfigStruct.clockSource = kADC16_ClockSourceAsynchronousClock;
	//打开异步时钟
	adc16ConfigStruct.enableAsynchronousClock = true;
	//分频系数配置
	adc16ConfigStruct.clockDivider = kADC16_ClockDivider1;
	//分辨率配置为 16 bit
	adc16ConfigStruct.resolution = kADC16_ResolutionSE16Bit;
	// Disable the long sample feature.
	adc16ConfigStruct.longSampleMode = kADC16_LongSampleDisabled;
	//Close High-Speed
	adc16ConfigStruct.enableHighSpeed = false;
	//Close Low-Power
	adc16ConfigStruct.enableLowPower = false;
	//关闭连续转换
	adc16ConfigStruct.enableContinuousConversion = false;

	//ADC 初始化
    ADC16_Init(ADC0, &adc16ConfigStruct);

    //使用软件触发
    ADC16_EnableHardwareTrigger(ADC0, false);
    ADC16_DoAutoCalibration(ADC0);
}

//===========================================================================//
//函数说明：KW36 ADC通道转换
//作者：
//输入说明：ADC_ch，ADC的通道，取值0-31
//输出说明：true，初始化成功
//配置说明：单次采样通道
//===========================================================================//
unsigned char KW36_ADC_Start (uint8_t ADC_ch)
{
	adc16_channel_config_t adc16ChannelConfigStruct;

	//通道合法性校验
	if(ADC_ch >= 31)
	{
		return false;
	}

	//ADC通道配置
	adc16ChannelConfigStruct.channelNumber = ADC_ch;
	//关闭转换中断
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
	//选择为单端输入
	adc16ChannelConfigStruct.enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */

	//写SC1A开始采样
	ADC16_SetChannelConfig(ADC0, ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
	return true;
}

//===========================================================================//
//函数说明：KW36 获取ADC转换结果
//作者：
//输入说明：无
//输出说明：ADC转换结果
//配置说明：无
//===========================================================================//
unsigned char KW36_ADC_GetResult (unsigned short int*p_Result)
{
	uint8_t count = 100;

	while (0U == (kADC16_ChannelConversionDoneFlag &
					  ADC16_GetChannelStatusFlags(ADC0, ADC16_CHANNEL_GROUP)))
	{
		//防止读不到数据死循环
		if(count-- <= 0)
		{
			return false;
		}
	}
	*p_Result = (uint16_t)ADC16_GetChannelConversionValue(ADC0, ADC16_CHANNEL_GROUP);
	return true;
}
//#define EEPROM_DATA_SET_SIZE_CODE (0x33U)//2KB
#define EEPROM_DATA_SET_SIZE_CODE (0x32U)//4KB
#define FLEXNVM_PARTITION_CODE (0x3U)
//===========================================================================//
//函数说明：KW36初始化内部自带的eeeprom（使用DFLASH模拟）
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
status_t KW38_eeprom_Init (void)
{
	ftfx_security_state_t securityStatus = kFTFx_SecurityStateNotSecure; /* Return protection status */
	status_t result; /* Return code from each flash driver function */

    uint32_t dflashTotalSize  = 0;
    uint32_t dflashBlockBase  = 0;
    /* Clean up Flash driver Structure*/
	memset(&s_flashDriver, 0, sizeof(flexnvm_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLEXNVM_Init(&s_flashDriver);
    if (kStatus_FTFx_Success != result)
    {
        return result;
    }
    /* Check security status. */
    result = FLEXNVM_GetSecurityState(&s_flashDriver, &securityStatus);
    if (kStatus_FTFx_Success != result)
    {
    	return result;
    }
    /* Print security status. */
    switch (securityStatus)
    {
        case kFTFx_SecurityStateNotSecure:
            break;
        case kFTFx_SecurityStateBackdoorEnabled:
            break;
        case kFTFx_SecurityStateBackdoorDisabled:
            break;
        default:
            break;
    }

    /* Debug message for user. */
    /* Test flexnvm dflash feature only if flash is unsecure. */
    if (kFTFx_SecurityStateNotSecure != securityStatus)
    {
        return 1;
    }
    else
    {
        uint32_t flexramTotalSize = 0;
        /* Get flash properties*/
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyDflashBlockBaseAddr, &dflashBlockBase);
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyFlexRamBlockBaseAddr, &g_flexramBlockBase);
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyFlexRamTotalSize, &flexramTotalSize);
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyEepromTotalSize, &g_eepromTotalSize);
        if (!g_eepromTotalSize)
        {
            /* Note: The EEPROM backup size must be at least 16 times the EEPROM partition size in FlexRAM. */
            uint32_t eepromDataSizeCode = EEPROM_DATA_SET_SIZE_CODE;//gEEPROM_DATA_SET_SIZE_CODE_c
            uint32_t flexnvmPartitionCode = FLEXNVM_PARTITION_CODE;//gFLEXNVM_PARTITION_CODE_c
            result = FLEXNVM_ProgramPartition(&s_flashDriver, kFTFx_PartitionFlexramLoadOptLoadedWithValidEepromData,
                                              eepromDataSizeCode, flexnvmPartitionCode);
            if (kStatus_FLASH_Success != result)
            {
            	return result;
            }
            /* Reset MCU */
            NVIC_SystemReset();
        }
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyDflashTotalSize, &dflashTotalSize);

        result = FLEXNVM_SetFlexramFunction(&s_flashDriver, kFTFx_FlexramFuncOptAvailableForEeprom);
        if (kStatus_FLASH_Success != result)
        {
        	return result;
        }
    }
}

//===========================================================================//
//函数说明：KW36写内部自带的eeeprom（使用DFLASH模拟）
//输入说明：addr, eeprom地址，取值 EEPROM_ADDR_START - EEPROM_ADDR_END
//		   p_data, 待写入eeprom的数据
//		   len, 数据的长度
//输出说明：EEPROM_ACCESS_xxxx. 0，写入成功；=1，地址超限；=2，写入失败
//配置说明：无
//===========================================================================//
uint8_t KW38_Write_eeprom (uint32_t addr, uint8_t *p_data,uint8_t len)
{
	//status_t DriverResult = EEPROM_ACCESS_WRITE_FAIL; /* Return code from each flash driver function */
	uint8_t Result;
	uint8_t tmpData[128] = {0};
	uint16_t tmpLength;
	uint8_t ret;
	ret = hw_flash_read(NVM_FLASH_CAN_AREA,(u16)addr,tmpData,&tmpLength);
	if (ret == NVM_SUCCESS)
	{
		//LOG_L_S()
		if (tmpLength == len)
		{			
			if (0U == core_mm_compare(tmpData,p_data,tmpLength))
			{
				return;//待更新的数据和保存的数据一致，不写
			}
		}
	}
	else
	{
		//LOG_L_S()
	}
	ret = hw_flash_write(NVM_FLASH_CAN_AREA,(u16)addr,p_data,len);
	if (ret != NVM_SUCCESS)
	{
		LOG_L_S(CAN_MD,"CAN Flash Write Faid!!!  ID: %0.4x, Len:%d \r\n",(u16)addr,len);
	}
    return ret;
}

//===========================================================================//
//函数说明：KW36读内部自带的eeeprom（使用DFLASH模拟）
//输入说明：addr, eeprom地址，取值 EEPROM_ADDR_START - EEPROM_ADDR_END
//		   p_data, eeprom数据读出来保存的地址
//		   len, 读取数据的长度
//输出说明：EEPROM_ACCESS_xxxx. 0，读取成功；=1，地址超限；=2，读取失败
//配置说明：无
//===========================================================================//
uint8_t KW38_Read_eeprom(uint32_t addr, uint8_t *p_data,uint16_t len)
{	
	// uint16_t i;
	uint8_t ret;
	uint16_t tmpLength;
	uint8_t tmpData[128] = {0};
	ret = hw_flash_read(NVM_FLASH_CAN_AREA,(u16)addr,tmpData,&tmpLength);
	if (ret == NVM_SUCCESS)
	{
		if (len <= tmpLength)
		{
			core_mm_copy(p_data,tmpData,len);
			return 1;
		}
	}
	core_mm_set(p_data,0x00,len);
    return 1;
}

//===========================================================================//
//函数说明：KW36初始化函数
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void Board_Config (void)
{
#ifdef APP_DEBUG_MODE    
	BOARD_InitLPUART();
	serial_debug_init();
	//serial_init();
#endif	
	KW36_Clock_Config ();
	KW38_GPIO_Config ();

	// KW36_RTC_Config ();
	//KW38_SPI1_Config ();/*UWB 初始化在UWB SDK中调用*/

	KW36_ADC_Config ();
#if (DISABLE_WDOG == 0U)/*开启开门狗*/ 
	KW38_Watchdog_Config();
#endif	
}

//===========================================================================//
//函数说明：KW36进入低功耗模式8
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void Board_LowPower (void)
{
	gWakeFlag = 1U;
	user_Task_pend();
	//LIN

	//SE安全芯片
	SE_SPI_CS_Low ();
	SE_Reset_Set_Low ();
	SE_PowerOff ();
	// KW36_SPI0_Config (false);
//	//打印口
#ifdef DEBUG_PRINT
	 LOG_L_S(CAN_MD,"Board_LowPower\r\n");
#endif
	rtc_display_time();
	PWR_AllowDeviceToSleep();

}

//===========================================================================//
//函数说明：KW36从低功耗模式恢复
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
extern void  BOARD_BootClockRUN(void);
void Board_LowPower_Recovery (void)
{
#if 1
	if (gWakeFlag == 0U)/*当前如果处于工作模式下，不允许重复调用*/
	{
		return;
	}
	gWakeFlag = 0U;

	user_Task_reSume();

// 	//唤醒后恢复配置
 	Board_Config ();

// 	//通知睡眠模块
 	PWR_DisallowDeviceToSleep();

	rtc_display_time();
	// LOG_L_S(CAN_MD,"WakeUp Source: %d\r\n",Get_Wakeup_Reason());
	// LOG_L_S(CAN_MD,"LPCD Status: %d\r\n",g_nfcBuzStatus.lpcdStatus);
	// LOG_L_S(CAN_MD,"gWakePortScan: %d\r\n",gWakePortScan);
	// LOG_L_S(CAN_MD,"PWRLib_MCU_WakeupReason: %0.2x\r\n",PWRLib_MCU_WakeupReason);
	
//	LOG_L_S(CAN_MD,"\r\n WakeUp Source: %d\r\n LPCD Status: %d\r\n gWakePortScan: %d\r\n PWRLib_MCU_WakeupReason: %x\r\n",Get_Wakeup_Reason(),g_nfcBuzStatus.lpcdStatus,gWakePortScan,PWRLib_MCU_WakeupReason);
#endif
}

//===========================================================================//
//函数说明：配置KW36的唤醒源
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void Board_Set_WakeupSource (uint8_t src)
{
	BSW_Set_Sleep_Wakeup_Reason (src);
}

//===========================================================================//
//函数说明：板级基础软件运行时主函数
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void Board_BSW_main (void)
{
	uint16_t ADC_SampleData;
	uint8_t ADC_Result;
	//uint16_t tmp;
	KW36_ADC_Start (ADC_BAT);
	ADC_Result = KW36_ADC_GetResult (&ADC_SampleData);
	if (ADC_Result)
	{
	}
}


