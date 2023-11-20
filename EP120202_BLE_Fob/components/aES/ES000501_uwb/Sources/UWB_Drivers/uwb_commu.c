/*
 * nxp_29d5d_uwb_uci.c
 *
 *  Created on: 2022年6月24日
 *      Author: JohnSong
 */

//#include "lpspi_master_driver.h"
#include "../uwb_common_def.h"
#include "./uwb_timer.h"
#include "./uwb_commu.h"

#include "Ble_Spi_Drv.h"
#include "MKW38A4.h"
#include "fsl_dspi.h"
#include "EM000401.h"
#include "fsl_gpio.h"
#include "Board_Config.h"


static bool		uwb_commu_is_notify_coming(void);
static uint32_t uwb_commu_init(void);
static uint32_t uwb_commu_reset(ST_TimerTools* pst_timer_tools, uint8_t* recv_buff_ptr, uint16_t* length);
static uint32_t uwb_commu_send(ST_TimerTools* pst_timer_tools, const uint8_t* send_buff_ptr, const uint16_t length);
static uint32_t uwb_commu_recv(ST_TimerTools* pst_timer_tools, uint8_t* recv_buff_ptr, uint16_t* length);
static uint32_t uwb_commu_sleep(void);
static uint32_t uwb_commu_wakeup(void);
static uint32_t uwb_commu_deinit(void);


#define PRINT_LOG
#define UWB_SPI_MASTER_INSTANCE (1u)//(SPI_OVER_LPSPI00_INSTANCE)
#define TIMER_CHAN_TYPE_ONESHOT		0U

//Global value
static 			uint8_t 		uwb_spi_send_buffer[SPI_BUF_MAX_LENS] 	= {0};
static 			uint8_t 		uwb_spi_recv_buffer[SPI_BUF_MAX_LENS] 	= {0};
static const 	uint8_t 		UWB_SPI_NULL_BYTE[SPI_BUF_MAX_LENS] 	= {0};	//UCI max payload length
static 			E_UCICommuState	cmd_state 								= UciCommuState_Undefined;
static 			E_UCICommuState	res_state 								= UciCommuState_Undefined;

/* NCJ29D5D --------> Slave S32K_SPI Master
 * 6 Line SPI PIN Map
 * -------------------------------
 * 5 Chip select 		PTB5
 * 7 SDI 				PTB4
 * 9 SDO 				PTB3
 * 11 Serial clock 		PTB2
 * 13 GND 				GND
 * 6 READY 				PTE0 |GPIO < - in
 * 7 3.3 V supply 3.3 V
 * 8 INT 				PTE9 |GPIO < - in
 * 9 5 V supply 5 V
 * 10 /RESET 			PTC5 |GPIO - > out
 * 11,13 GND 			GND
 * ncj29d5d default by reset is CPHA=0  CPOL=1 。
 */

/*
 NCJ29D5D 与 libphscaUciCom 提供的车内其他设备之间的通信是通过 SPI 接口完成的。 NCJ29D5D 始终充当 SPI 从机，需要成为唯一的从机（我们仅支持点对点连接）。 为了实现更稳定和更快的通信，使用了 6 线 SPI 连接，除了默认的 4 线之外，我们还使用两条额外的从机控制线（RDY、INT）扩展了接口。
以下 SPI 时钟模式在 libphscaUciCom 中配置。
• 时钟极性：高
• 时钟相位：数据在跟随沿发生变化。
DMA 通道 3 配置为发送，通道 4 配置为接收。 从 NCJ29D5D 到主机的所有传输帧都有 1 个额外的前导字节，带有 0x00
SPI 是超宽带命令接口 (UCI) 使用的较低层。 正如 UCI 中定义的那样，我们有三种不同类型的消息：
• 命令：从主机发送到 NCJ29D5D 的数据
• 响应：在发送命令后从 NCJ29D5D 发送到主机的数据
• 通知：NCJ29D5D 自发向主机发送数据

 */


#if defined(NXP_RCI_FRAME)
/* CRC余式表 */
const unsigned int crc_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

//查表法计算crc
unsigned short do_crc_table(unsigned char *ptr,int len)
{
    unsigned short crc = 0x0000;

    while(len--)
    {
        crc = (crc << 8) ^ crc_table[(crc >> 8 ^ *ptr++) & 0xff];
    }

    return(crc);
}
#endif
typedef struct st_NXPS32KPortpin_t
{
	PORT_Type * PortBase;
	GPIO_Type * GpioBase;
	uint32_t PinsIdx;
} st_NXPS32KPortpin;

/** The host drives RSTn signal to hard-reset RCI device */
//#define UWB_SPI_PORT	PTB
//EVB 接线方式
static const st_NXPS32KPortpin UWB_SPI_SS 		= { PORTA, GPIOA, 19u };
//static const st_NXPS32KPortpin UWB_SPI_MOSI 	= { PORTB, PTB, 4u };
//static const st_NXPS32KPortpin UWB_SPI_MISO 	= { PORTB, PTB, 3u };
//static const st_NXPS32KPortpin UWB_SPI_SCK 	= { PORTB, PTB, 2u };
#if defined(UWB_INITIATOR)
	static const st_NXPS32KPortpin UWB_SPI_nRST 	= { PORTC, GPIOC, 1u };
	static const st_NXPS32KPortpin UWB_SPI_RDY 		= { PORTC, GPIOC, 4u };
