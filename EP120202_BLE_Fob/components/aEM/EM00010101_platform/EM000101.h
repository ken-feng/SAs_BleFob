#ifndef _HEADER_EM000101_H
#define _HEADER_EM000101_H


#include "EM000101_config.h"
typedef unsigned char						u8;
typedef signed char							s8;
typedef unsigned short						u16;
typedef signed short						s16;
typedef unsigned int						u32;
typedef signed int							s32;
typedef float                               f32;
typedef unsigned char						boolean;
typedef unsigned int						memref;

#ifndef TRUE
#define TRUE								1
#endif

#ifndef FALSE
#define FALSE								0
#endif

#ifndef NULL
#define NULL								(0)
#endif

#ifndef NULL_PTR
#define NULL_PTR							(0)
#endif

#if EM_000101_CONFIG_FEATURE_RAM_MANAGEMENT

#define RAM_BUFFER_SIZE		3*1024//12*1024

u8 core_platform_ram_init(void);
void core_platform_ram_reset(void);
u8* core_platform_alloc(u16 allocSize);
void core_platform_free(u8* allocBuffer);

#endif


#define FLASH_PAGE_SIZE		0x0800
#define CORE_FS_START  	    0x0007A000
#define CORE_FS_PAGES   	8
u8 core_platform_flash_erase_page(u8* flashAddr);
u8 core_platform_flash_write_page(u8* flashAddr, u8* ramAddr);
u8 core_platform_flash_write_byte(u8* flashAddr, u8* ramAddr, u16 length);



#define core_dcm_u8_lo(b)                   ((u8)((u8)(b)&0x0f))
#define core_dcm_u8_hi(b)                   ((u8)((u8)(b)>>4))

#define core_dcm_u16_lo(w)                   ((u8)((u16)(w)&0x00ff))
#define core_dcm_u16_hi(w)                   ((u8)((u16)(w)>>8))

#define core_dcm_u32_lo(dw)                  ((u16)((u32)(dw)&0xffff))
#define core_dcm_u32_hi(dw)                  ((u16)((u32)(dw)>>16))

#define core_dcm_mku16(h, l)                ((u16)(((u16)(h)<<8) | (l)))
#define core_dcm_mku32(hh, hl, lh, ll)      ((u32)((((u32)hh)<<24) | ((u32)(hl)<<16) | ((u16)(lh)<<8) | (ll)))
#define core_dcm_readU8(src)                ((u8)(*((u8 *)(src))))
#define core_dcm_readBig16(src)             core_dcm_mku16((((u8 *)(src))[0]), (((u8 *)(src))[1]))
#define core_dcm_readBig32(src)             core_dcm_mku32((((u8 *)(src))[0]), (((u8 *)(src))[1]), (((u8 *)(src))[2]), (((u8 *)(src))[3]))
#define core_dcm_readBig24(src)             core_dcm_mku32(                 0, (((u8 *)(src))[0]), (((u8 *)(src))[1]), (((u8 *)(src))[2]))

#define core_dcm_writeU8(dest, value)       ((((u8 *)(dest))[0]) = (value))

#define core_dcm_writeBig16(dest, value)    do {(((u8 *)(dest))[0]) = (u8)((value)>>8); (((u8 *)(dest))[1]) = (u8)(value);} while(0);

#define core_dcm_writeBig32(dest, value)    do { \
                                                (((u8 *)(dest))[0]) = (u8)((value)>>24); \
                                                (((u8 *)(dest))[1]) = (u8)((value)>>16); \
                                                (((u8 *)(dest))[2]) = (u8)((value)>>8);  \
                                                (((u8 *)(dest))[3]) = (u8)(value);       \
                                            } while(0);

#define core_dcm_writeBig24(dest, /* u32 */value)   do { \
                                                (((u8 *)(dest))[0]) = (u8)((value)>>16); \
                                                (((u8 *)(dest))[1]) = (u8)((value)>>8);  \
                                                (((u8 *)(dest))[2]) = (u8)(value);       \
                                            } while(0);


void core_array_reversed_u8(u8 * src, u16 length);
void core_mm_copy(u8 * dest, u8 * src, u16 length);
void core_mm_copy_u32(u32 * dest, u32 * src, u16 length);
void core_mm_set(u8 * dest, u8 val, u16 length);
void core_mm_set_u32(u32 *dest, u32 val, u16 length);
void core_mm_set_mass(u8 * dest, u8 val, u32 length);
s8 core_mm_compare(s8 * buf1, s8 * buf2, u16 length);
u8 core_mm_compare_with_byte(u8 *src, u8 val, u16 length);
s8 core_mm_compare_unsigned(u8 * buf1, u8 * buf2, u16 length);

u8 core_algo_reflect_u8(u8 thedata);
u16 core_algo_reflect_u16(u16 thedata);
u32 core_algo_reflect_u32(u32 thedata);
u16 core_algo_convert_u8tou32(u8 *inBuf, u16 inLength, u32 *outBuf);
u16 core_algo_convert_u32tou8(u32 *inBuf, u16 inLength, u8 *outBuf);

void core_algo_swap_u8(u8 *dest, const u8 *src, u16 length);

//big number sub
boolean core_algo_bigNumsub(u8* p1, u16 len1, u8* p2, u16 len2, u8* p3, u16* len3);
boolean core_algo_BigNumCompare(u8* p1, u16 len1, u8* p2, u16 len2);

u8 tlv_resolve(u8* src,u8 len,u16 dest_tag,u8* dest_buf,u16* olen,u8* off);

#endif//_HEADER_RGP_PF_H
