#include "EM000201.h"

#if EM_000201_CONFIG_FEATURE_CRYPTO_HMAC

extern u8 core_digest[64];
extern u8 core_state[128];
extern u8 core_count[16];

#define HMAC_IPAD		0x36
#define HMAC_OPAD		0x5C

#define HMAC_MD_DLEN			SHA256_DIGEST_LENGTH
#define HMAC_MD_SLEN			SHA256_STATE_LENGTH
#define HMAC_MD_INIT_FUNC		core_algo_sha256Initial
#define HMAC_MD_UPDATE_FUNC		core_algo_sha256Update
#define HMAC_MD_DOFINAL_FUNC	core_algo_sha256Final

#define MD_BCNT					MAX_SHA_256_SUUPORT_BIT_COUNT

void core_algo_hmacInitial(u8 *key, u16 key_len, u8 *k_pad0, u8 *k_pad1)
{
	if (key_len > HMAC_MD_SLEN)
	{
		HMAC_MD_INIT_FUNC();
		HMAC_MD_UPDATE_FUNC(key, key_len);
		HMAC_MD_DOFINAL_FUNC(core_digest);
		key = core_digest;
		key_len = HMAC_MD_DLEN;
	}
	
	// set HMAC_IPAD and HMAC_OPAD
	core_mm_set(k_pad0, HMAC_IPAD, HMAC_MD_SLEN);
	core_mm_set(k_pad1, HMAC_OPAD, HMAC_MD_SLEN);

	// do xor
	core_algo_xor(key, k_pad0, k_pad0, key_len);
	core_algo_xor(key, k_pad1, k_pad1, key_len);

	HMAC_MD_INIT_FUNC();
	HMAC_MD_UPDATE_FUNC(k_pad0, HMAC_MD_SLEN);
}

boolean core_algo_hmacUpdate(u8 *buf, u16 length)
{
	if (length != 0)
	{
		return HMAC_MD_UPDATE_FUNC(buf, length);
	}
	
	return TRUE;
}

void core_algo_hmacFinal(u8* buf, u8 *k_pad1)
{
	u8 digest[MAX_MD_DIGEST_LEN];

	HMAC_MD_DOFINAL_FUNC(core_digest);
	core_mm_copy(digest, core_digest, HMAC_MD_DLEN);

	HMAC_MD_INIT_FUNC();
	HMAC_MD_UPDATE_FUNC(k_pad1, HMAC_MD_SLEN);
	HMAC_MD_UPDATE_FUNC(digest, HMAC_MD_DLEN);
	HMAC_MD_DOFINAL_FUNC(core_digest);

	core_mm_copy(buf,core_digest,HMAC_MD_DLEN);
}

void core_algo_hmac_sha256(u8* key, u16 keyLength,u8* srcBuf, u16 length,u8* destBuf)
{
	u8 k_pad0[64];
	u8 k_pad1[64];
	core_algo_hmacInitial(key,keyLength,k_pad0,k_pad1);
	core_algo_hmacUpdate(srcBuf,length);
	core_algo_hmacFinal(destBuf,k_pad1);
}

#endif