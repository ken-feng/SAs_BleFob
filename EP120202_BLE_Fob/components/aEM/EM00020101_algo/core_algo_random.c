#include "EM000201.h"

#if EM_000201_CONFIG_FEATURE_CRYPTO_RANDOM

#include <stdlib.h>
#include <time.h>
#include "RNG_Interface.h"
void core_algo_genRandom(u8 *outBuf, u16 randLength)
{
	// srand(time(0));
	// while (randLength--)
	// {
	// 	*outBuf++ = core_algo_genRandomU8();//(u8)rand()%256;
	// }
	RNG_GetPseudoRandomNo(outBuf,randLength,NULL);
}

u8 core_algo_genRandomU8(void)
{
	return (u8)rand()%256;
}

#endif
