#ifndef __CORE_ALGORITHM_H__
#define __CORE_ALGORITHM_H__
#include "EM000201_config.h"
//#include "hw.h"
#include "EM000101.h"
/*
 * Crypto related definition
 */
#define GENERAL_NOT_SUPPORT         0xFF

#define MODE_ENCRYPT				1
#define MODE_DECRYPT				0

#define HASH_UPDATE					0
#define HASH_DOFINAL				1

// cipher mode
#define CIPHER_MODE_AES_CBC				1
#define CIPHER_MODE_AES_ECB				2
#define CIPHER_MODE_DES_CBC				3
#define CIPHER_MODE_DES_ECB				4
#define CIPHER_MOIDE_KOREAN_SEED_CBC	5
#define CIPHER_MODE_KOREAN_SEED_ECB		6
#define CIPHER_MODE_RSA					7

// message digest algorithm
#define MD_ALG_SHA				1
#define MD_ALG_MD5				2
#define MD_ALG_RIPEMD160		3
#define MD_ALG_SHA224			4
#define MD_ALG_SHA256			5
#define MD_ALG_SHA384			6
#define MD_ALG_SHA512			7


#define MAX_MD_ALG_SUPPORT		7

// block size
#define BLOCK_LENGTH_NULL			0
#define DES_BLOCK_LENGTH			8
#define AES_128_BLOCK_LENGTH		16
#define AES_192_BLOCK_LENGTH		24
#define AES_256_BLOCK_LENGTH		32

#define BLOCK_LENGTH_INVALID		0xFF

#define CRC16_LENGTH				2
#define CRC32_LENGTH				4

/**
 * MessageDigest related Constants
 */
#define SHA1_DIGEST_LENGTH					20
#define SHA1_STATE_LENGTH					64
#define MAX_SHA1_SUUPORT_BIT_COUNT			8

#define MD5_DIGEST_LENGTH					16
#define MD5_STATE_LENGTH					64
#define MAX_MD5_SUUPORT_BIT_COUNT			8

#define RIPEMD160_DIGEST_LENGTH				20
#define RIPEMD160_STATE_LENGTH				64
#define MAX_RIPEMD160_SUUPORT_BIT_COUNT		8

#define SHA224_DIGEST_LENGTH				28
#define SHA224_STATE_LENGTH					64
#define MAX_SHA_224_SUUPORT_BIT_COUNT		8

#define SHA256_DIGEST_LENGTH				32
#define SHA256_STATE_LENGTH					64
#define MAX_SHA_256_SUUPORT_BIT_COUNT		8

#define SHA384_DIGEST_LENGTH				48
#define SHA384_STATE_LENGTH					128
#define MAX_SHA_384_SUUPORT_BIT_COUNT		16

#define SHA512_DIGEST_LENGTH				64
#define SHA512_STATE_LENGTH					128
#define MAX_SHA_512_SUUPORT_BIT_COUNT		16


#define MAX_MD_DIGEST_LEN					64
#define MAX_MD_STATE_LEN					128
#define MAX_MD_BIT_COUNT_LEN				16



#define SECRET_KEY_ALGO_DES					0
#define SECRET_KEY_ALGO_AES					1
#define MAX_SECRET_KEY_LEN					32
#define MAX_SECRET_KEY_BLOCK_LEN			32
#define MAX_SECRET_KEY_ALGO_SUPPORT			3

#define SECRET_KEY_PAD_ISO9797_M1			0
#define SECRET_KEY_PAD_ISO9797_M2			1
#define SECRET_KEY_PAD_PKCS5				2
#define MAX_SECRET_KEY_PAD_SUPPORT			3

#if EM_000201_CONFIG_FEATURE_CRYPTO_OTHER
/**
 * Internal Methods definition
 */
void core_algo_xor(const u8 *buf1, const u8 *buf2, u8 *outBuf, u16 length);
u8 core_algo_luhn(u8 *indata, u16 inLength);

#endif

#if EM_000201_CONFIG_FEATURE_CRYPTO_PADDING
/*PADDING*/
u16 core_algo_padding_ISO9797_M1(u8 *src, u16 srcLen, u8 blockLen);
u16 core_algo_unpadding_ISO9797_M1(u8 *src, u16 srcLen, u8 blockLen);
u16 core_algo_padding_ISO9797_M2(u8 *src, u16 srcLen, u8 blockLen);
u16 core_algo_unpadding_ISO9797_M2(u8 *src, u16 srcLen, u8 blockLen);
u16 core_algo_padding_PKCS5(u8 *src, u16 srcLen, u8 blockLen);
u16 core_algo_unpadding_PKCS5(u8 *src, u16 srcLen, u8 blockLen);
void core_algo_padding_ISO9796(u8 *srcBuf, u16 blockLength, u8 *msg1, u16 msgLength, u8 *hash);
boolean core_algo_unpadding_ISO9796(u8 *srcBuf, u16 blockLength, u16 msgLength, u8 *hash);
u16 core_algo_unpadding_ISO9796_recoveryMessage(u8 *srcBuf, u16 blockLength);
#endif

