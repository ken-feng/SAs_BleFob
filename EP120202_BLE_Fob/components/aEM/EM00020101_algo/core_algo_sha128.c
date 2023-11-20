#include "EM000201.h"

#if EM_000201_CONFIG_FEATURE_CRYPTO_SHA128
u8 core_digest[64];
u8 core_state[128];
u8 core_count[16];



/**
 * Constants definition
 */
#define K1 0x5a827999
#define K2 0x6ed9eba1
#define K3 0x8f1bbcdc
#define K4 0xca62c1d6

static const u8 SHA1_IV[SHA1_DIGEST_LENGTH] = {0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89,
			0x98, 0xba, 0xdc, 0xfe, 0x10, 0x32, 0x54, 0x76,0xc3, 0xd2, 0xe1, 0xf0};

/**
 * Variables definition
 */
//SHA1_CONTEXT core_algo_shaContext;

/**
 * Static Methods implementation
 */
#define shaRotate(a, n) (((u32)(a) << (n)) | ((a) >> (32-(n))))

static u32 shaFtk(u32 B, u32 C, u32 D, u8 t)
{
	if (t < 20)
		return( ((B & C) | (~B & D)) + K1 );
	else if (t < 40)
		return( (B ^ C ^ D) + K2 );
	else if (t < 60)
		return( ((B & C) | (B & D) | (C & D)) + K3 );
	else
		return( (B ^ C ^ D) + K4 );
}

static void shaHash()
{
	u8 i, idx;
	u32 T;
	u32 S[SHA1_STATE_LENGTH/4];
	u32 D[SHA1_DIGEST_LENGTH/4];
	u32 D1[SHA1_DIGEST_LENGTH/4];

	// copy to the temporary variable
	core_algo_convert_u8tou32(core_digest, SHA1_DIGEST_LENGTH, D);
	core_mm_copy_u32(D1, D, SHA1_DIGEST_LENGTH/4);
	core_algo_convert_u8tou32(core_state, SHA1_STATE_LENGTH, S);

	// perform the 80 rounds, using the "alternate method" in which
    // the later values of S are computed in place
	for (i=0; i<80; i++)
	{
		idx = (u8)(i & 0x0F);
		if (i >= 16)
		{
			T = S[(i-3) & 0x0f] ^ S[(i-8) & 0x0f] ^ S[(i-14) & 0x0f] ^ S[idx];
			S[idx] = shaRotate(T, 1);
		}

		T = shaRotate(D[0], 5) + shaFtk(D[1], D[2], D[3], i) + D[4] + S[idx];
		D[4] = D[3];
		D[3] = D[2];
		D[2] = shaRotate(D[1], 30);
		D[1] = D[0];
		D[0] = T;
	}

	// add in the original values
	for (i=0; i<5; i++)
	{
		D1[i] += D[i];
	}

	// save resulting digest
	core_algo_convert_u32tou8(D1, SHA1_DIGEST_LENGTH/4, core_digest);

	// clear the state
	core_mm_set(core_state, 0, SHA1_STATE_LENGTH);
}


/**
 * Methods implementation
 */
void core_algo_shaInitial()
{
	// set digest to initial IV
	core_mm_copy(core_digest, (u8*)SHA1_IV, SHA1_DIGEST_LENGTH);
	// set all state byte to 0
	core_mm_set(core_state, 0, SHA1_STATE_LENGTH);
	// set count to 0
	core_mm_set(core_count, 0, 8);
}

boolean core_algo_shaUpdate(u8 *buf, u16 length)
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
		if (data_count == SHA1_STATE_LENGTH)
		{
			shaHash();
			data_count = 0;
		}
	}

	core_dcm_writeBig32(core_count, count1);
	core_dcm_writeBig32(core_count+4, count2);

	return TRUE;
}

void core_algo_shaFinal(u8 *buf)
{
	u8 data_count;

	data_count = (u8)((core_dcm_readBig16(core_count+6) >> 3) & 0x3F);
	core_state[data_count] = 0x80;
	
	// if state is full, update the digest
	if (data_count > 55)
	{
		shaHash();
	}

	core_mm_copy(core_state+56, core_count, 8);

	shaHash();

	core_mm_copy(buf, core_digest, SHA1_DIGEST_LENGTH);
}

#endif