
#include"EM000101.h"
#if EM_000101_CONFIG_FEATURE_OPR_FLASH
#include "Flash_Adapter.h"

u8 core_platform_flash_erase_page(u8* flashAddr)
{

	//KW38_Write_eeprom(EEPROM_ADDR_OFFSET_UQBLEKEY+offset,pdata,length);
	// uint32_t status;
	// status = NV_FlashEraseSector((u32)flashAddr,FLASH_PAGE_SIZE);
	// if(status == kStatus_FLASH_Success)
	// {
	// 	return 0;
	// }

	return 1;
}

 u8 core_platform_flash_write_page(u8* flashAddr, u8* ramAddr)
{

	// uint32_t status;
	// if(core_platform_flash_erase_page((u32)flashAddr))
	// {
	// 	return 1;
	// }
	// status = NV_FlashProgram((u32)flashAddr,FLASH_PAGE_SIZE,ramAddr);
	// if(status == kStatus_FLASH_Success)
	// 	return 0;

	return 1;
}
//u8 core_pagebuff[FLASH_PAGE_SIZE];
u8 core_platform_flash_write_byte(u8* flashAddr, u8* ramAddr, u16 length)
{


	// u32 pageStart, pageEnd, start;
	// u16 validLen;
	// if(length == 0)
	// {
	// 	return 0;
	// }

	// pageStart = (((u32)flashAddr % FLASH_PAGE_SIZE) == 0) ?  (u32)flashAddr : (((u32)flashAddr/FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE);
	// pageEnd = ((((u32)flashAddr + length) % FLASH_PAGE_SIZE) == 0) ? \
	// 	((u32)flashAddr + length) : (((((u32)flashAddr + length) /FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE) + FLASH_PAGE_SIZE);

	// start = pageStart;

	// do
	// {
	// 	core_mm_copy(core_pagebuff, (u8*)start, FLASH_PAGE_SIZE);
	// 	if(start == pageStart)  
	// 	{
	// 			validLen = (length + (((u32)flashAddr - pageStart)) >= (pageStart + FLASH_PAGE_SIZE)) ? (FLASH_PAGE_SIZE - ((u32)flashAddr - pageStart)) : length;
	// 			core_mm_copy(core_pagebuff + ((u32)flashAddr - pageStart), ramAddr, validLen);
	// 	}
	// 	else if(start == (pageEnd - FLASH_PAGE_SIZE))  
	// 	{
	// 			core_mm_copy(core_pagebuff, ramAddr + (start - pageStart) - ((u32)flashAddr - pageStart), length +  ((u32)flashAddr - pageStart) - (start - pageStart));
	// 	}
	// 	else  
	// 	{
	// 			core_mm_copy(core_pagebuff, ramAddr + (start - pageStart) - ((u32)flashAddr - pageStart), FLASH_PAGE_SIZE);
	// 	}

	// 	if(core_platform_flash_write_page((u8*)start, core_pagebuff) != 0)
	// 	{
	// 			return 1;
	// 	}

	// 	start += FLASH_PAGE_SIZE;
	// }while(start < pageEnd);

	return 0x00;
}
#endif
