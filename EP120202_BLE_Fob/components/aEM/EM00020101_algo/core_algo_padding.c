#include "EM000201.h"

#if EM_000201_CONFIG_FEATURE_CRYPTO_PADDING
u16 core_algo_padding_ISO9797_M1(u8 *srcBuf, u16 srcLength, u8 blockLength)
{
	u8 left = blockLength - (u8)(srcLength % blockLength);
	
	if (left == blockLength)
	{
		left = 0;
	}

	core_mm_set(srcBuf+srcLength, 0, left);

	return (u16)(srcLength+left);
}

u16 core_algo_unpadding_ISO9797_M1(u8 *srcBuf, u16 srcLength, u8 blockLength)
{
	if ((srcLength % blockLength) != 0)
	{
		return 0;
	}

	return srcLength;
}

u16 core_algo_padding_ISO9797_M2(u8 *srcBuf, u16 srcLength, u8 blockLength)
{
	u8 left;
	
	srcBuf[srcLength] = 0x80;
	srcLength++;

	left = blockLength - (u8)(srcLength % blockLength);
	
	if (left == blockLength)
	{
		left = 0;
	}

	core_mm_set(srcBuf+srcLength, 0, left);

	return (u16)(srcLength+left);
}

/**
 * remove the last padding byte from the last '80'
 */
u16 core_algo_unpadding_ISO9797_M2(u8 *srcBuf, u16 srcLength, u8 blockLength)
{
	u16 i;

	if (srcLength == 0)
	{
		return 0;
	}

	if ((srcLength % blockLength) != 0)
	{
		return 0;
	}

	for (i=srcLength; i>srcLength-blockLength; i--)
	{
		if (srcBuf[i-1] == 0x80)
		{
			break;
		}
        if (srcBuf[i-1] != 0x00)
        {
            srcBuf[0] = 0x00;
            return 0;
        }
	}

	if (i == (u16)(srcLength-blockLength))
	{
        srcBuf[0] = 0x00;
		return 0;
	}

	return (u16)(i - 1);
}


/**
 * Concatenate M and a padding string PS to form an encoded message EM:
 * EM = M || PS ,
 * where the padding string PS consists of 8-(||M|| mod 8) octets
 * each with value 8-(||M|| mod 8)
 */
u16 core_algo_padding_PKCS5(u8 *srcBuf, u16 srcLength, u8 blockLength)
{
	u8 left;

	left = blockLength - (u8)(srcLength % blockLength);

	core_mm_set(srcBuf+srcLength, left, left);

	return (u16)(srcLength+left);
}

/**
 * remove the last padding byte 
 */
u16 core_algo_unpadding_PKCS5(u8 *srcBuf, u16 srcLength, u8 blockLength)
{
	if (srcLength == 0)
	{
		return 0;
	}

	if ((srcLength % blockLength) != 0)
	{
		return 0;
	}
	
	if (srcBuf[srcLength-1] > blockLength)
	{
		return 0;
	}
	else
	{
		if (core_mm_compare_with_byte(srcBuf+srcLength-srcBuf[srcLength-1], srcBuf[srcLength-1], srcBuf[srcLength-1]))
		{
			return 0;
		}
	}

	return (u16)(srcLength-srcBuf[srcLength-1]);
}

/**
 * X := (B || MSG1 || H || E).
 * H := Hash[MSG] of the message M
 * Split MSG into two parts MSG =(MSG1 || MSG2), where MSG1 consists of the N - 22 
 * leftmost (most significant bytes) of MSG and MSG2 of the remaining (least significant)
 * L - N + 22 bytes of MSG.
 * B = '6A', E = 'BC'
 * 
 * return the offset of HASH massage
 */
void core_algo_padding_ISO9796(u8 *srcBuf, u16 blockLength, u8 *msg1, u16 msgLength, u8 *hash)
{
	srcBuf[0] = ((msgLength + 22 >= blockLength) ? (msgLength + 22 == blockLength ? 0x4A : 0x6A) : 0x4B);
	if (srcBuf[0] == 0x4B)
	{
		core_mm_copy(srcBuf+blockLength-21-msgLength, msg1, msgLength);
		core_mm_set(srcBuf+1, 0xBB, blockLength-23-msgLength);
		srcBuf[blockLength-22-msgLength] = 0xBA;
	}
	core_mm_copy(srcBuf+blockLength-21, hash, 20);
	srcBuf[blockLength-1] = 0xBC;
}

/**
 * X := (B || MSG1 || H || E).
 * H := Hash[MSG] of the message M
 * Split MSG into two parts MSG =(MSG1 || MSG2), where MSG1 consists of the N - 22 
 * leftmost (most significant bytes) of MSG and MSG2 of the remaining (least significant)
 * L - N + 22 bytes of MSG.
 * B = '6A', E = 'BC'
 * 
 */
boolean core_algo_unpadding_ISO9796(u8 *srcBuf, u16 blockLength, u16 msgLength, u8 *hash)
{
	u8 fb = ((msgLength + 22 >= blockLength) ? (msgLength + 22 == blockLength ? 0x4A : 0x6A) : 0x4B);
	if (srcBuf[0] != fb || srcBuf[blockLength-1] != 0xBC ||
			core_mm_compare(hash, srcBuf+blockLength-21, 20))
	{
		return FALSE;
	}

	if (fb == 0x4B && (srcBuf[blockLength-22-msgLength] != 0xBA || 
			core_mm_compare_with_byte(srcBuf+1, 0xBB, blockLength-23-msgLength)))
	{
		return FALSE;
	}

	return TRUE;
}

u16 core_algo_unpadding_ISO9796_recoveryMessage(u8 *srcBuf, u16 blockLength)
{
	u8 i;

	if ((srcBuf[0] != 0x4A && srcBuf[0] != 0x4B && srcBuf[0] != 0x6A) || srcBuf[blockLength-1] != 0xBC)
	{
		return 0;
	}

	if (srcBuf[0]  == 0x4B)
	{
		for (i=1; i<blockLength-21; i++)
		{
			if (srcBuf[i] != 0xBB)
			{
				break;
			}
		}
		if (i == blockLength-21 || srcBuf[i] != 0xBA)
		{
			return 0;
		}
		return i+1;
	}

	return 1;
}

#endif