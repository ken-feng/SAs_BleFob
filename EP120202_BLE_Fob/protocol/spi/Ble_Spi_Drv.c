/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of HiRain Technologies. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   HiRain Technologies.
************************************************************************************************
*   File Name       : SleepManagement.c
************************************************************************************************
*   Project/Product :
*   Title           :
*   Author          :
************************************************************************************************
*   Description     :
*
************************************************************************************************
*   Limitations     :
*
************************************************************************************************
*
************************************************************************************************
*   Revision History:
*
*    Version      Date          Initials      CR#          Descriptions
*   ---------   ----------    ------------  ----------  ---------------
*     1.0        2019/12/10   weitong.zhang     N/A          Original
************************************************************************************************
* END_FILE_HDR*/
#include "ble_general.h"
#include "portmacro.h"
#include "projdefs.h"
#include "fsl_dspi.h"
#include "fsl_gpio.h"

#include "Ble_Gpio_Drv.h"
#include "Ble_Spi_Drv.h"
#include "EM000101.h"
volatile bool pitIsrFlag = false;

volatile uint32_t masterTxCount;
volatile uint32_t masterRxCount;

static uint8_t lBle_Spi_Used_Flg = BLE_SPI_USED_IDLE;
/* BEGIN_FUNCTION_HDR
***********************************************************************************************
* Function Name: EXAMPLE_DSPI_MASTER_IRQHandler
*
* Description:   used for setting data initial value.
*
* Inputs:       None
*
* Outputs:       None
*
* Limitations:
***********************************************************************************************
END_FUNCTION_HDR*/
/*SPI MASTER IRQ FUNCTION*/
void EXAMPLE_DSPI_MASTER_IRQHandler(void)
{
    if (masterRxCount < TRANSFER_SIZE)
    {
        while (DSPI_GetStatusFlags(EXAMPLE_DSPI_MASTER_BASEADDR) & kDSPI_RxFifoDrainRequestFlag)
        {
  //      	masterRxData[masterRxCount] = DSPI_ReadData(EXAMPLE_DSPI_MASTER_BASEADDR);
            ++masterRxCount;

            DSPI_ClearStatusFlags(EXAMPLE_DSPI_MASTER_BASEADDR, kDSPI_RxFifoDrainRequestFlag);

            if (masterRxCount == TRANSFER_SIZE)
            {
                break;
            }
        }
    }

    if (masterTxCount < TRANSFER_SIZE)
    {
        //while ((DSPI_GetStatusFlags(EXAMPLE_DSPI_MASTER_BASEADDR) & kDSPI_TxFifoFillRequestFlag) &&
               //((masterTxCount - masterRxCount) < masterFifoSize))
    	while ((DSPI_GetStatusFlags(EXAMPLE_DSPI_MASTER_BASEADDR) & kDSPI_TxFifoFillRequestFlag))
        {
            if (masterTxCount < TRANSFER_SIZE)
            {
                //EXAMPLE_DSPI_MASTER_BASEADDR->PUSHR = masterCommand | masterTxData[masterTxCount];
 //           	 masterTxData[masterTxCount] = EXAMPLE_DSPI_MASTER_BASEADDR->PUSHR;
                ++masterTxCount;
            }
            else
            {
                break;
            }

            /* Try to clear the TFFF; if the TX FIFO is full this will clear */
            DSPI_ClearStatusFlags(EXAMPLE_DSPI_MASTER_BASEADDR, kDSPI_TxFifoFillRequestFlag);
        }
    }

    /* Check if we're done with this transfer.*/
    if ((masterTxCount == TRANSFER_SIZE) && (masterRxCount == TRANSFER_SIZE))
    {
   //Modify by zwt     isTransferCompleted = true;
        /* Complete the transfer and disable the interrupts */
        DSPI_DisableInterrupts(EXAMPLE_DSPI_MASTER_BASEADDR,
                               kDSPI_RxFifoDrainRequestInterruptEnable | kDSPI_TxFifoFillRequestInterruptEnable);
    }
}
/* BEGIN_FUNCTION_HDR
***********************************************************************************************
* Function Name: Spi_User_Init_eSE
*
* Description:   used for eSE init.
*
* Inputs:       None
*
* Outputs:       None
*
* Limitations:
***********************************************************************************************
END_FUNCTION_HDR*/
/*SPI INIT FUCNTION FOR ESE */
void Spi_User_Init_eSE(void)
{
	/* Master config */
	uint32_t srcClock_Hz;

	dspi_master_config_t masterConfig;
	masterConfig.whichCtar = kDSPI_Ctar0;
	masterConfig.ctarConfig.baudRate = TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.bitsPerFrame = 8U;
	masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
	masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
	masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
	masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 10000000000U / TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;

	masterConfig.whichPcs = EXAMPLE_DSPI_MASTER_PCS;
	masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

	masterConfig.enableContinuousSCK = false;
	masterConfig.enableRxFifoOverWrite = false;
	masterConfig.enableModifiedTimingFormat = false;
	masterConfig.samplePoint = kDSPI_SckToSin0Clock;

	srcClock_Hz = EXAMPLE_DSPI_MASTER_CLK_FREQ;
	srcClock_Hz = srcClock_Hz*2;
	DSPI_MasterInit(EXAMPLE_DSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);

}

