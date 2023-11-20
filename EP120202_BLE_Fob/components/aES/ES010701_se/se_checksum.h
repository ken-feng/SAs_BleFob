#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__
#include "se_common_type.h" 
 
u8_t lrc8(const void *s, u32_t n);

u16_t crc_ccitt2(u16_t crc, const u8_t *s, u32_t n);

u16_t crc_ccitt1(u16_t crc, const u8_t *s, u32_t n);
 
#endif

 /******************* (C) *****END OF FILE****/
