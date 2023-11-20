#include"EM000101.h"

u8 core_algo_reflect_u8(u8 thedata)
{
	thedata = ((thedata & 0x55)<<1) | ((thedata>>1) & 0x55);
	thedata = ((thedata & 0x33)<<2) | ((thedata>>2) & 0x33);
	thedata = ((thedata)<<4) | ((thedata>>4));
	return thedata;
}

u16 core_algo_reflect_u16(u16 thedata)
{
	u16 out = core_algo_reflect_u8((u8)(thedata>>8)) | ((u16)core_algo_reflect_u8((u8)thedata)<<8);
	return out;
}

u32 core_algo_reflect_u32(u32 thedata)
{
	thedata = ((thedata & 0x55555555)<<1) | ((thedata>>1) & 0x55555555);
	thedata = ((thedata & 0x33333333)<<2) | ((thedata>>2) & 0x33333333);
	thedata = ((thedata & 0x0F0F0F0F)<<4) | ((thedata>>4) & 0x0F0F0F0F);
	thedata = ((thedata & 0x00FF00FF)<<8) | ((thedata>>8) & 0x00FF00FF);
	thedata = (thedata<<16) | (thedata>>16);
	return thedata;
}
u16 core_algo_convert_u8tou32(u8 *inBuf, u16 inLength, u32 *outBuf)
{
	u16 i;
	u8 j;
	for (i=0, j=24; i<inLength; i++, j-=8)
	{
		if (j == 24)
		{
			outBuf[i/4] = 0;
		}
		outBuf[i/4] |= ((u32)inBuf[i]) << j;
		if (j == 0)
		{
			j = 32;
		}
	}
	return (u16)((inLength+3)/4);
}

u16 core_algo_convert_u32tou8(u32 *inBuf, u16 inLength, u8 *outBuf)
{
	u16 i;

	for (i=0; i<inLength; i++)
	{
		outBuf[i*4] = (u8)(inBuf[i] >> 24);
		outBuf[i*4+1] = (u8)(inBuf[i] >> 16);
		outBuf[i*4+2] = (u8)(inBuf[i] >> 8);
		outBuf[i*4+3] = (u8)(inBuf[i]);
	}

	return (u16)(inLength*4);
}


// do byte array swap, from Big endian(src) to little endian(dest).
void core_algo_swap_u8(u8 *dest, const u8 *src, u16 length)
{
    u16 i;

    for (i=0; i<length; i++)
    {
        dest[length-i-1] = src[i];
    }
}

/***
*Big number sub
*p1 is the sub1;len1 is length of the sub1
*p2 is the sub2;len2 is length of the sub2
*p3 is the destBuf to save the result;len3 is len of the result
*the return boolean value:
*TRUE: the result is greater than zero or equal to zero
*FALSE: the result is less than zero
***/
boolean core_algo_bigNumsub(u8* p1, u16 len1, u8* p2, u16 len2, u8* p3, u16* len3)
{
	u16 i,j;
	u16 k = (len1 >= len2 ? len1 : len2);
	u8 bit=0; 

	*len3 = k;

    for(i=len1,j=len2;i>0 && j>0;--i,--j) 
    {   
        if(p1[i-1] < (p2[j-1] + bit))
        {  
            p3[--k]=256+p1[i-1]-p2[j-1]-bit;  
            bit=1;    
        }
        else
        {
			p3[--k]=p1[i-1]-p2[j-1]-bit;
            bit=0;
        }
    }
    while(i>0)
    {
        if(p1[i-1]<bit)
        {
			p3[--k]=p1[i-1]-bit+256;
            bit=1;
        }
        else
        {
			p3[--k]=p1[i-1]-bit;
            bit=0;
        }
        i--;
    }
    while(j>0)
    {        
        p3[--k]=256-bit-p2[j-1];
		bit = 1;
        j--;
    }
    if(bit==1)
    {
        p3[(*len3)-1]=256 - p3[(*len3)-1];
        for(i=((*len3)-1);i>0;--i)
        {
            p3[i-1]=256-p3[i-1]-bit;
        }        
    }

	if(bit == 1)
	{
		//the result is less than zero
		return FALSE;
	}
	else
	{
		//the result is greater than or equal to zero
		return TRUE;
	}
}

