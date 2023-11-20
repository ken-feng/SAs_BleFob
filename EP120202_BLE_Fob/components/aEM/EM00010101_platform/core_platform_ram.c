#include "EM000101.h"
#include "EM000401.h"
#if EM_000101_CONFIG_FEATURE_RAM_MANAGEMENT
u8 ramBuffer[RAM_BUFFER_SIZE];

#define RAM_BUFFER	ramBuffer

#define BUFFER_TAG_VALID	0x01
#define BUFFER_TAG_INVALID	0x00

#define TLV_OFFSET_TAG		0
#define TLV_OFFSET_LENGTH	1
#define TLV_OFFSET_VALUE	3


u8 core_platform_ram_init(void)
{
	core_mm_set(ramBuffer,0x00,sizeof(ramBuffer));
	RAM_BUFFER[0] = 0x00;
	core_dcm_writeBig16(RAM_BUFFER+1,RAM_BUFFER_SIZE-3);
}

u8* core_platform_get_next_tlv(u8* tlv)
{
	u16 length;
	length = core_dcm_readBig16(tlv+TLV_OFFSET_LENGTH);
	return tlv+length+3;
}

void core_platform_ram_reset(void)
{
	u16 length;
	u8* ptr = 0;
	if(RAM_BUFFER[TLV_OFFSET_TAG] != BUFFER_TAG_INVALID)
	{
		return;
	}
	RAM_BUFFER[TLV_OFFSET_TAG] = BUFFER_TAG_INVALID;
	length = core_dcm_readBig16(RAM_BUFFER+TLV_OFFSET_LENGTH);
	ptr = core_platform_get_next_tlv(RAM_BUFFER);
	if((u32)ptr >= (u32)(RAM_BUFFER+RAM_BUFFER_SIZE))
	{
		return;
	}
	while (ptr[TLV_OFFSET_TAG] == BUFFER_TAG_INVALID)
	{
		length += core_dcm_readBig16(ptr+TLV_OFFSET_LENGTH) + 3;
		ptr = core_platform_get_next_tlv(ptr);
		if((u32)ptr >= (u32)(RAM_BUFFER+RAM_BUFFER_SIZE))
		{
			break;
		}
	}
	core_dcm_writeBig16(RAM_BUFFER+TLV_OFFSET_LENGTH,length);
	core_mm_set(RAM_BUFFER+TLV_OFFSET_VALUE,0x00,length);
	return;
}



u8* core_platform_alloc(u16 allocSize)
{
	u8* ptr = 0;
	u32 i = 0;
	u32 leftLength;
	u16 length;
	u16 realSize = allocSize;
	while (i < RAM_BUFFER_SIZE)
	{
		if (RAM_BUFFER[i+TLV_OFFSET_TAG] == BUFFER_TAG_INVALID)
		{
			length = core_dcm_readBig16(RAM_BUFFER+i+TLV_OFFSET_LENGTH);
			ptr = RAM_BUFFER + i + TLV_OFFSET_VALUE;
			leftLength = (u32)ptr%4;
			if (leftLength != 0)
			{
				ptr = ptr + (4 - leftLength);
				realSize += (4 - leftLength);
			}
			if (length < realSize)
			{
				i += 3 + length;
				continue;
			}
			RAM_BUFFER[i+TLV_OFFSET_TAG] = BUFFER_TAG_VALID;
			if (length > realSize + 3)
			{
				core_dcm_writeBig16(RAM_BUFFER+i+TLV_OFFSET_LENGTH,realSize);
				RAM_BUFFER[i+3+realSize+TLV_OFFSET_TAG] = BUFFER_TAG_INVALID;
				core_dcm_writeBig16(RAM_BUFFER+i+3+realSize+TLV_OFFSET_LENGTH,length-realSize-3);
			}
			else
			{
				core_dcm_writeBig16(RAM_BUFFER+i+TLV_OFFSET_LENGTH,length);
			}
			return ptr;
		}
		else
		{
			length = core_dcm_readBig16(RAM_BUFFER+i+TLV_OFFSET_LENGTH);
			i += 3 + length;
		}
	}
	return 0;
}


void core_platform_free(u8* allocBuffer)
{
	u32 i,j;
	u16 length;
	u8* ptr = 0;
	u32 leftLength = 0;
	i = (u32)allocBuffer;
	while(i)
	{
		i--;
		if(((u8*)i)[0] != 0x00)
		{
			break;
		}
		else
		{
			leftLength++;
		}
	}

	i = allocBuffer - RAM_BUFFER - 3 - leftLength;
	RAM_BUFFER[i+TLV_OFFSET_TAG] = BUFFER_TAG_INVALID;
	length = core_dcm_readBig16(RAM_BUFFER+i+TLV_OFFSET_LENGTH);
	ptr = core_platform_get_next_tlv(RAM_BUFFER+i);
	
	while (ptr[TLV_OFFSET_TAG] == BUFFER_TAG_INVALID)
	{
		length += core_dcm_readBig16(ptr+TLV_OFFSET_LENGTH) + 3;
		ptr = core_platform_get_next_tlv(ptr);
		if((u32)ptr >= (u32)(RAM_BUFFER+RAM_BUFFER_SIZE))
		{
			break;
		}
	}
	if((u32)(RAM_BUFFER+i+TLV_OFFSET_LENGTH+length) > (u32)(RAM_BUFFER+RAM_BUFFER_SIZE))
	{
		LOG_L_S(BASE_MD,"[!!!!!!!!******Warning*******!!!!!!!]Free Memory Failed! offset:%x, length:%x \n\r",i,length);
		LOG_L_S(BASE_MD,"[!!!!!!!!******Warning*******!!!!!!!]Please Check Memory Alloc Totol Size!\n\r");
		//core_platform_ram_init();
		return;
	}
	core_dcm_writeBig16(RAM_BUFFER+i+TLV_OFFSET_LENGTH,length);
	core_mm_set(RAM_BUFFER+i+TLV_OFFSET_VALUE,0x00,length);
	return;
}
#endif
