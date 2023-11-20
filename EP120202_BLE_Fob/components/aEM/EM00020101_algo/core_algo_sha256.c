#include "EM000201.h"

#if EM_000201_CONFIG_FEATURE_CRYPTO_SHA256 || EM_000201_CONFIG_FEATURE_CRYPTO_SHA224
extern u8 core_digest[64];
extern u8 core_state[128];
extern u8 core_count[16];

/**
 * Constants definition
 */
static const u32 SHA256_K[64] = {
   0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
   0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
   0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
   0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
   0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
   0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
   0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
   0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static const u8 SHA256_IV[SHA256_DIGEST_LENGTH] = {
	0x6a, 0x09, 0xe6, 0x67, 0xbb, 0x67, 0xae, 0x85, 0x3c, 0x6e, 0xf3, 0x72, 0xa5, 0x4f, 0xf5, 0x3a,
	0x51, 0x0e, 0x52, 0x7f, 0x9b, 0x05, 0x68, 0x8c, 0x1f, 0x83, 0xd9, 0xab, 0x5b, 0xe0, 0xcd, 0x19
};

static const u8 SHA224_IV[SHA256_DIGEST_LENGTH] = {
	0xc1, 0x05, 0x9e, 0xd8, 0x36, 0x7c, 0xd5, 0x07, 0x30, 0x70, 0xdd, 0x17, 0xf7, 0x0e, 0x59, 0x39,
	0xff, 0xc0, 0x0b, 0x31, 0x68, 0x58, 0x15, 0x11, 0x64, 0xf9, 0x8f, 0xa7, 0xbe, 0xfa, 0x4f, 0xa4
};


/**
 * Static Methods implementation
 */
// DBL_INT_ADD treats two unsigned ints a and b as one 64-bit integer and adds c to it
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

u32 SHA256_M[128];
//#define SHA256_M	((u32 *)core_hw_sysBuf)

static void sha256Hash(u8 *state, u8 *digest)
{
	u8 i;
	u32 D[SHA256_DIGEST_LENGTH/4];
	u32 D1[SHA256_DIGEST_LENGTH/4];
	u32 t1, t2;

	core_algo_convert_u8tou32(digest, SHA256_DIGEST_LENGTH, D);
	core_mm_copy_u32(D1, D, SHA256_DIGEST_LENGTH/4);
	core_algo_convert_u8tou32(state, SHA256_STATE_LENGTH, SHA256_M);

	for (i=16; i<64; i++)
	{
		SHA256_M[i] = SIG1(SHA256_M[i-2]) + SHA256_M[i-7] + SIG0(SHA256_M[i-15]) + SHA256_M[i-16];
	}
	
	for (i = 0; i < 64; ++i)
	{
		t1 = D[7] + EP1(D[4]) + CH(D[4],D[5],D[6]) + SHA256_K[i] + SHA256_M[i];
		t2 = EP0(D[0]) + MAJ(D[0],D[1],D[2]);
		D[7] = D[6];
		D[6] = D[5];
		D[5] = D[4];
		D[4] = D[3] + t1;
		D[3] = D[2];
		D[2] = D[1];
		D[1] = D[0];
		D[0] = t1 + t2;
	}

	for (i=0; i<8; i++)
	{
		D1[i] += D[i];
	}

	// save resulting digest
	core_algo_convert_u32tou8(D1, SHA256_DIGEST_LENGTH/4, digest);

	// clear the state
	core_mm_set(state, 0, SHA256_STATE_LENGTH);
}

void core_algo_sha256Initial()
{
	// set digest to initial IV
	core_mm_copy(core_digest, (u8*)SHA256_IV, SHA256_DIGEST_LENGTH);
	// set all state byte to 0
	core_mm_set(core_state, 0, SHA256_STATE_LENGTH);
	// set count to 0
	core_mm_set(core_count, 0, 8);
}

boolean core_algo_sha256Update(u8 *buf, u16 length)
{
	u16 i;
	u8 data_count;
	u32 count1, count2;

	data_count = (u8)((core_dcm_readBig16(core_count+6) >> 3) & 0x3F);
	count1 = core_dcm_readBig32(core_count);
	count2 = core_dcm_readBig32(core_count+4);

	for (i=0; i<length; i++)
	{
		core_state[data_count++] = buf[i];

		count2 += 8;
		if (count2 == 0)
		{
			// check overflow, max is 2^64 - 1
			if (count1 == 0xFFFFFFFF)
			{
				return FALSE;
			}
			count1++;
		}
		
		// if state is full, update the digest
		if (data_count == SHA256_STATE_LENGTH)
		{
			sha256Hash(core_state, core_digest);
			data_count = 0;
		}
	}

	core_dcm_writeBig32(core_count, count1);
	core_dcm_writeBig32(core_count+4, count2);

	return TRUE;
}

void core_algo_sha256Final(u8 *buf)
{
	u8 data_count;

	data_count = (u8)((core_dcm_readBig16(core_count+6) >> 3) & 0x3F);
	core_state[data_count] = 0x80;
	
	// if state is full, update the digest
	if (data_count > 55)
	{
		sha256Hash(core_state, core_digest);
	}

	core_mm_copy(core_state+56, core_count, 8);

	sha256Hash(core_state, core_digest);

	core_mm_copy(buf, core_digest, SHA256_DIGEST_LENGTH);
}

void core_algo_sha224Initial()
{
	// set digest to initial IV
	core_mm_copy(core_digest, (u8*)SHA224_IV, SHA256_DIGEST_LENGTH);
	// set all state byte to 0
	core_mm_set(core_state, 0, SHA224_STATE_LENGTH);
	// set count to 0
	core_mm_set(core_count, 0, 8);
}

boolean core_algo_sha224Update(u8 *buf, u16 length)
{
	u16 i;
	u8 data_count;
	u32 count1, count2;

	data_count = (u8)((core_dcm_readBig16(core_count+6) >> 3) & 0x3F);
	count1 = core_dcm_readBig32(core_count);
	count2 = core_dcm_readBig32(core_count+4);

	for (i=0; i<length; i++)
	{
		core_state[data_count++] = buf[i];

		count2 += 8;
		if (count2 == 0)
		{
			// check overflow, max is 2^64 - 1
			if (count1 == 0xFFFFFFFF)
			{
				return FALSE;
			}
			count1++;
		}
		
		// if state is full, update the digest
		if (data_count == SHA224_STATE_LENGTH)
		{
			sha256Hash(core_state, core_digest);
			data_count = 0;
		}
	}

	core_dcm_writeBig32(core_count, count1);
	core_dcm_writeBig32(core_count+4, count2);

	return TRUE;
}

void core_algo_sha224Final(u8 *buf)
{
	u8 data_count;

	data_count = (u8)((core_dcm_readBig16(core_count+6) >> 3) & 0x3F);
	core_state[data_count] = 0x80;
	
	// if state is full, update the digest
	if (data_count > 55)
	{
		sha256Hash(core_state, core_digest);
	}

	core_mm_copy(core_state+56, core_count, 8);

	sha256Hash(core_state, core_digest);

	core_mm_copy(buf, core_digest, SHA224_DIGEST_LENGTH);
}
#endif