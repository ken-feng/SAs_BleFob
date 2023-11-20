#include "EM000201.h"
#if EM_000201_CONFIG_FEATURE_CRYPTO_OTHER

u8 core_algo_luhn(u8 *indata, u16 inLength)
{
	u8 i;
	u16 sum = 0;
	u16 tmp;
	for(i = inLength - 1; i > 0; i--)
	{
		sum += indata[i]&0x0F;//奇数位
		tmp = ((indata[i]>>4)&0x0F)*2;//偶数位
		sum += tmp >= 10 ? tmp-9:tmp;
	}
	sum += indata[0]&0x0F;//奇数位
	tmp = ((indata[0]>>4)&0x0F)*2;//偶数位
	sum += tmp >= 10 ? tmp-9:tmp;

	if (sum%10 == 0)
	{
		return 0;
	}
	return 1;
}

/**
 * Methods implementation
 */
void core_algo_xor(const u8 *buf1, const u8 *buf2, u8 *outBuf, u16 length)
{
	while (length--)
	{
		*outBuf = (*buf1) ^ (*buf2);
		buf1++;
		buf2++;
		outBuf++;
	}
}

#endif
