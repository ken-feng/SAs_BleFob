#include "EM000201.h"

#if EM_000201_CONFIG_FEATURE_CRYPTO_HKDF
void core_algo_hkdf(u8 *ikm, u16 ikm_len, u8 *salt, u16 salt_len,u8* info,u16 infoLen, u16 L_len, u8 *odata)
{
	u8 offset, len, i;
	u8 cnt;
	u8 PRK_buff[32]={0};
	u8 Tn_buff[32]={0};
	u8 T_buff[256]={0};
	u8 k_pad0[64]={0};
	u8 k_pad1[64]={0};    
	
	u8 mdBuffer[128] ={0};

	if((L_len % 32) == 0)
	{
		len = L_len / 32;
	}
	else
	{
		len = L_len / 32 + 1;
	}
	
	/*first step:calc PRK*/
	core_algo_hmacInitial(salt, salt_len, k_pad0, k_pad1);
	core_algo_hmacUpdate(ikm, ikm_len);
	core_algo_hmacFinal(PRK_buff,k_pad1);

	/*second step: calc T[1] =  HMAC-Hash(PRK, T(0) | info | 0x01)*/
	cnt = 0x01;
	core_algo_hmacInitial( PRK_buff, 32, k_pad0, k_pad1);
	core_algo_hmacUpdate( info, infoLen);
	core_algo_hmacUpdate( &cnt, 1);
	core_algo_hmacFinal(Tn_buff,k_pad1);
	core_mm_copy(T_buff, Tn_buff, 32);
	offset = 32;
	
	for(i = 1; i < len; i++)
	{
		/*third step: calc T[n] =  HMAC-Hash(PRK, T(n-1) | info | n)*/
		core_algo_hmacInitial( PRK_buff, 32, k_pad0, k_pad1);
		core_algo_hmacUpdate( Tn_buff, 32);
		core_algo_hmacUpdate( info, infoLen);
		cnt = i+1;
		core_algo_hmacUpdate( &cnt, 1);
		core_algo_hmacFinal(Tn_buff, k_pad1);
		core_mm_copy((T_buff + offset), Tn_buff, 32);
		offset += 32;
	}
	core_mm_copy(odata, T_buff, L_len);
	return;
}

#endif