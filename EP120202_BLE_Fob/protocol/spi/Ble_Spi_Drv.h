#ifndef _BLE_SPI_DRV_H_
#define _BLE_SPI_DRV_H_

#include "stdint.h"
/*************************************************************************************
**************************************************************************************
* Public macros
*
**************************************************************************************
*************************************************************************************/
#define EXAMPLE_DSPI_MASTER_BASEADDR 			SPI0
#define EXAMPLE_DSPI_MASTER_CLK_SRC 			DSPI0_CLK_SRC
#define EXAMPLE_DSPI_MASTER_CLK_FREQ 			CLOCK_GetFreq(DSPI0_CLK_SRC)
#define EXAMPLE_DSPI_MASTER_PCS 				kDSPI_Pcs0
#define EXAMPLE_DSPI_MASTER_PCS_eSE 			kDSPI_Pcs1
#define EXAMPLE_DSPI_MASTER_IRQ 				SPI0_IRQn
#define EXAMPLE_DSPI_MASTER_IRQHandler 			SPI0_IRQHandler
#define EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER 	kDSPI_MasterPcs0


#define TRANSFER_SIZE 8U         				/*! Transfer dataSize */
#define TRANSFER_BAUDRATE 4000000U 				/*! Transfer baudrate - 4000k */


#define BLE_SPI_USED_IDLE     0U
#define BLE_SPI_USED_ESE      2U


#define UWB_DSPI_MASTER_BASEADDR 			SPI1
#define UWB_DSPI_MASTER_CLK_SRC 			DSPI1_CLK_SRC
#define UWB_DSPI_MASTER_CLK_FREQ 			CLOCK_GetFreq(DSPI1_CLK_SRC)
#define UWB_DSPI_MASTER_PCS 				kDSPI_Pcs0
#define UWB_DSPI_MASTER_IRQ 				SPI1_IRQn
#define UWB_DSPI_MASTER_IRQHandler 			SPI1_IRQHandler
#define UWB_DSPI_MASTER_PCS_FOR_TRANSFER 	kDSPI_MasterPcs0




/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/
void Spi_User_Init_eSE(void);
void spi_tx_data_eSE(uint8_t data);
void spi_rx_data_eSE_Temp(uint8_t *rxdata);
void spi_tx_data_eSE_Temp(uint8_t *txdata);

void spi_rx_data_eSE_Temp2(uint8_t *rxdata, uint16_t length);
void spi_tx_data_eSE_Temp2(uint8_t *txdata,uint16_t length);




void spi_tx_data_UWB_Temp(uint8_t *txdata,uint16_t length);
void spi_rx_data_UWB_Temp(uint8_t *txdata,uint8_t *rxdata,uint16_t length);

#endif /* _APP_H_ */