#elif defined(UWB_RESPONDER)
	static const st_NXPS32KPortpin UWB_SPI_nRST 	= { PORTC, GPIOC, 1u };
	static const st_NXPS32KPortpin UWB_SPI_RDY 		= { PORTC, GPIOB, 0u };
#else
?
#endif

static const st_NXPS32KPortpin UWB_SPI_INT 		= { PORTB, GPIOB, 18u };

#define UWB_SPI_IDEL_MODE(x) 		((pins_channel_type_t)(1 << x))
#define UWB_SET_IO_DIRC_OUT(x) 		((pins_channel_type_t)(1 << x))
#define UWB_SET_IO_DIRC_IN(x)		((pins_channel_type_t)(0 << x))


typedef enum enum_UWB_UCI_SPI_PinLevel_t
{
	UCI_SPI_PINLEVEL_LOW = 0u,
	UCI_SPI_PINLEVEL_HIGH = 1u,
	UCI_SPI_PINLEVEL_RST_ACTIVE 	= UCI_SPI_PINLEVEL_LOW,
	UCI_SPI_PINLEVEL_RST_INACTIVE 	= UCI_SPI_PINLEVEL_HIGH,
	UCI_SPI_PINLEVEL_IRQ_ACTIVE 	= UCI_SPI_PINLEVEL_LOW,
	UCI_SPI_PINLEVEL_IRQ_INACTIVE 	= UCI_SPI_PINLEVEL_HIGH,
	UCI_SPI_PINLEVEL_RDY_ACTIVE 	= UCI_SPI_PINLEVEL_LOW,
	UCI_SPI_PINLEVEL_RDY_INACTIVE 	= UCI_SPI_PINLEVEL_HIGH,
	UCI_SPI_PINLEVEL_CS_ACTIVE 		= UCI_SPI_PINLEVEL_LOW,
	UCI_SPI_PINLEVEL_CS_INACTIVE 	= UCI_SPI_PINLEVEL_HIGH,

}E_UWBUCISPIPinLevel;

