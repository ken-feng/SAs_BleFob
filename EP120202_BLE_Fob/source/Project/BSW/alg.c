//===========================================================================//
//文件说明：算法相关定义
//===========================================================================//
//修订版本：V0.1
//修订人：
//修订时间：2020/12/07
//修订说明：
//1. 创建CRC8_SAE_J1850算法
//2. 创建LIN校验和函数
//===========================================================================//

//文件包含
#include "alg.h"

//数据结构定义

//函数定义和声明

//变量定义和声明


//===========================================================================//
//函数说明：校验算法CRC8_SAE_J1850，多项式X8 + X4 + X3 + X2 + 1
//作者：
//输入：无
//输出：无
//===========================================================================//
unsigned char CRC8_SAE_J1850 (unsigned char *u8_data, unsigned short u8_len)
{
	unsigned char i;
	unsigned char u8_crc8;
	unsigned char u8_poly;


	u8_crc8 = 0xFF;
	u8_poly = 0x1D;


	while(u8_len--)
	{
		u8_crc8 ^= (*u8_data);
		++u8_data;
		for (i = 0; i < 8; i++)
		{
			if (u8_crc8 & 0x80)
			{
				u8_crc8 = (u8_crc8 << 1) ^ u8_poly;
			}
			else
			{
				u8_crc8 <<= 1;
			}
		}
	}

	u8_crc8 ^= (unsigned char)0xFF;
	return u8_crc8;
}

/******************************************************************************
 * Name:    CRC-8/ITU           x8+x2+x+1
 * Poly:    0x07
 * Init:    0x00
 * Refin:   False
 * Refout:  False
 * Xorout:  0x55
 * Alias:   CRC-8/ATM
 *****************************************************************************/
unsigned char crc8_itu(unsigned char *data, unsigned short length)
{
	unsigned char i;
	unsigned char crc = 0;        // Initial value
    while(length--)
    {
        crc ^= *data++;        // crc ^= *data; data++;
        for ( i = 0; i < 8; i++ )
        {
            if ( crc & 0x80 )
            {
            	crc = (crc << 1) ^ 0x07;
            }
            else
            {
            	crc <<= 1;
            }
        }
    }
    return crc ^ 0x55;
}

/******************************************************************************
 * Name:    CRC-8/ROHC          x8+x2+x+1
 * Poly:    0x07
 * Init:    0xFF
 * Refin:   True
 * Refout:  True
 * Xorout:  0x00
 * Note:
 *****************************************************************************/
unsigned char crc8_rohc(unsigned char *data, unsigned short length)
{
	unsigned char i;
	unsigned char crc = 0xFF;         // Initial value
    while(length--)
    {
        crc ^= *data++;            // crc ^= *data; data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
            {
            	crc = (crc >> 1) ^ 0xE0;        // 0xE0 = reverse 0x07
            }
            else
            {
            	crc = (crc >> 1);
            }
        }
    }
    return crc;
}


//===========================================================================//
//函数说明：计算LIN的校验和
//标准校验和校验对象：数据段各字节
//增强型校验和校验对象：数据段各字节和PID
//诊断数据帧都是采用标准校验和
//睡眠帧的校验和为0
//输入: p_data，校验和对象起始位置（如果是增强型校验就要从PID开始，普通就是从response开始）
//输出：校验和
//===========================================================================//
unsigned char LIN_frame_calc_checksum(unsigned char *p_data, unsigned char len)
{
    unsigned char sum = 0, i = 0;
    for (i = 0; i < len; i++)
    {
        sum += p_data[i];
        if (sum < p_data[i])
        {//带进位+1
            sum += 1;
        }
    }

    sum = ~sum;
    return sum;
}


//===========================================================================//
//函数说明：检查数组里的数据是否是全0
//输入: p_data，待校验数据
//	  len, 数据长度
//输出：校验和
//===========================================================================//
unsigned char Check_Buff_All_0x00 (unsigned char *p_data, unsigned char len)
{
	unsigned char Result = 1;
	while (len--)
	{
		if ((*p_data) > 0)
		{
			Result = 0;
			break;
		}
		else
		{

		}
		++p_data;
	}
	return Result;
}


