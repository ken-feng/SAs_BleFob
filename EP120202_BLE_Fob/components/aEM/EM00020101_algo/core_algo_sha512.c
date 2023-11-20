#include "EM000201.h"

#if EM_000201_CONFIG_FEATURE_CRYPTO_SHA384 || EM_000201_CONFIG_FEATURE_CRYPTO_SHA512

extern u8 core_digest[64];
extern u8 core_state[128];
extern u8 core_count[16];

#define COUNT_LENGTH_SHA_384				16
#define COUNT_LENGTH_SHA_512				16

/* define SHA intial value  & parameter */  
#define SHA512_BLOCKSIZE  128

#define ROTL64(a, n)		(((a)>>(n))|((a)<<(64-(n))))
#define SHR(a,n)			((a)>>(n))

#define CH_FUN(x,y,z)		(((x)&(y))^((~x)&(z)))
#define PARITY_FUN(x,y,z)	((x)^(y)^(z))
#define MAJ_FUN(x,y,z)		(((x)&(y))^((x)&(z))^((y)&(z)))

#define Ez512_FUN(x)		(ROTL64(x,28)^ROTL64(x,34)^ROTL64(x,39))
#define Eo512_FUN(x)		(ROTL64(x,14)^ROTL64(x,18)^ROTL64(x,41))
#define Oz512_FUN(x)		(ROTL64(x,1)^ROTL64(x,8)^SHR(x,7))
#define Oo512_FUN(x)		(ROTL64(x,19)^ROTL64(x,61)^SHR(x,6))

typedef unsigned long long u64;
const u8 r_SHA_512_IV[64] = {0x6a, 0x09, 0xe6, 0x67, 0xf3, 0xbc, 0xc9, 0x08, 
                            0xbb, 0x67, 0xae, 0x85, 0x84, 0xca, 0xa7, 0x3b, 
							0x3c, 0x6e, 0xf3, 0x72, 0xfe, 0x94, 0xf8, 0x2b, 
							0xa5, 0x4f, 0xf5, 0x3a, 0x5f, 0x1d, 0x36, 0xf1, 
							0x51, 0x0e, 0x52, 0x7f, 0xad, 0xe6, 0x82, 0xd1,
							0x9b, 0x05, 0x68, 0x8c, 0x2b, 0x3e, 0x6c, 0x1f,
							0x1f, 0x83, 0xd9, 0xab, 0xfb, 0x41, 0xbd, 0x6b,
							0x5b, 0xe0, 0xcd, 0x19, 0x13, 0x7e, 0x21, 0x79};
const u8 r_SHA_384_IV[64] = {0xcb, 0xbb, 0x9d, 0x5d, 0xc1, 0x05, 0x9e, 0xd8, 
							0x62, 0x9a, 0x29, 0x2a, 0x36, 0x7c, 0xd5, 0x07, 
							0x91, 0x59, 0x01, 0x5a, 0x30, 0x70, 0xdd, 0x17, 
							0x15, 0x2f, 0xec, 0xd8, 0xf7, 0x0e, 0x59, 0x39, 
							0x67, 0x33, 0x26, 0x67, 0xff, 0xc0, 0x0b, 0x31,
							0x8e, 0xb4, 0x4a, 0x87, 0x68, 0x58, 0x15, 0x11,
							0xdb, 0x0c, 0x2e, 0x0d, 0x64, 0xf9, 0x8f, 0xa7,
							0x47, 0xb5, 0x48, 0x1d, 0xbe, 0xfa, 0x4f, 0xa4};