void spi_set_rst_line_level(uint8_t level)
//void spi_set_rst_line_level(E_UWBUCISPIPinLevel level)
{
	__asm("NOP");
	__asm("NOP");
	__asm("NOP");
	PORT_SetPinMux(PORTC, 1u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 1u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
	if (UCI_SPI_PINLEVEL_HIGH == level)
	{
		// PINS_DRV_SetPins(UWB_SPI_nRST.GpioBase,(pins_channel_type_t) 1u << (UWB_SPI_nRST.PinsIdx));
		GPIO_PinWrite (UWB_SPI_nRST.GpioBase, UWB_SPI_nRST.PinsIdx, 1U);
	}
	else
	{
		// PINS_DRV_ClearPins(UWB_SPI_nRST.GpioBase,(pins_channel_type_t) 1u << (UWB_SPI_nRST.PinsIdx));
		GPIO_PinWrite (UWB_SPI_nRST.GpioBase, UWB_SPI_nRST.PinsIdx, 0U);
	}
}

void spi_set_csn_line_level(uint8_t level)
//void spi_set_csn_line_level(E_UWBUCISPIPinLevel level)
{
	if (UCI_SPI_PINLEVEL_HIGH == level)
	{
		//PINS_DRV_SetPins(UWB_SPI_SS.GpioBase,(pins_channel_type_t) 1u << (UWB_SPI_SS.PinsIdx));
		GPIO_PinWrite (UWB_SPI_SS.GpioBase, UWB_SPI_SS.PinsIdx, 1U);
	}
	else
	{
		//PINS_DRV_ClearPins(UWB_SPI_SS.GpioBase,(pins_channel_type_t) 1u << (UWB_SPI_SS.PinsIdx));
		GPIO_PinWrite (UWB_SPI_SS.GpioBase, UWB_SPI_SS.PinsIdx, 0U);
	}
}

uint8_t spi_get_int_line_level(void)
//E_UWBUCISPIPinLevel spi_get_int_line_level(void)
{
	E_UWBUCISPIPinLevel 	pinVal;
	u8 	portVal;

	//portVal = PINS_DRV_ReadPins(UWB_SPI_INT.GpioBase);
	portVal = GPIO_PinRead (UWB_SPI_INT.GpioBase, UWB_SPI_INT.PinsIdx);
	// pinVal = (0u == (portVal & (pins_channel_type_t) 1u << (UWB_SPI_INT.PinsIdx))) ?
	// 				UCI_SPI_PINLEVEL_LOW : UCI_SPI_PINLEVEL_HIGH;
	pinVal = (0u == portVal ) ?	UCI_SPI_PINLEVEL_LOW : UCI_SPI_PINLEVEL_HIGH;
	return pinVal;
}

static E_UWBUCISPIPinLevel spi_get_rdy_line_level(void)
{
	E_UWBUCISPIPinLevel 	pinVal;
	u8 portVal;
	//portVal = PINS_DRV_ReadPins(UWB_SPI_RDY.GpioBase);
	portVal = GPIO_PinRead (UWB_SPI_RDY.GpioBase, UWB_SPI_RDY.PinsIdx);
	// pinVal =(0u == (portVal & (pins_channel_type_t) 1u << (UWB_SPI_RDY.PinsIdx))) ?
	// 				UCI_SPI_PINLEVEL_LOW : UCI_SPI_PINLEVEL_HIGH;
	pinVal = (0u == portVal ) ?	UCI_SPI_PINLEVEL_LOW : UCI_SPI_PINLEVEL_HIGH;
	return pinVal;
}

static bool uwb_is_transmit_complate(void)
{
	E_UWBUCISPIPinLevel 	pinVal;
	u8 	portVal;

	portVal = GPIO_PinRead (UWB_SPI_SS.GpioBase, UWB_SPI_SS.PinsIdx);

	if(UCI_SPI_PINLEVEL_CS_INACTIVE == portVal)
		return true;
	else
		return false;
}

uint32_t spi_half_duplex_send(uint8_t* senddat, uint16_t sendlen)
{
	// status_t status = SPI_MasterTransfer(UWB_SPI_MASTER_INSTANCE, senddat, uwb_spi_recv_buffer, sendlen);
	// SPI_GetStatus(UWB_SPI_MASTER_INSTANCE);
	// if(STATUS_SUCCESS == status)
	// {
	// 	return UWB_Err_Success_0;
	// }
	spi_tx_data_UWB_Temp(senddat,sendlen);
	return UWB_Err_Success_0;
//	if(STATUS_SUCCESS == SPI_GetStatus(SPI_OVER_LPSPI00_INSTANCE))
//	{
//		return UWB_Err_Success_0;
//	}
//	else
//	{
//		//这里如果超时的话要考虑spi死了的情况，SPI通信Reset
//		return UWB_Err_DRIV_SPI_Send_Failed;
//	}
	//spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);



	//return status;

}

uint32_t spi_half_duplex_recv(uint8_t* recvdat, uint16_t recvlen)
{
	if(0 == recvlen)
	{
		return 0;
	}
	spi_rx_data_UWB_Temp((void*)UWB_SPI_NULL_BYTE, recvdat, recvlen);
	return UWB_Err_Success_0;
	// status_t status = SPI_MasterTransfer(UWB_SPI_MASTER_INSTANCE, (void*)UWB_SPI_NULL_BYTE, recvdat, recvlen);
	// SPI_GetStatus(UWB_SPI_MASTER_INSTANCE);
	// //LOG_OUT_HEX("SPI <- :", uwb_spi_recv_buffer, recvlen);
	// if(STATUS_SUCCESS == status)
	// {
	// 	return UWB_Err_Success_0;
	// }

	// //这里如果超时的话要考虑spi死了的情况，SPI通信Reset
	// //return UWB_Err_DRIV_SPI_Recv_Failed;



	// return status;
}

static uint32_t spi_abort_transceive(void)
{
	dspi_master_handle_t dspi_master_handle;
	// status_t status = LPSPI_DRV_MasterAbortTransfer(UWB_SPI_MASTER_INSTANCE);

	// if(STATUS_SUCCESS == status)
	// {
	// 	return (uint32_t)UWB_Err_Success_0;
	// }
	// else
	// {
	// 	return UWB_Err_DRIV_SPI_AbortTransmit_Failed;
	// }

	// return status;
	DSPI_MasterTransferAbort(SPI1, &dspi_master_handle);
	return (uint32_t)UWB_Err_Success_0;
}

static uint32_t spi_get_transceive_status(void)
{
	// status_t status = SPI_GetStatus(UWB_SPI_MASTER_INSTANCE);

	// if(STATUS_SUCCESS == status)
	// {
	// 	return (uint32_t)UWB_Err_Success_0;
	// }
	// else
	// {
	// 	return (uint32_t)UWB_Err_COMMU_The_Line_Still_Bussy;
	// }
	return (uint32_t)UWB_Err_Success_0;
}

//UWB comu time out single
static volatile bool uwb_commu_timer_out_flag = false;

static void ISR_uwb_commu_timer(void* userData)
{
	if(NULL != userData) (void*)userData;
	uwb_commu_timer_out_flag = true;
}


static void set_uwb_commu_timeout_count_start(ST_TimerTools* p, uint32_t millisecond)
{
	uint64_t input = millisecond * 1000000;

	uwb_commu_timer_out_flag = false;

	p->fpCreateTimer(TIMER_CHAN_TYPE_ONESHOT, UWB_LPIT_TIMER2, (fp_Timer_Handler_t)&ISR_uwb_commu_timer, (void*)NULL);

	p->fpStartTimer(UWB_LPIT_TIMER2, input);
}

static void stop_uwb_commu_timeout_count(ST_TimerTools* p)
{
	p->fpStopTimer(UWB_LPIT_TIMER2);

	uwb_commu_timer_out_flag = false;
}


/****
 * spi init
 */

static uint32_t uwb_commu_init(void)
{
	/*by manual transmission
	//Set Direction
	PINS_DRV_SetPinsDirection(UWB_SPI_INT.GpioBase,		UWB_SET_IO_DIRC_IN(UWB_SPI_INT.PinsIdx));
	PINS_DRV_SetPinsDirection(UWB_SPI_MISO.GpioBase,	UWB_SET_IO_DIRC_IN(UWB_SPI_MISO.PinsIdx));
	PINS_DRV_SetPinsDirection(UWB_SPI_RDY.GpioBase,		UWB_SET_IO_DIRC_IN(UWB_SPI_RDY.PinsIdx));

	PINS_DRV_SetPinsDirection(UWB_SPI_SS.GpioBase, 		UWB_SET_IO_DIRC_OUT(UWB_SPI_SS.PinsIdx));
	PINS_DRV_SetPinsDirection(UWB_SPI_SCK.GpioBase,		UWB_SET_IO_DIRC_OUT(UWB_SPI_SCK.PinsIdx));
	PINS_DRV_SetPinsDirection(UWB_SPI_MOSI.GpioBase, 	UWB_SET_IO_DIRC_OUT(UWB_SPI_MOSI.PinsIdx));
	PINS_DRV_SetPinsDirection(UWB_SPI_nRST.GpioBase,	UWB_SET_IO_DIRC_OUT(UWB_SPI_nRST.PinsIdx));

	//Set GPIO to IDEL
	PINS_DRV_SetPins(UWB_SPI_nRST.GpioBase,				UWB_SPI_IDEL_MODE(UWB_SPI_nRST.PinsIdx));
	PINS_DRV_SetPins(UWB_SPI_SS.GpioBase, 				UWB_SPI_IDEL_MODE(UWB_SPI_SS.PinsIdx));
	PINS_DRV_SetPins(UWB_SPI_SCK.GpioBase,				UWB_SPI_IDEL_MODE(UWB_SPI_SCK.PinsIdx));
	PINS_DRV_SetPins(UWB_SPI_SS.GpioBase, 				UWB_SPI_IDEL_MODE(UWB_SPI_SS.PinsIdx));
	 */

	#if defined(NXP_UCI_FRAME)
	cmd_state = UciCommuState_Idle;
	res_state = UciCommuState_Idle;
	#endif

	//SPI_MasterInit(UWB_SPI_MASTER_INSTANCE, &spi1_MasterConfig0);

	KW38_SPI1_Config ();

#if defined(DEBUG_CODE)
#if defined(NXP_UCI_FRAME)
	LOG_L_S(CCC_MD,"Boot Begin UCI.");

	SPI_MasterInit(UWB_SPI_MASTER_INSTANCE, &spi1_MasterConfig0);

	spi_set_rst_line_level(UCI_SPI_PINLEVEL_RST_INACTIVE);

	st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(3));

	spi_set_rst_line_level(UCI_SPI_PINLEVEL_RST_ACTIVE);

	st_timer_tool.ptr_delay(NS_PERIOD_UWB_REST_TIME_5_4_MS);

	spi_set_rst_line_level(UCI_SPI_PINLEVEL_RST_INACTIVE);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	//Recv NTF status
	while(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};
	spi_half_duplex_recv(uwb_spi_recv_buffer,SPI_RECV_BUF_MAX_LENS);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);

	LOG_L_S(CCC_MD,"Boot Over UCI.");

	LOG_L_S(CCC_MD,"uwb_commu_reset Being UCI. ");

	//Send Rest CMD
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_RDY_INACTIVE == spi_get_rdy_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};
	uwb_spi_send_buffer[0] = 0x20;
	uwb_spi_send_buffer[1] = 0x00;
	uwb_spi_send_buffer[2] = 0x00;
	uwb_spi_send_buffer[3] = 0x01;
	uwb_spi_send_buffer[4] = 0x00;
	spi_half_duplex_send(uwb_spi_send_buffer,5);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);


	//Recv Rest RESP
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};
	spi_half_duplex_recv(uwb_spi_recv_buffer,SPI_RECV_BUF_MAX_LENS);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);

	//Recv NTF Status
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};
	spi_half_duplex_recv(uwb_spi_recv_buffer,SPI_RECV_BUF_MAX_LENS);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);


	//Send Get Version
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_RDY_INACTIVE == spi_get_rdy_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};
	uwb_spi_send_buffer[0] = 0x20;
	uwb_spi_send_buffer[1] = 0x02;
	uwb_spi_send_buffer[2] = 0x00;
	uwb_spi_send_buffer[3] = 0x00;
	spi_half_duplex_send(uwb_spi_send_buffer,4);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);

	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};
	spi_half_duplex_recv(uwb_spi_recv_buffer,SPI_RECV_BUF_MAX_LENS);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);