/* BEGIN_FUNCTION_HDR
***********************************************************************************************
* Function Name: spi_tx_data_eSE
*
* Description:   used for eSE data tx.
*
* Inputs:       None
*
* Outputs:       None
*
* Limitations:
***********************************************************************************************
END_FUNCTION_HDR*/
/*SPI TRANSMIT FUCNTION FOR MCU AND ESE*/
void spi_tx_data_eSE(uint8_t data)
{

    dspi_transfer_t masterXfer;
    uint8_t masterTxData[2] = {0};

    masterTxData[0] = data;
    masterTxData[1] = (uint8_t)(data);

    masterXfer.txData = masterTxData;
    masterXfer.dataSize = 1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS | kDSPI_MasterPcsContinuous;
    DSPI_MasterTransferBlocking(EXAMPLE_DSPI_MASTER_BASEADDR, &masterXfer);

}


void spi_rx_data_eSE_Temp(uint8_t *rxdata)
{
    dspi_transfer_t masterXfer;
    if(BLE_SPI_USED_ESE!=lBle_Spi_Used_Flg)
    {
    	//Spi_User_Init_eSE();
    	lBle_Spi_Used_Flg = BLE_SPI_USED_ESE;
    }
    else
    {
    	/* do nothing */
    }

    masterXfer.txData = NULL;
    masterXfer.rxData = rxdata;
    masterXfer.dataSize = 1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_MasterTransferBlocking(EXAMPLE_DSPI_MASTER_BASEADDR, &masterXfer);
}

void spi_tx_data_eSE_Temp(uint8_t *txdata)
//void spi_tx_data_eSE_Temp(uint8_t *txdata,uint16_t length)
{
    dspi_transfer_t masterXfer;
    if(BLE_SPI_USED_ESE!=lBle_Spi_Used_Flg)
    {
    	//Spi_User_Init_eSE();
    	lBle_Spi_Used_Flg = BLE_SPI_USED_ESE;
    }
    else
    {
    	/* do nothing */
    }

    masterXfer.txData = txdata;
    masterXfer.rxData = NULL;
    masterXfer.dataSize =  1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_MasterTransferBlocking(EXAMPLE_DSPI_MASTER_BASEADDR, &masterXfer);
}

void spi_rx_data_eSE_Temp2(uint8_t *rxdata, uint16_t length)
{
    dspi_transfer_t masterXfer;
    if(BLE_SPI_USED_ESE!=lBle_Spi_Used_Flg)
    {
    	//Spi_User_Init_eSE();
    	lBle_Spi_Used_Flg = BLE_SPI_USED_ESE;
    }
    else
    {
    	/* do nothing */
    }

    masterXfer.txData = NULL;
    masterXfer.rxData = rxdata;
    masterXfer.dataSize = length;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_MasterTransferBlocking(EXAMPLE_DSPI_MASTER_BASEADDR, &masterXfer);
}

void spi_tx_data_eSE_Temp2(uint8_t *txdata,uint16_t length)
{
    dspi_transfer_t masterXfer;

    masterXfer.txData = txdata;
    masterXfer.rxData = NULL;
    masterXfer.dataSize =  length;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_MasterTransferBlocking(EXAMPLE_DSPI_MASTER_BASEADDR, &masterXfer);
}
void spi_txrx_data_eSE_Temp2(uint8_t *txdata,uint8_t *rxdata,uint16_t length)
{
    dspi_transfer_t masterXfer;

    masterXfer.txData = txdata;
    masterXfer.rxData = rxdata;
    masterXfer.dataSize =  length;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_MasterTransferBlocking(EXAMPLE_DSPI_MASTER_BASEADDR, &masterXfer);
}

void spi_tx_data_UWB_Temp(uint8_t *txdata,uint16_t length)
{
    dspi_transfer_t masterXfer;

    masterXfer.txData = txdata;
    masterXfer.rxData = NULL;
    masterXfer.dataSize =  length;
    masterXfer.configFlags = kDSPI_MasterCtar0 | UWB_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_MasterTransferBlocking(UWB_DSPI_MASTER_BASEADDR, &masterXfer);
}

void spi_rx_data_UWB_Temp(uint8_t *txdata,uint8_t *rxdata,uint16_t length)
{
    dspi_transfer_t masterXfer;

    masterXfer.txData = txdata;
    masterXfer.rxData = rxdata;
    masterXfer.dataSize =  length;
    masterXfer.configFlags = kDSPI_MasterCtar0 | UWB_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_MasterTransferBlocking(UWB_DSPI_MASTER_BASEADDR, &masterXfer);
}



void spi_uwb_test()
{
    u8 tempBuf[64];
    for (u8 i = 0; i < 64; i++)
    {
        tempBuf[i] = 0;
    }
    
    spi_tx_data_UWB_Temp(tempBuf,64);
}
