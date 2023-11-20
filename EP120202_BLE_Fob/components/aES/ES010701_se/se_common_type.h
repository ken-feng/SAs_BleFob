#ifndef __COMMON_TYPE_H__

#define __COMMON_TYPE_H__

/* Includes------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#define CONFIG_FEATURE_GP_SPI
#define SDK_VER_MAJOR    0x01
#define SDK_VER_MINOR    0x00
/*---------------------------------------------------------------------------*/
/**< 基本类似定义 */

typedef unsigned char           u8_t;
typedef unsigned short int      u16_t;
typedef unsigned long int       u32_t;

typedef signed char             s8_t;
typedef signed short int        s16_t;
typedef long int                s32_t;

typedef unsigned int            u_unit_t;       /**< 无符号 MCU访问数据单元 */
typedef s32_t                   s_unit_t;       /**< 有符号 MCU访问数据单元 */

typedef unsigned char           bool_t;

#define FALSE	0
#define TRUE	1

#define NULL	0
/**-----------------------------------------------------------------------*/
/**< 常用函数 */
/**< 交换字节 */
#define SWAP_BYTE(a, b) \
	a = (a) ^ (b); \
	b = (a) ^ (b); \
	a = (a) ^ (b);

/** 获取p_data的第n个数据 */
#define GET_BYTE(p_data, n) \
	(((p_data) >> (8 * (n))) & 0xFF)

/** p1与p2合成两字节 */
#define ADD16(p1, p2) \
	(((u16_t)(p1) << 8) | ((u16_t)p2))

/** p1/p2/p3/p4合成四字节 */
#define ADD32(p1, p2, p3, p4) \
	(((u32_t)(p1) << 24) | ((u32_t)(p2) << 16) | \
	((u32_t)(p3) << 8) | ((u32_t)p4))

/** 取x/y的最小值 */
#if !defined(MIN)
#define MIN(x, y) \
	((x) < (y) ? (x) : (y))
#endif

#if !defined(MAX)
/** 取x/y的最大值 */
#define MAX(x, y) \
	((x) > (y) ? (x) : (y))
#endif

#if !defined(ABS_SUB)
/** 取x/y的差值 */
#define ABS_SUB(x, y) \
    (((x) >= (y)) ? (x - y) : (y - x))
#endif

#if !defined(SET_BIT)
/** data的第n位置1 */
#define SET_BIT(data, n) \
    (data |= (1U << (n)))
#endif

#if !defined(CLEAR_BIT)
/** data的第n位清0 */
#define CLEAR_BIT(data, n) \
    (data &= (~(1U << (n))))
#endif
#if !defined(GET_BIT)
/** Get Bit Value */
#define GET_BIT(data, n) \
    (data & (1U << (n)))
#endif

/**
 * @brief
 *      硬件加密模块初始化
 *
 *
 * @return
 * 	    若成功则返回0，否则返回失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于初始化SE.
 *
 *
 */
typedef int (*cb_hsm_init_t)(void);


/**
 * @brief
 *      硬件加密模块复位
 *
 *
 * @return
 * 	    若成功则返回0，否则返回失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于硬件加密模块的复位操作
 *
 *
 */
typedef int (*cb_hsm_reset_t)(void);


/**
 * @brief
 *      OS系统延时
 *
 * @param [ms] 延时时间-单位毫秒
 *
 * @return
 * 	    若成功则返回0，否则返回失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于阻塞时释放CPU资源。
 *
 *
 */
typedef void (*cb_hsm_delay_t)(u32_t ms);




/**
 * @brief
 *      SPI发送
 *
 * @param [tbuff] 发送数据指针
 * @param [length] 发送数据长度
 *
 * @return
 * 	    若成功则返回实际发送长度，其它表示失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于安全芯片通信的SPI数据发送
 *
 *
 */
typedef int (*cb_hsm_spi_send_t)(u8_t *tbuff, u16_t length);


/**
 * @brief
 *      SPI发送
 *
 * @param [rbuff] 接收数据指针
 * @param [length] 接收数据长度
 *
 * @return
 * 	    若成功则返回实际接收长度，其它表示失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于安全芯片通信的SPI数据接收
 *
 *
 */
typedef int (*cb_hsm_spi_recv_t)(u8_t *rbuff, u16_t length);


/**< HSM Control Attributes Type */
typedef struct 
{
	cb_hsm_init_t					init;            		/**< 初始化  */
	cb_hsm_reset_t					reset;					/**< 复位 */
	cb_hsm_delay_t					delay;					/**< 系统延时 */
	cb_hsm_spi_send_t				send;					/**< 发送 */
	cb_hsm_spi_recv_t				recv;					/**< 接收 */
}hsm_ctrl_attr_t;


#endif /* __DL_TYPE_H__ */
