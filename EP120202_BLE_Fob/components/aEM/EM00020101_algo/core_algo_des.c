#include "EM000201.h"

#if EM_000201_CONFIG_FEATURE_CRYPTO_DES
/**
 * Constant Variables definition
 */
static const u8 mask_byte[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

static const u8 key_table[16][48]={
    {10,51,34,60,49,17,33,57, 2, 9,19,42, 3,35,26,25,44,58,59, 1,36,27,18,41,22,28,39,54,37, 4,47,30, 5,53,23,29,61,21,38,63,15,20,45,14,13,62,55,31},
    { 2,43,26,52,41, 9,25,49,59, 1,11,34,60,27,18,17,36,50,51,58,57,19,10,33,14,20,31,46,29,63,39,22,28,45,15,21,53,13,30,55, 7,12,37, 6, 5,54,47,23},
    {51,27,10,36,25,58, 9,33,43,50,60,18,44,11, 2, 1,49,34,35,42,41, 3,59,17,61, 4,15,30,13,47,23, 6,12,29,62, 5,37,28,14,39,54,63,21,53,20,38,31, 7},
    {35,11,59,49, 9,42,58,17,27,34,44, 2,57,60,51,50,33,18,19,26,25,52,43, 1,45,55,62,14,28,31, 7,53,63,13,46,20,21,12,61,23,38,47, 5,37, 4,22,15,54},
    {19,60,43,33,58,26,42, 1,11,18,57,51,41,44,35,34,17, 2, 3,10, 9,36,27,50,29,39,46,61,12,15,54,37,47,28,30, 4, 5,63,45, 7,22,31,20,21,55, 6,62,38},
    { 3,44,27,17,42,10,26,50,60, 2,41,35,25,57,19,18, 1,51,52,59,58,49,11,34,13,23,30,45,63,62,38,21,31,12,14,55,20,47,29,54, 6,15, 4, 5,39,53,46,22},
    {52,57,11, 1,26,59,10,34,44,51,25,19, 9,41, 3, 2,50,35,36,43,42,33,60,18,28, 7,14,29,47,46,22, 5,15,63,61,39, 4,31,13,38,53,62,55,20,23,37,30, 6},
    {36,41,60,50,10,43,59,18,57,35, 9, 3,58,25,52,51,34,19,49,27,26,17,44, 2,12,54,61,13,31,30, 6,20,62,47,45,23,55,15,28,22,37,46,39, 4, 7,21,14,53},
    {57,33,52,42, 2,35,51,10,49,27, 1,60,50,17,44,43,26,11,41,19,18, 9,36,59, 4,46,53, 5,23,22,61,12,54,39,37,15,47, 7,20,14,29,38,31,63,62,13, 6,45},
    {41,17,36,26,51,19,35,59,33,11,50,44,34, 1,57,27,10,60,25, 3, 2,58,49,43,55,30,37,20, 7, 6,45,63,38,23,21,62,31,54, 4,61,13,22,15,47,46,28,53,29},
    {25, 1,49,10,35, 3,19,43,17,60,34,57,18,50,41,11,59,44, 9,52,51,42,33,27,39,14,21, 4,54,53,29,47,22, 7, 5,46,15,38,55,45,28, 6,62,31,30,12,37,13},
    { 9,50,33,59,19,52, 3,27, 1,44,18,41, 2,34,25,60,43,57,58,36,35,26,17,11,23,61, 5,55,38,37,13,31, 6,54,20,30,62,22,39,29,12,53,46,15,14,63,21,28},
    {58,34,17,43, 3,36,52,11,50,57, 2,25,51,18, 9,44,27,41,42,49,19,10, 1,60, 7,45,20,39,22,21,28,15,53,38, 4,14,46, 6,23,13,63,37,30,62,61,47, 5,12},
    {42,18, 1,27,52,49,36,60,34,41,51, 9,35, 2,58,57,11,25,26,33, 3,59,50,44,54,29, 4,23, 6, 5,12,62,37,22,55,61,30,53, 7,28,47,21,14,46,45,31,20,63},
    {26, 2,50,11,36,33,49,44,18,25,35,58,19,51,42,41,60, 9,10,17,52,43,34,57,38,13,55, 7,53,20,63,46,21, 6,39,45,14,37,54,12,31, 5,61,30,29,15, 4,47},
    {18,59,42, 3,57,25,41,36,10,17,27,50,11,43,34,33,52, 1, 2, 9,44,35,26,49,30, 5,47,62,45,12,55,38,13,61,31,37, 6,29,46, 4,23,28,53,22,21, 7,63,39}
};

static const u8 sp[8][64]={
    {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13},
    {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9},
    {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12},
    { 7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14},
    {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3},
    {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13},
    {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12},
    {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
};
static const u8 extend_table[48]={32,1,2,3,4,5,4,5,6,7,8,9, 8,9,10,11,12,13,12,13,14,15,16,17,16,17,18,19,20,21,20,21,22,23,24,25,24,25,26,27,28,29,28,29,30,31,32,1};

static const u8 p_table[32]={16, 7,20,21,29,12,28,17, 1,15,23,26, 5,18,31,10, 2, 8,24,14,32,27, 3, 9,19,13,30, 6,22,11, 4,25};


/**
 * Static Methods implementation
 */
static void des_block( u8 *message,  u8 *key, u8 *unmessage, u8 mode)
{
    u8 i,j,k,l,m,n;
    u8 x[8],roundkey[8],extend[8];

    //  Init permutation
    for(i=0; i<8;i++)
    {
        for (j=0;j<8;j++)
        {
            if(j<4)
                k=(u8)(2*j+1);
            else
                k=(u8)(2*(j-4));
            x[j]<<=1;
            x[j]|=(message[7-i]&mask_byte[k])?1:0;
        }
    }

    // 16 round loop,
    for(i=0;i<16;i++)
    {
        // Produce the encrypt or decrypt key
        l=(u8)(mode != MODE_ENCRYPT?(15-i):i);
        for(j=0;j<8;j++)
        {
            for(k=0;k<6;k++)
            {
                n=(u8)(key_table[l][6*j+k]-1);
                m=(u8)(n>>3);
                roundkey[j]<<=1;
                roundkey[j]|=(key[m]&mask_byte[n&0x07])?1:0;
            }
        }

        // Extend 32 bits to 48 bits
        l=(u8)((i&0x01)?0:4); // choose extend the right part or left part
        for(j=0;j<8;j++)
        {
            for(k=0;k<6;k++)
            {
                n=(u8)(extend_table[6*j+k]-1);
                m=(u8)(n>>3);
                extend[j]<<=1;
                extend[j]|=(x[m+l]&mask_byte[n&0x07])?1:0;
            }
        }

        // Xor between the extended result and the roundkey
        for(j=0;j<8;j++)
            extend[j]^=roundkey[j];

        // Filter the Xor result by s box
        for(j=0;j<8;j++)
        {
            k=(u8)((extend[j]&0x20)>>4 | (extend[j]&0x01));
            m=(u8)((extend[j]&0x1e)>>1);
            extend[j]=(u8)(sp[j][k*16+m]);
        }

        //Compress the 8 half bytes into 4 bytes
        for(j=0;j<4;j++)
            extend[j]=(u8)((extend[2*j]<<4)|(extend[2*j+1]));

        // Execute round permutation to the compressing result
        for(j=0;j<4;j++)
            for(k=0;k<8;k++)
            {
                n=(u8)(p_table[8*j+k]-1);
                m=(u8)(n>>3);
                extend[j+4]<<=1;
                extend[j+4]|=(extend[m]&mask_byte[n&0x07])?1:0;
            }

        // Xor between the permuted result and another not extended part
        for(j=0;j<4;j++)
            x[4-l+j]^=extend[j+4];

    }

    // Final permutation
    for(i=0; i<8;i++)
    {
        k=(u8)(i>>1);
        if(i&0x01) k=(u8)(k+4);
        for (j=0;j<8;j++)
        {
            extend[j]<<=1;
            extend[j]|=(x[k]&mask_byte[7-j])?1:0;
        }
    }

    //Copy the result to the output parameter
    for(i=0;i<8;i++)
        unmessage[i]=extend[i];

}

static void tdes2k_block( u8 *message,  u8 *key, u8 *unmessage, u8 mode)
{
    des_block(message, key, unmessage, mode);
    des_block(unmessage, key+8, unmessage, (u8)!mode);
    des_block(unmessage, key, unmessage, mode);
}

static void tdes3k_block( u8 *message,  u8 *key, u8 *unmessage, u8 mode)
{
    des_block(message, key+(1-mode)*16, unmessage, mode);
    des_block(unmessage, key+8, unmessage, (u8)!mode);
    des_block(unmessage, key+mode*16, unmessage, mode);
}

/**
 * Methods implementation
 */
void core_algo_des(u8 *key, u8 keyLen,  u8 *src, u8 *out,u8 mode)
{

	if (keyLen == 8)
	{
		des_block(src, key, out, mode);
	}
	else if (keyLen == 16)
	{
		tdes2k_block(src, key, out, mode);
	}
	else if (keyLen == 24)
	{
		tdes3k_block(src, key, out, mode);
	}
	else
	{
		
	}
}
u16 core_algo_des_ecb_cipher(u8 mode,  u8 *key, u8 key_len,  u8 *src_buf, u8 *dest_buf, u16 src_len)
{
	u16 i;

	for (i=0; i<src_len; i+=8)
	{
		core_algo_des(key, key_len, src_buf+i, dest_buf+i, mode);
	}

	return src_len;
}

u16 core_algo_des_cbc_cipher(u8 mode,  u8 *key, u8 key_len, u8 *iv, u8 *src_buf, u8 *dest_buf, u16 src_len)
{
	u16 i;
	u8 iv_bak[8];

	if (src_len == 0)
	{
		return 0;
	}

	if (mode == MODE_DECRYPT)
	{
		src_buf += src_len - 8;
		dest_buf += src_len - 8;
		core_mm_copy(iv_bak, src_buf, 8);
	}

	for (i=0; i<src_len; i+=8)
	{
		if (mode == MODE_ENCRYPT)
		{
			core_algo_xor(iv, src_buf, iv, 8);
			core_algo_des(key, key_len, iv, dest_buf, mode);
			core_mm_copy(iv, dest_buf, 8);

			src_buf += 8;
			dest_buf += 8;
		}
		else 
		{
			core_algo_des(key, key_len, src_buf, dest_buf, mode);
			if (i != src_len - 8)
			{
				core_algo_xor(dest_buf, src_buf-8, dest_buf, 8);
			}
			else
			{
				core_algo_xor(dest_buf, iv, dest_buf, 8);
				core_mm_copy(iv, iv_bak, 8);
			}

			src_buf -= 8;
			dest_buf -= 8;
		}
	}

	return src_len;
}

u8 core_algo_des_cbc_sign( u8 *key, u8 key_len, u8 *iv, u8 *src_buf, u8 *sign_buf, u16 src_len)
{
	u16 i;

	if (src_len == 0)
	{
		return 0;
	}

	for (i=0; i<src_len; i+=8)
	{
		core_algo_xor(iv, src_buf, iv, 8);
		core_algo_des(key, key_len, iv, sign_buf, MODE_ENCRYPT);
		core_mm_copy(iv, sign_buf, 8);

		src_buf += 8;
	}
	
	return 8;
}

boolean core_algo_des_cbc_verify( u8 *key, u8 key_len, u8 *iv,  u8 *src_buf, u16 src_len,  u8 *sign_buf, u8 sign_len)
{
	core_algo_des_cbc_sign(key, key_len, iv, src_buf, iv, src_len);

	if (!core_mm_compare(iv, sign_buf, sign_len))
	{
		return TRUE;
	}

	return FALSE;
}

#endif