#elif defined(NXP_RCI_FRAME)

	LOG_L_S(CCC_MD,"Boot Begin RCI.");

	spi_set_rst_line_level(UCI_SPI_PINLEVEL_RST_ACTIVE);

	st_timer_tool.ptr_delay(NS_PERIOD_UWB_REST_TIME_5_4_MS);

	spi_set_rst_line_level(UCI_SPI_PINLEVEL_RST_INACTIVE);

	SPI_MasterInit(UWB_SPI_MASTER_INSTANCE, &spi1_MasterConfig0);
	while(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);

	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	spi_half_duplex_recv(uwb_spi_recv_buffer,SPI_RECV_BUF_MAX_LENS);

	LOG_L_S(CCC_MD,"Boot Over RCI.");

	uint16_t crc16 = 0;

	LOG_L_S(CCC_MD,"uwb_commu_reset Being RCI. ");

	//Send Rest CMD
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_RDY_INACTIVE == spi_get_rdy_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};
	core_mm_set(uwb_spi_send_buffer, 0, SPI_SEND_BUF_MAX_LENS);
	uwb_spi_send_buffer[0] = 0x08;
	uwb_spi_send_buffer[1] = 0x01;
	uwb_spi_send_buffer[2] = 0x3A;
	uwb_spi_send_buffer[3] = 0x04;

	//LSB
	crc16 = do_crc_table(uwb_spi_send_buffer, SPI_SEND_BUF_MAX_LENS-2);
	uwb_spi_send_buffer[SPI_SEND_BUF_MAX_LENS - 1] = (uint8_t)((crc16&0xFF00)>>8);
	uwb_spi_send_buffer[SPI_SEND_BUF_MAX_LENS - 2] = (uint8_t)(crc16&0x00FF);

	spi_half_duplex_send(uwb_spi_send_buffer, SPI_SEND_BUF_MAX_LENS);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);


	//Recv Reset RES
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};

	spi_half_duplex_recv(uwb_spi_recv_buffer, SPI_RECV_BUF_MAX_LENS);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);

	//Get Version
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_RDY_INACTIVE == spi_get_rdy_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};

	core_mm_set(uwb_spi_send_buffer, 0, SPI_SEND_BUF_MAX_LENS);

	uwb_spi_send_buffer[0] = 0x08;
	uwb_spi_send_buffer[1] = 0x00;
	uwb_spi_send_buffer[2] = 0x3A;
	uwb_spi_send_buffer[3] = 0x00;
	uwb_spi_send_buffer[4] = 0x80;

	//LSB
	crc16 = 0;
	crc16 = do_crc_table(uwb_spi_send_buffer, SPI_SEND_BUF_MAX_LENS-2);
	uwb_spi_send_buffer[SPI_SEND_BUF_MAX_LENS - 1] = (uint8_t)((crc16&0xFF00)>>8);
	uwb_spi_send_buffer[SPI_SEND_BUF_MAX_LENS - 2] = (uint8_t)(crc16&0x00FF);
	spi_half_duplex_send(uwb_spi_send_buffer, SPI_SEND_BUF_MAX_LENS);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);

	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	while(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
	{
		st_timer_tool.ptr_delay(NS_PERIOD_BY_MS(1));
	};

	spi_half_duplex_recv(uwb_spi_recv_buffer, SPI_RECV_BUF_MAX_LENS);
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
	LOG_L_S(CCC_MD,"uwb_commu_reset End . ");
#endif
#endif
	return 0;
}