const u64 r_KT512[80] = {
0x428a2f98d728ae22,0x7137449123ef65cd,0xb5c0fbcfec4d3b2f,0xe9b5dba58189dbbc,
0x3956c25bf348b538,0x59f111f1b605d019,0x923f82a4af194f9b,0xab1c5ed5da6d8118,
0xd807aa98a3030242,0x12835b0145706fbe,0x243185be4ee4b28c,0x550c7dc3d5ffb4e2,
0x72be5d74f27b896f,0x80deb1fe3b1696b1,0x9bdc06a725c71235,0xc19bf174cf692694,
0xe49b69c19ef14ad2,0xefbe4786384f25e3,0x0fc19dc68b8cd5b5,0x240ca1cc77ac9c65,
0x2de92c6f592b0275,0x4a7484aa6ea6e483,0x5cb0a9dcbd41fbd4,0x76f988da831153b5,
0x983e5152ee66dfab,0xa831c66d2db43210,0xb00327c898fb213f,0xbf597fc7beef0ee4,
0xc6e00bf33da88fc2,0xd5a79147930aa725,0x06ca6351e003826f,0x142929670a0e6e70,
0x27b70a8546d22ffc,0x2e1b21385c26c926,0x4d2c6dfc5ac42aed,0x53380d139d95b3df,
0x650a73548baf63de,0x766a0abb3c77b2a8,0x81c2c92e47edaee6,0x92722c851482353b,
0xa2bfe8a14cf10364,0xa81a664bbc423001,0xc24b8b70d0f89791,0xc76c51a30654be30,
0xd192e819d6ef5218,0xd69906245565a910,0xf40e35855771202a,0x106aa07032bbd1b8,
0x19a4c116b8d2d0c8,0x1e376c085141ab53,0x2748774cdf8eeb99,0x34b0bcb5e19b48a8,
0x391c0cb3c5c95a63,0x4ed8aa4ae3418acb,0x5b9cca4f7763e373,0x682e6ff3d6b2b8a3,
0x748f82ee5defb2fc,0x78a5636f43172f60,0x84c87814a1f0ab72,0x8cc702081a6439ec,
0x90befffa23631e28,0xa4506cebde82bde9,0xbef9a3f7b2c67915,0xc67178f2e372532b,
0xca273eceea26619c,0xd186b8c721c0c207,0xeada7dd6cde0eb1e,0xf57d4f7fee6ed178,
0x06f067aa72176fba,0x0a637dc5a2c898a6,0x113f9804bef90dae,0x1b710b35131c471b,
0x28db77f523047d84,0x32caab7b40c72493,0x3c9ebe0a15c9bebc,0x431d67c49c100d4c,
0x4cc5d4becb3e42b6,0x597f299cfc657e2a,0x5fcb6fab3ad6faec,0x6c44198c4a475817
};
#define STATE_LENGTH_SHA_384				SHA384_STATE_LENGTH
#define STATE_LENGTH_SHA_512				SHA512_STATE_LENGTH

#define HASH_LENGTH_SHA_384					SHA384_DIGEST_LENGTH
#define HASH_LENGTH_SHA_512					SHA512_DIGEST_LENGTH

