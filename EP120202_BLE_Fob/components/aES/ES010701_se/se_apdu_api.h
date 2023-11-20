
/* Includes -----------------------------------------------------------------*/
#ifndef __APDU_API_H__
#define __APDU_API_H__

#include "se_iso7816_3_t1.h"
#include "se_transport.h"
#include "se_common_type.h"

/**
 * @brief
 *      获取SDK版本；
 * 
 * @param [pOutVer] SDK版本号存放地址
 * 
 * @return SDK版本号长度
 * 
 * @note
 *      获取SDK版本;
 */
u8_t api_get_sdk_version(u8_t* pOutVer);

/**
 * @brief
 *      上电初始化接口
 * 
 * @param [hsm_ctrl_attr] 集成方提供的访问SE的SPI驱动，详见hsm_ctrl_attr_t描述
 * @note
 *      此接口用于安全芯片通信的上电初始化
 */
void api_power_on_init(void);

/**
 * @brief
 *      组织APDU发送，并接受响应数据
 *
 * @param [sendBuffer] 发送数据指针
 * @param [sendLength] 发送数据长度
 * @param [recvRspBuffer] 接收数据指针
 * @param [recvLength] 接收数据长度，可设置为recvRspBuffer的空间大小；
 *
 * @return 返回接受到数据的长度，若
 *       >= 2: SE正常响应的数据长度；
 *       == 0: SE响应异常，但进行了一次RST重试（会调用集成方提供的cb_hsm_reset_t)，并恢复了正常，但本条APDU响应是异常的。由于安全芯片复位，后面集成方需要进行重新选择应用；
 *       others：则表示SE通信失败，并进行了RST重试仍然失败，可能原因SE异常；
 * @note
 *      此接口用于组织好的APDU数据的发送与响应的接收
 *
 *
 */
int api_apdu_transceive(u8_t* sendBuffer, u16_t sendLength,u8_t* recvRspBuffer, u16_t recvLength);

#endif
