


#ifndef _ALG_H
#define _ALG_H


//CRC8_SAE_J1850
extern unsigned char CRC8_SAE_J1850 (unsigned char *u8_data, unsigned short u8_len);

//crc8_itu
extern unsigned char crc8_itu(unsigned char *data, unsigned short length);

//crc8_rohc
extern unsigned char crc8_rohc(unsigned char *data, unsigned short length);

//LIN校验和
extern unsigned char LIN_frame_calc_checksum (unsigned char *p_data, unsigned char len);

//
extern unsigned char Check_Buff_All_0x00 (unsigned char *p_data, unsigned char len);

#endif