/****
 * uwb communication interface hard reset
 * 0 - Success , other failed
 */
static uint32_t uwb_commu_reset(ST_TimerTools* pst_timer_tools, uint8_t* recv_buff_ptr, uint16_t* length)
{
	E_UWBErrCode	ResCode = UWB_Err_Success_0;
	uint16_t		i = 0x00;
	uint16_t		j = 0x00;
	//
//	__disable_irq();
//	__asm("NOP");
//	__asm("NOP");
//	__asm("NOP");
	spi_set_rst_line_level(UCI_SPI_PINLEVEL_RST_ACTIVE);

	pst_timer_tools->fpDelay(NS_PERIOD_UWB_REST_TIME_5_4_MS);

//	pst_timer_tools->fpOSDelay(6);
	// for(i=0;i<500;i++)
	// {
	// 	for(j=0;j<108;j++)
	// 	{

	// 	}
	// }

	spi_set_rst_line_level(UCI_SPI_PINLEVEL_RST_INACTIVE);

	pst_timer_tools->fpDelay(NS_PERIOD_BY_MS(3));
//	pst_timer_tools->fpOSDelay(3);
//	for(i=0;i<500;i++)
//	{
//		for(j=0;j<50;j++)
//		{
//
//		}
//	}
//	__enable_irq();
	while(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level());
		//Recv NTF status
	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
	ResCode = spi_half_duplex_recv(recv_buff_ptr,6);


	if(UWB_Err_Success_0 == ResCode)
	{
		//while(STATUS_SUCCESS != SPI_GetStatus(SPI_OVER_LPSPI00_INSTANCE));
		spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
		//LOG_OUT_HEX("UCI Reset RES [Device Status NTF]:", recv_buff_ptr, 6);
		*length = 6;
		return ResCode;
	}
	else
	{
		return ResCode;
	}
}

