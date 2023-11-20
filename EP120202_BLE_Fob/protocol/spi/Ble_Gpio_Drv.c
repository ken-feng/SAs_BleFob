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
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "Ble_Gpio_Drv.h"

/* BEGIN_FUNCTION_HDR
***********************************************************************************************
* Function Name: Gpio_Init
*
* Description:   used for Gpio init.
*
* Inputs:       None
*
* Outputs:       None
*
* Limitations:
***********************************************************************************************
END_FUNCTION_HDR*/
/*GPIO INIT FUNCTION*/
void Gpio_Init(void)
{
    /* Define the init structure for the default gpio output pin*/
    gpio_pin_config_t gpio_output_config = {
        kGPIO_DigitalOutput, 0,
    };
    /* Define the init structure for the default gpio input pin*/
//    gpio_pin_config_t gpio_input_config = {
//    	kGPIO_DigitalInput, 0,
//    };

    GPIO_PinInit(BOARD_PTC_GPIO, BOARD_GPIOC_PIN5, &gpio_output_config);
    GPIO_PinWrite(BOARD_PTC_GPIO, BOARD_GPIOC_PIN5, 1);/*BUS LIN EN*/

    GPIO_PinInit(BOARD_PTB_GPIO, BOARD_GPIOB_PIN18, &gpio_output_config);
    GPIO_PinWrite(BOARD_PTB_GPIO, BOARD_GPIOB_PIN18, 0);/*SE RST*/
}

void Ble_ESE_PowerOn()
{
	// GPIO_PinWrite(BOARD_PTC_GPIO, BOARD_GPIOC_PIN5, 0);/*eSE powerON*/
}
void Ble_ESE_PowerOff()
{
	// GPIO_PinWrite(BOARD_PTC_GPIO, BOARD_GPIOC_PIN5, 1);/*eSE powerOff*/
}

void Ble_ESE_CS_HIGH()
{
	//GPIO_PinWrite(BOARD_PTC_GPIO, BOARD_GPIOC_PIN4, 1);/*eSE cs high*/
}
void Ble_ESE_CS_LOW()
{
	//GPIO_PinWrite(BOARD_PTC_GPIO, BOARD_GPIOC_PIN4, 0);/*eSE cs low*/
}