static void Hash_SHA_512_Core(u64 *w, u64 *s)
{
	u8 i,idx;
	u64 t1, t2;
	u64 a,b,c,d,e,f,g,h;

	a = s[0]; b = s[1];c = s[2]; d = s[3];
	e = s[4]; f = s[5];g = s[6]; h = s[7];

	//prepare message schedule
	for(i=0; i<80; i++)
	{
		idx = i&0x0f;  // mod 16
		if(i >= 16)
		{
			w[idx] = Oo512_FUN(w[(i-2)&0x0f])+w[(i-7)&0x0f]+Oz512_FUN(w[(i-15)&0x0f])+w[idx];
		}
	     
		t1 = h + Eo512_FUN(e) + CH_FUN(e,f,g) + r_KT512[i] + w[idx];
		t2 = Ez512_FUN(a) + MAJ_FUN(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	s[0] += a; s[1] += b;s[2] += c; s[3] += d;
	s[4] += e; s[5] += f;s[6] += g; s[7] += h;
}

void r_u32tu8(u32 a , u8 *p)
{
	*p = (u8)(a>>24);
	*(p+1) = (u8)((a<<8)>>24);
    *(p+2) = (u8)((a<<16)>>24);
	*(p+3) = (u8)(a&0xff);
	
}
u32 r_u8tu32(u8 *p)
{
	return (*p)<<24 |  (*(p+1))<<16 | (*(p+2))<<8 |(*(p+3));
}
void r_u64tu8(u64 a,u8 *p)
{
	r_u32tu8((u32)(a>>32), p);
    r_u32tu8((u32)a, p+4);
}

u64 r_u8tu64(u8 *p)
{
   return (u64)r_u8tu32(p)<<32 | (u64)r_u8tu32(p+4);
}


typedef struct
{
	u32 v[4];
}U128_t;
typedef U128_t* U128;

boolean r_U128AddU32(U128 v1, u32 v2)
{
	u32 t;
	u8 i;
	boolean of = FALSE;

	for(i = 0; i < 128/32; i++)
	{
		t = v1->v[i] + v2 + ((of)?1:0);
		of = FALSE;
		if (t < v1->v[i])
		{
			of = TRUE;
		}
		v1->v[i] = t;
		v2 = 0;
	}
	return of;
}

void r_U128ToU8(u8* dat, U128 v1)
{
	u8 i;
	for(i = 0; i < 128/32; i++)
	{
		core_dcm_writeBig32(dat + i*4, v1->v[128/32 - i - 1]);
	}
}

void r_U8ToU128(U128 v, u8* dat)
{
	u8 i;
	for(i = 0; i < 128/32; i++)
	{
		v->v[128/32 - i - 1] = core_dcm_readBig32(dat + i*4);
	}
}

void r_sha512Hash(u8* state, u8* digest)
{
	u64 w[16];
	u64 s[8];
	u8 i;

	for(i=0; i<16; i++)
		w[i]  = r_u8tu64(state+8*i);
	for(i=0; i<8; i++)
		s[i]  = r_u8tu64(digest+8*i);

    Hash_SHA_512_Core(w,s);

	core_mm_set(state, 0, STATE_LENGTH_SHA_384);
	for(i=0; i<8; i++)
		r_u64tu8(s[i],digest + 8 * i);
}



void core_algo_sha512Initial()
{
	// set digest to initial IV
	core_mm_copy(core_digest, (u8*)r_SHA_512_IV, HASH_LENGTH_SHA_512);
	// set all state byte to 0
	core_mm_set(core_state, 0, STATE_LENGTH_SHA_512);
	// set count to 0
	core_mm_set(core_count, 0, COUNT_LENGTH_SHA_512);
}

boolean core_algo_sha512Update(u8 *buf, u16 length)
{
	u16 i;
	u8 idx;
	U128_t count;
	
	r_U8ToU128(&count, core_count);
	idx = (u8)((core_dcm_readBig16(core_count+COUNT_LENGTH_SHA_512 - 2) >> 3) & (STATE_LENGTH_SHA_512 - 1));

	for (i=0; i<length; i++)
	{
		core_state[idx++] = buf[i];

		if (r_U128AddU32(&count, 8))
		{
			return FALSE;
		}
		
		// if state is full, update the digest
		if (idx == STATE_LENGTH_SHA_512)
		{
			r_sha512Hash(core_state, core_digest);
			idx = 0;
		}
	}

	r_U128ToU8(core_count, &count);

	return TRUE;
}

void core_algo_sha512Final(u8 *buf)
{
	u8 data_count;

	data_count = (u8)((core_dcm_readBig16(core_count+COUNT_LENGTH_SHA_512 - 2) >> 3) & (STATE_LENGTH_SHA_512 - 1));
	core_state[data_count] = 0x80;
	
	// if state is full, update the digest
	if (data_count >= STATE_LENGTH_SHA_512 - COUNT_LENGTH_SHA_512)
	{
		r_sha512Hash(core_state, core_digest);
	}

	core_mm_copy(core_state+STATE_LENGTH_SHA_512 - COUNT_LENGTH_SHA_512, core_count, COUNT_LENGTH_SHA_512);

	r_sha512Hash(core_state, core_digest);
	core_mm_copy(buf, core_digest, SHA512_DIGEST_LENGTH);
}

void core_algo_sha384Initial()
{
	// set digest to initial IV
	core_mm_copy(core_digest, (u8*)r_SHA_384_IV, HASH_LENGTH_SHA_512);
	// set all state byte to 0
	core_mm_set(core_state, 0, STATE_LENGTH_SHA_384);
	// set count to 0
	core_mm_set(core_count, 0, COUNT_LENGTH_SHA_384);
}

boolean core_algo_sha384Update(u8 *buf, u16 length)
{
	u16 i;
	u8 idx;
	U128_t count;
	
	r_U8ToU128(&count, core_count);
	idx = (u8)((core_dcm_readBig16(core_count+COUNT_LENGTH_SHA_384 - 2) >> 3) & (STATE_LENGTH_SHA_384 - 1));

	for (i=0; i<length; i++)
	{
		core_state[idx++] = buf[i];

		if (r_U128AddU32(&count, 8))
		{
			return FALSE;
		}
		
		// if state is full, update the digest
		if (idx == STATE_LENGTH_SHA_384)
		{
			r_sha512Hash(core_state, core_digest);
			idx = 0;
		}
	}

	r_U128ToU8(core_count, &count);

	return TRUE;
}

void core_algo_sha384Final(u8 *buf)
{
	u8 data_count;

	data_count = (u8)((core_dcm_readBig16(core_count+COUNT_LENGTH_SHA_384 - 2) >> 3) & (STATE_LENGTH_SHA_384 - 1));
	core_state[data_count] = 0x80;
	
	// if state is full, update the digest
	if (data_count >= STATE_LENGTH_SHA_384 - COUNT_LENGTH_SHA_384)
	{
		r_sha512Hash(core_state, core_digest);
	}

	core_mm_copy(core_state+STATE_LENGTH_SHA_384 - COUNT_LENGTH_SHA_384, core_count, COUNT_LENGTH_SHA_384);

	r_sha512Hash(core_state, core_digest);

	core_mm_copy(buf, core_digest, SHA384_DIGEST_LENGTH);
}

#endif