#if EM_000201_CONFIG_FEATURE_CRYPTO_RANDOM
void core_algo_genRandom(u8 *outBuf, u16 randLength);
u8 core_algo_genRandomU8(void);
#endif

#if EM_000201_CONFIG_FEATURE_CRYPTO_CRC
/*CRC*/
#if EM_000201_CONFIG_FEATURE_CRYPTO_CRC8
u8 core_algo_iso3309_crc8(u8 crc8_iv, u8 *buf, u32 length);
void core_algo_iso3309_crc8_special(boolean is_last, u8* iv, u8 *buf ,u32 length);
#endif
#if EM_000201_CONFIG_FEATURE_CRYPTO_CRC16
u16 core_algo_iso3309_crc16(u16 crc16_iv, u8 *buf, u32 length);
void core_algo_iso3309_crc16_special(boolean is_last, u8 *iv, u8 *buf ,u32 length);
#endif
#if EM_000201_CONFIG_FEATURE_CRYPTO_CRC32
u32 core_algo_iso3309_crc32(u32 crc32_iv, u8 *buf ,u32 length);
void core_algo_iso3309_crc32_special(boolean is_last, u8 *iv, u8 *buf ,u32 length);
#endif
#endif

#if EM_000201_CONFIG_FEATURE_CRYPTO_SHA128
void core_algo_shaInitial();
boolean core_algo_shaUpdate(u8 *buf, u16 length);
void core_algo_shaFinal(u8 *buf);
#endif

#if EM_000201_CONFIG_FEATURE_CRYPTO_SHA224 || EM_000201_CONFIG_FEATURE_CRYPTO_SHA256
void core_algo_sha256Initial();
boolean core_algo_sha256Update(u8 *buf, u16 length);
void core_algo_sha256Final(u8 *buf);

void core_algo_sha224Initial();
boolean core_algo_sha224Update(u8 *buf, u16 length);
void core_algo_sha224Final(u8 *buf);
#endif

#if EM_000201_CONFIG_FEATURE_CRYPTO_SHA384 || EM_000201_CONFIG_FEATURE_CRYPTO_SHA512
void core_algo_sha384Initial();
boolean core_algo_sha384Update(u8 *buf, u16 length);
void core_algo_sha384Final(u8 *buf);

void core_algo_sha512Initial();
boolean core_algo_sha512Update(u8 *buf, u16 length);
void core_algo_sha512Final(u8 *buf);
#endif


#if EM_000201_CONFIG_FEATURE_CRYPTO_HMAC
void core_algo_hmacInitial(u8 *key, u16 key_len, u8 *key_pad0, u8 *key_pad1);
boolean core_algo_hmacUpdate(u8 *buf, u16 length);
void core_algo_hmacFinal(u8 *buf, u8 *key_pad1);

void core_algo_hmac_sha256(u8* key, u16 keyLength,u8* srcBuf, u16 length,u8* destBuf);
#endif

#if EM_000201_CONFIG_FEATURE_CRYPTO_DES
/*DES*/
u16 core_algo_des_cbc_cipher(u8 mode,  u8 *key, u8 key_len, u8 *iv, u8 *src_buf, u8 *dest_buf, u16 src_len);
u16 core_algo_des_ecb_cipher(u8 mode,  u8 *key, u8 key_len,  u8 *src_buf, u8 *dest_buf, u16 src_len);
u8 core_algo_des_cbc_sign( u8 *key, u8 key_len, u8 *iv, u8 *src_buf, u8 *sign_buf, u16 src_len);
boolean core_algo_des_cbc_verify( u8 *key, u8 key_len, u8 *iv,  u8 *src_buf, u16 src_len,  u8 *sign_buf, u8 sign_len);
void core_algo_des(u8 *key, u8 keyLen,  u8 *src, u8 *out,u8 mode);
#endif


#if EM_000201_CONFIG_FEATURE_CRYPTO_AES
/*AES*/
u16 core_algo_aes_ecb_cipher(u8 mode,  u8 *key, u8 key_len, u8 blocklen,  u8 *src_buf, u8 *dest_buf, u16 src_len);
u16 core_algo_aes_cbc_cipher(u8 mode,  u8 *key, u8 key_len, u8 blocklen, u8 *iv, u8 iv_len,  u8 *src_buf,  u8 *dest_buf, u16 src_len);
u8 core_algo_aes_cbc_sign( u8 *key, u8 key_len, u8 blocklen, u8 *iv, u8 iv_len,  u8 *src_buf,  u8 *sign_buf, u16 src_len);
boolean core_algo_aes_cbc_verify( u8 *key, u8 key_len, u8 blocklen, u8 *iv, u8 iv_len,  u8 *src_buf, u16 src_len,  u8 *sign_buf, u8 sign_len);
u16 core_algo_aes_cbc_cipher_padding_M2(u8 mode,  u8 *key, u8 key_len, u8 blocklen, u8 *iv, u8 iv_len,  u8 *src_buf,  u8 *dest_buf, u16 src_len);
#endif


#if EM_000201_CONFIG_FEATURE_CRYPTO_HKDF
void core_algo_hkdf(u8 *ikm, u16 ikm_len, u8 *salt, u16 salt_len,u8* info,u16 infoLen, u16 L_len, u8 *odata);
#endif

#endif
