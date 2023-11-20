/*
 * adpu.c
 *
 *  Created on: 2020年8月25日
 *      Author: liyupeng
 */

/* Includes -----------------------------------------------------------------*/
#include "se_iso7816_3_t1.h"
#include "se_transport.h"
#include "se_common_type.h"
#include "Board_Config.h"
//#include "Ble_Gpio_Drv.h"
#include "Ble_Spi_Drv.h"
#include "se_apdu_api.h"

int impl_hsm_init(void)
{
	SE_PowerOff();
	Ble_ESE_CS_HIGH();
 	Spi_User_Init_eSE();
 	OSA_TimeDelay(20);
 	Ble_ESE_PowerOn();
	SE_PowerOn();
 	OSA_TimeDelay(20);
    return 0;
}
/*Reset ESE function*/
volatile uint8_t geSESpiUseFlg = 0;
volatile uint8_t geSESpiRecvOverFlg = 0;
 int impl_hsm_reset(void)
{
	if(geSESpiUseFlg == 1)
	{
		OSA_TimeDelay(4);
	}
	geSESpiUseFlg = 1;
	OSA_TimeDelay(2);
	Ble_ESE_PowerOff();
	SE_PowerOff();
	Ble_ESE_CS_LOW();
    OSA_TimeDelay(2);
    Ble_ESE_CS_HIGH();
    Ble_ESE_PowerOn();
	SE_PowerOn();
    geSESpiUseFlg = 0;
    return 0;
}

void impl_hsm_delay(u32_t ms)
{
	OSA_TimeDelay(ms);
    return ;
}
 /* HSM spi receive */
 int impl_hsm_spi_recvieve(u8_t *rbuff, u16_t length)
 {
 	uint16_t recv_cnt;
 	uint8_t buffer = 0;

  	geSESpiUseFlg = 1;
  	Ble_ESE_CS_LOW();
// 	OSA_TimeDelay(1);//at least 120 us before transmitting data
 	for (recv_cnt = 0; recv_cnt < length; recv_cnt++)
 	{
 		spi_rx_data_eSE_Temp(&buffer);
 		rbuff[recv_cnt] = buffer;
 	}
 	Ble_ESE_CS_HIGH();
 	geSESpiUseFlg = 0;
 	geSESpiRecvOverFlg = 1;
 	return recv_cnt;

 }
 /* HSM spi send */
 int impl_hsm_spi_send(u8_t *tbuff, u16_t length)
 {
	uint16_t send_cnt;

	if(geSESpiRecvOverFlg == 1)
	{
		OSA_TimeDelay(1);
		geSESpiRecvOverFlg = 0;
	}
 	geSESpiUseFlg = 1;
// 	OSA_TimeDelay(1);
 	spi_tx_data_eSE_Temp2(tbuff,length);
 	geSESpiUseFlg = 0;

 	send_cnt = length;
 	return send_cnt;
 }


// void apdu_1_init(struct t1_state_t *t1, u8_t src, u8_t dst)
// {
// 	isot1_init(t1);
// 	isot1_bind(t1, src, dst);
// }


struct t1_state_t t1_attr;
hsm_ctrl_attr_t hsm_ctrl_attr;
/**< 上电初始化 */
void api_power_on_init(void)
{
//	hsm_ctrl_attr_t hsm_ctrl_attr;
	hsm_ctrl_attr.init = impl_hsm_init;
	hsm_ctrl_attr.reset = impl_hsm_reset;
	hsm_ctrl_attr.delay = impl_hsm_delay;
	hsm_ctrl_attr.recv = impl_hsm_spi_recvieve;
	hsm_ctrl_attr.send = impl_hsm_spi_send;
	t1_attr.pcfg = &hsm_ctrl_attr;

	if(t1_attr.pcfg->init)
	{
		t1_attr.pcfg->init();
	}
	apdu_1_init(&t1_attr, 2, 1);  /* NAD= 0x12 */
}

// int api_apdu_transceive(u8_t* sendBuffer, u16_t sendLength,u8_t* recvRspBuffer, u16_t recvLength)
// {
// 	return isot1_transceive(&t1_attr,sendBuffer,sendLength,recvRspBuffer,recvLength);
// }


u8_t api_get_sdk_version(u8_t* pOutVer)
{
	pOutVer[0] = SDK_VER_MAJOR;
	pOutVer[1] = SDK_VER_MINOR;

	return 0x02;
}

void apdu_1_init(struct t1_state_t *t1, u8_t src, u8_t dst)
{
	isot1_init(t1);
	isot1_bind(t1, src, dst);

#ifdef CONFIG_FEATURE_GP_SPI
	if(t1->pcfg->delay)
	{
		t1->pcfg->delay(100);
	}
	isot1_request_cip(t1);
	if(t1->pcfg->delay)
	{
		t1->pcfg->delay(100);
	}
	isot1_negotiate_ifsd(t1, t1->ifsc);

	//isot1_reset(t1);
#endif
}

void api_t1_recover()
{
	apdu_1_init(&t1_attr, 2, 1);  /* NAD= 0x12 */
}

static int func_se_reset()
{
	u8_t apdu[] = {0xFE, 0x31, 0x00, 0x00, 0x00};
	u8_t apduResp[0x20];
	if(t1_attr.pcfg->reset)
	{
		t1_attr.pcfg->reset();
		if(t1_attr.pcfg->delay)
		{
			t1_attr.pcfg->delay(100);
		}

		apdu_1_init(&t1_attr, 2, 1);  /* NAD= 0x12 */

		return api_apdu_transceive(apdu, 5, apduResp, 0x20);
	}
	else
	{
		return -1;
	}
}

int api_apdu_transceive(u8_t* sendBuffer, u16_t sendLength,u8_t* recvRspBuffer, u16_t recvLength)
{
	int dlCode = 0;
	int resLen = isot1_transceive(&t1_attr,sendBuffer,sendLength,recvRspBuffer,recvLength);
#if 1
	if(resLen < 2)
	{
		if(t1_attr.spiResetNum == 0)
        {
            t1_attr.spiResetStatus = 0;
            t1_attr.spiResetNum++;

            dlCode = func_se_reset();
            if(dlCode >= 2)
            {
				if(t1_attr.spiResetNum >= 1)
				{
					resLen = 0;  //第一次复位成功;
				}
                t1_attr.spiResetNum = 0;
            }
        }
        else
        {
            t1_attr.spiResetStatus = 1;
        }
	}
	else
	{
		if (t1_attr.spiResetStatus)
        {
            t1_attr.spiResetStatus = 0;
            //func_se_reset();
        }
	}
#endif

	return resLen;
}