/****
 * send cmd to uwb module
 */
static uint32_t uwb_commu_send(ST_TimerTools* pst_timer_tools, const uint8_t* send_buff_ptr, const uint16_t length)
{
	E_UWBErrCode		ResCode 				= UWB_Err_Success_0;
	bool 				IsDone 					= false;
	const uint16_t		CrashProtectionTimeOut 	= UCI_CMD_PROCESS_MAX_TIME_BY_MILLISECOND;

	//status check
	if(UciCommuState_Idle != cmd_state)
	{
		ResCode = UWB_Err_COMMU_Incorrect_Status;
		return ResCode;
	}

	//Null pointer check
	if(NULL == send_buff_ptr)
	{
		ResCode = UWB_Err_Null_Data_Pointer;
		return ResCode;
	}

	if(SPI_BUF_MAX_LENS < length)
	{
		ResCode = UWB_Err_COMMU_Send_Lens_Illegal;
		return ResCode;
	}

	if(0 == length)
	{
		ResCode = UWB_Err_COMMU_Send_Lens_Illegal;
		return ResCode;
	}

	core_mm_copy(uwb_spi_send_buffer, (uint8_t*)send_buff_ptr, length);

	do
    {
		switch(cmd_state)
    	{
    		case	UciCommuState_Idle:						//Timer on
    														if(UCI_SPI_PINLEVEL_IRQ_ACTIVE == spi_get_int_line_level())
    														{
    															cmd_state = ResCode = UWB_Err_COMMU_Send_Cancel;
    															cmd_state = UciCommuState_Idle;
    															IsDone = true;
    														}
    														else
    														{
        														set_uwb_commu_timeout_count_start(
																		pst_timer_tools,
        																CrashProtectionTimeOut);
        														cmd_state = UciCommuState_WaitReady;
        														spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
    														}
    														break;
    		case	UciCommuState_WaitReady:

    			 	 	 	 	 	 	 	 	 	 	 	if(UCI_SPI_PINLEVEL_RDY_ACTIVE == spi_get_rdy_line_level())
    			 	 	 	 	 	 	 	 	 	 	 	{
    			 	 	 	 	 	 	 	 	 	 	 		cmd_state = UciCommuState_Start;
    			 	 	 	 	 	 	 	 	 	 	 	}
    			 	 	 	 	 	 	 	 	 	 	 	else if(UCI_SPI_PINLEVEL_IRQ_ACTIVE == spi_get_int_line_level())
    														{
    															cmd_state = ResCode = UWB_Err_COMMU_Send_Cancel;
    															cmd_state = UciCommuState_Idle;
    															IsDone = true;
    														}
    														break;

    		case	UciCommuState_Start:

    														if (UCI_SPI_PINLEVEL_RDY_ACTIVE == spi_get_rdy_line_level()
                												&& UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
                											{
                												ResCode = spi_half_duplex_send(uwb_spi_send_buffer, length);
                												if(UWB_Err_Success_0 == ResCode)
                												{
                													cmd_state = UciCommuState_WaitTransferComplete;
                												}
                												else
                												{
                													cmd_state = UciCommuState_Error;
                												}

                											}
                											else if (UCI_SPI_PINLEVEL_RDY_INACTIVE == spi_get_rdy_line_level()
                												&& UCI_SPI_PINLEVEL_IRQ_ACTIVE == spi_get_int_line_level())
                											{
                												cmd_state = UciCommuState_Cancelled;
                											}
                											else if (UCI_SPI_PINLEVEL_RDY_ACTIVE == spi_get_rdy_line_level()
                												&& UCI_SPI_PINLEVEL_IRQ_ACTIVE == spi_get_int_line_level())
                											{
                												cmd_state = UciCommuState_Cancelled;
                											}
                											else
                											{
                												//Will never go into this place .
                												ResCode 	= UWB_Err_DRIV_Line_State_Exception;
                												cmd_state 	= UciCommuState_Error;
                											}
    														break;
    		case	UciCommuState_WaitTransferComplete:
    														while(!uwb_commu_timer_out_flag)
    														{
    															ResCode = spi_get_transceive_status();
    															if(UWB_Err_Success_0 == ResCode)
    															{
        															spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
        															IsDone = true;
        															cmd_state = UciCommuState_Idle;
        															//LOG_OUT_HEX("UCI SendCMD -> : ", uwb_spi_send_buffer, length);
        															break;
    															}
    															else
    															{
    																ResCode = UWB_Err_COMMU_Send_Line_Always_Bussy;
    															}
    														}
    														spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
    														break;
    		case	UciCommuState_Cancelled:
    														ResCode = spi_abort_transceive();
    														if(UWB_Err_Success_0 == ResCode)
    														{
    															ResCode = UWB_Err_COMMU_Send_Cancel;
    														}
    														else
    														{
    															ResCode = UWB_Err_COMMU_Send_Abort_Failed;
    														}
															spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
															cmd_state = UciCommuState_Idle;
															IsDone = true;
    														break;
    		case	UciCommuState_Error:
															spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
															IsDone = true;
															cmd_state = UciCommuState_Idle;
    														break;
    		case	UciCommuState_Undefined:
    		default:
    														ResCode = UWB_Err_COMMU_Default_Unkonw_Error;
															spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
															IsDone = true;
															cmd_state = UciCommuState_Idle;
															break;
    	}

    }while((!IsDone) && (!uwb_commu_timer_out_flag));

	//Time out
	if(uwb_commu_timer_out_flag)
	{
		ResCode = spi_abort_transceive();
		if(UWB_Err_Success_0 == ResCode)
		{
			LOG_L_S(CCC_MD,"spi send time out abort !");
			// do nothing .
		}
		else
		{
			ResCode = UWB_Err_DRIV_Send_Time_Out_Abort_Failed;
		}
		spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
		cmd_state = UciCommuState_Idle;
	}
	else
	{
		//Timer off
		stop_uwb_commu_timeout_count(pst_timer_tools);
	}

	return ResCode;
}

/****
 * recv response from uwb module .
 */
static uint32_t uwb_commu_recv(ST_TimerTools* pst_timer_tools, uint8_t* recv_buff_ptr, uint16_t* length)
{
	E_UWBErrCode 		ResCode 				= UWB_Err_Success_0;
	bool 				IsDone 					= false;
	const uint32_t		CrashProtectionTimeOut 	= UCI_CMD_PROCESS_MAX_TIME_BY_MILLISECOND;
	uint8_t				HeaderLens				= UCISW_HEADER_LENGTH;//on
	uint8_t				BodyLens				= 0;

	//check the status .
	if(UciCommuState_Idle != res_state)
	{
		ResCode = UWB_Err_COMMU_Incorrect_Status;
		return ResCode;
	}

	//checking the null pointer .
	if(NULL == recv_buff_ptr)
	{
		ResCode = UWB_Err_Null_Data_Pointer;
		return ResCode;
	}

	do{
		switch(res_state)
		{
			case UciCommuState_Idle:
															set_uwb_commu_timeout_count_start(
																	pst_timer_tools,
																	CrashProtectionTimeOut);
															res_state = UciCommuState_WaitReady;
															break;
			case UciCommuState_WaitReady:
															if (UCI_SPI_PINLEVEL_IRQ_ACTIVE == spi_get_int_line_level())
															{
																spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
																res_state = UciCommuState_WaitTransferHeader;
															}
															break;
			case UciCommuState_WaitTransferHeader:
															ResCode = spi_half_duplex_recv(uwb_spi_recv_buffer, HeaderLens);

															if(UWB_Err_Success_0 == ResCode)
															{
																while(!uwb_commu_timer_out_flag)
																{
																	if(UWB_Err_Success_0 == spi_get_transceive_status())
																	{
																		core_mm_copy(	recv_buff_ptr,
																						uwb_spi_recv_buffer,
																						UCISW_HEADER_LENGTH);
																		if(0x80 == (recv_buff_ptr[2]&0x80))//single byte lens or double bytes lens judge .
																		{
																			BodyLens = core_dcm_mku16(recv_buff_ptr[4],recv_buff_ptr[3]);
																		}
																		else
																		{
																			BodyLens = (uint16_t)recv_buff_ptr[4];
																		}

																		if(0 != BodyLens)
																		{
																			res_state = UciCommuState_WaitTransferComplete;
																			//goto recv body .
																		}
																		else
																		{
																			res_state = UciCommuState_Idle;
																			IsDone = true;
																			//no body date .
																		}
																		break;
																	}
																	else
																	{
																		//do nothing .
																	}
																}
															}
															else
															{
																res_state = UciCommuState_Error;
																spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
															}
															//spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
															break;

			case UciCommuState_WaitTransferComplete:
															//spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_ACTIVE);
															ResCode = spi_half_duplex_recv(uwb_spi_recv_buffer, BodyLens);

															if(UWB_Err_Success_0 == ResCode)
															{
																while(!uwb_commu_timer_out_flag)
																{
																	if(UWB_Err_Success_0 == spi_get_transceive_status())
																	{
//																		spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);

																		while(!uwb_commu_timer_out_flag)
																		{
																			if(UCI_SPI_PINLEVEL_IRQ_INACTIVE == spi_get_int_line_level())
																			{
																				spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
																				break;
																			}
																			else
																			{
																				//do nothing .
																			}
																		}
																		core_mm_copy(	recv_buff_ptr + UCISW_HEADER_LENGTH,
																						uwb_spi_recv_buffer,
																						BodyLens);
																		*length = (uint16_t)UCISW_HEADER_LENGTH + BodyLens;
																		res_state = UciCommuState_Idle;
																		IsDone = true;
																		//LOG_OUT_HEX("UCI Recv <- :", recv_buff_ptr, *length);
																		break;
																	}
																	else
																	{
																		//do nothing .
																	}
																}
															}
															else
															{
																ResCode = UWB_Err_COMMU_Recv_Data_Body_Failed;
																spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
															}
															break;

			case UciCommuState_Error:
															spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
															res_state = UciCommuState_Idle;
															IsDone = true;
															break;
			case UciCommuState_Undefined:
			default:
															spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
															ResCode = UWB_Err_COMMU_Default_Unkonw_Error;
															res_state = UciCommuState_Idle;
															IsDone = true;
															break;
		}
	}while((!IsDone) && (!uwb_commu_timer_out_flag));

	spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
	//Time out
	if(uwb_commu_timer_out_flag)
	{
		ResCode = spi_abort_transceive();
		if(UWB_Err_Success_0 == ResCode)
		{
			//do nothing .
		}
		else
		{
			ResCode = UWB_Err_DRIV_Recv_Time_Out_Abort_Failed;
		}

		cmd_state = UciCommuState_Idle;
	}
	else
	{
		//Timer off
		stop_uwb_commu_timeout_count(pst_timer_tools);
	}
	return ResCode;
}

/****
 * communication module sleep .
 */
static uint32_t uwb_commu_sleep(void)
{
	return 0;
}

/****
 * communication module wake up .
 */
static uint32_t uwb_commu_wakeup(void)
{
	return 0;
}

static bool uwb_commu_is_notify_coming(void)
{
	if(UCI_SPI_PINLEVEL_IRQ_ACTIVE == spi_get_int_line_level())
	{
		//spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
		return true;
	}
	else
	{
		//spi_set_csn_line_level(UCI_SPI_PINLEVEL_CS_INACTIVE);
		return false;
	}
}
static uint32_t uwb_commu_deinit(void)
{
	#if defined(NXP_UCI_FRAME)
	cmd_state = UciCommuState_Undefined;
	res_state = UciCommuState_Undefined;
	#endif

//	SPI_MasterDeinit(UWB_SPI_MASTER_INSTANCE);
	//pstUWBCommu = NULL;

	return 0;
}


/****
 * @brief Anchor point and MCU communication interface initialization .
 * @return 0 success, other failed
 */
uint32_t API_UWB_Commu_Module_Init(ST_UWBCommuAttr* pst_uwb_commu)
{
	//Problem 1. Need the NULL pointer check in this here ..

	pst_uwb_commu->fpCmmuInit 		=  	(fp_UWB_Cmmu_Init_t)		(&uwb_commu_init);
	pst_uwb_commu->fpCmmuReset 		= 	(fp_UWB_Cmmu_Reset_t)		(&uwb_commu_reset);
	pst_uwb_commu->fpCmmuSend 		=  	(fp_UWB_Cmmu_Send_t)		(&uwb_commu_send);
	pst_uwb_commu->fpCmmuRecv 		=  	(fp_UWB_Cmmu_Recv_t)		(&uwb_commu_recv);
	pst_uwb_commu->fpCmmuSleep 		= 	(fp_UWB_Cmmu_Sleep_t)		(&uwb_commu_sleep);
	pst_uwb_commu->fpCmmuWakeup 	=	(fp_UWB_Cmmu_Wake_up_t)		(&uwb_commu_wakeup);
	pst_uwb_commu->fpCmmuDeinit		=	(fp_UWB_Cmmu_Deinit_t)		(&uwb_commu_deinit);
	pst_uwb_commu->fpCmmuIsntfcomin	=	(fp_UWB_Cmmu_Isntfcomin_t)	(&uwb_commu_is_notify_coming);
	pst_uwb_commu->fpCmmuIsTransmitComplate = (fp_UWB_Cmmu_IsTransmitComplate_t)(&uwb_is_transmit_complate);
	return UWB_Err_Success_0;
}

uint32_t API_UWB_Commu_Module_Deinit(ST_UWBCommuAttr* pst_uwb_commu)
{

	pst_uwb_commu->fpCmmuInit 		= NULL;
	pst_uwb_commu->fpCmmuReset 		= NULL;
	pst_uwb_commu->fpCmmuSend 		= NULL;
	pst_uwb_commu->fpCmmuRecv 		= NULL;
	pst_uwb_commu->fpCmmuSleep 		= NULL;
	pst_uwb_commu->fpCmmuWakeup 	= NULL;
	pst_uwb_commu->fpCmmuDeinit		= NULL;
	pst_uwb_commu->fpCmmuIsntfcomin	= NULL;
	pst_uwb_commu->fpCmmuIsTransmitComplate = NULL;
	//stptr = NULL;
	uwb_commu_deinit();

	return UWB_Err_Success_0;
}
#if 0 
abandoned code
st_uwb_commu_attr_t 					st_uwb_commu_attr;
st_uwb_commu_attr_t* pstUWBCommu = NULL;


#endif