//compare two big num
//when len1 greater than len2, then p1 greater than p2
//return: TRUE indicate p1 is greater or equal to p2
//return: FALSE indicate p1 is less than p2
boolean core_algo_BigNumCompare(u8* p1, u16 len1, u8* p2, u16 len2)
{
	u16 i;

	if(len1 > len2)
	{
		return TRUE;
	}
	else if(len1 < len2)
	{
		return FALSE;
	}
	else//(len1 == len2)
	{
		for(i=0;i<len1;i++)
		{
			if(p1[i] > p2[i])
			{
				return TRUE;
			}
			else if(p1[i] < p2[i])
			{
				return FALSE;
			}
		}
		//indicate p1 is equal to p2
		return TRUE;
	}
}
void core_array_reversed_u8(u8* src, u16 length)
{
    u8 temp = 0;
    u8* srcend = src + length - 1;
    
    if (NULL == src)
        return;
    if (0 == length)
        return;

    while (src < srcend)
    {
        temp = *src;
        *src = *srcend;
        src += 1L;
        *srcend = temp;
        srcend -= 1L;
    }
}


void core_mm_copy(u8 * dest, u8 * src, u16 length)
{
    if (dest < src)
	{
        while (length--)
		{
            *dest = *src;
			dest += 1L;
			src += 1L;
		}
	}
    else if (dest > src)
	{
        while (length--)
		{
            dest[length] = src[length];
		}
	}
}

void core_mm_copy_u32(u32 * dest, u32 * src, u16 length)
{
    if (dest < src)
	{
        while (length--)
		{
            *dest = *src;
			dest += 1L;
			src += 1L;
		}
	}
    else if (dest > src)
	{
        while (length--)
		{
            dest[length] = src[length];
		}
	}
}

void core_mm_set(u8 * dest, u8 val, u16 length)
{
    while (length--)
        dest[length] = val;
}

void core_mm_set_u32(u32 *dest, u32 val, u16 length)
{
	while (length--)
        dest[length] = val;
}


s8 core_mm_compare_unsigned(u8 * buf1, u8 * buf2, u16 length)
{
	u16 i;

	if (buf1 == buf2)
	{
		return 0;
	}

    for (i=0; i<length; i++)
	{
        if (buf1[i] > buf2[i])
		{
            return 1;
		}
		else if (buf1[i] < buf2[i])
		{
			return -1;
		}
	}

    return 0;
}


s8 core_mm_compare(s8 * buf1, s8 * buf2, u16 length)
{
	u16 i;

	if (buf1 == buf2)
	{
		return 0;
	}

    for (i=0; i<length; i++)
	{
        if (buf1[i] > buf2[i])
		{
            return 1;
		}
		else if (buf1[i] < buf2[i])
		{
			return -1;
		}
	}

    return 0;
}

u8 core_mm_compare_with_byte(u8 *src, u8 val, u16 length)
{
	while (length--)
		if (src[length] != val)
			return 1;

	return 0;
}

u8 tlv_resolve(u8* src,u8 len,u16 dest_tag,u8* dest_buf,u16* olen,u8* off)
{
    u16 idx = 0x00;
    u16 tag = 0x00;
    u16 tlv_len= 0x00;
    u16 value_len = 0x00;
    u8 isComplex=0x00;
    u8 isfinish = 0x00;

	if((src == NULL) || (len==0 )|| (dest_tag ==0x0000) || (dest_buf == NULL)||(olen==NULL))
    {
        return 0x00;//RV_PARAM_ERR;
    }

    while(idx<len)
    {
        tag = src[idx];//TLV Tag Len Value:First byte of Tag
		if((tag & 0x20) == 0x20)
        {
            isComplex = 0x01;
        }
        else
        {
            isComplex = 0x00;
        }
        if((tag & 0x1f) == 0x1f)
        {
            tag = (src[idx]<<8)|(src[idx+1]);
            idx++;
        }

        idx++;//TLV Tag Len Value:First byte of Len

        tlv_len = src[idx];
        if((tlv_len >0) && (tlv_len <=0x7f))
        {
            value_len = tlv_len;
        }
        else
        {
            if((tlv_len & 0x80) ==0x80)
            {
                if(tlv_len == 0x81)
                {
                    value_len = src[idx+1];
                    idx++;//TLV Tag Len Value:Last byte of Len
                }
                else if(tlv_len == 0x82)
                {
                    value_len = (src[idx+1]<<8)|(src[idx+2]);
                    idx+=2;//TLV Tag Len Value:Last byte of Len
                }else
                {
                    isfinish = 0x00;
                    break;
                }

            }
            else
            {
                isfinish = 0x00;
                break;
            }
        }
        idx++;//TLV Tag Len Value:First byte of Value OR First byte of TLV when isComplex=1

        if(!isComplex)
        {
            if(tag==dest_tag)
            {
                core_mm_copy(dest_buf,&src[idx],value_len);
                *olen = value_len;
				*off = idx+value_len;
                isfinish = 0x01;
                break;
            }
            idx+=value_len;//First byte of NEXT TLV
        }
        else
        {
            if(tag==dest_tag)
            {
                core_mm_copy(dest_buf,&src[idx],value_len);
                *olen = value_len;
				*off = idx+value_len;
                isfinish = 0x01;
                break;
            }
        }
    }

    return isfinish;
}
