/*
 * uwb_filter_optimization.c
 *
 *  Created on: 2022年10月21日
 *      Author: JohnSong
 */

#include "../uwb_common_def.h"
#include "uwb_filter_optimization.h"
#define AllowedStepWithE 							(20 + 10)
#define BasicDeviationLimit 						(15)

//static int invalid_data_kick_out_with_5_data(uint16_t *history_pos, uint16_t *history_neg, uint16_t* bufResult)
int ranging_data_filter(uint16_t *history_pos, uint16_t *history_neg, uint16_t* bufResult)
{
	uint16_t        n                 = 0;
	uint16_t*       ptr16         	  = 0;
	uint16_t        Nbase_pos         = 0;
	uint16_t        Nbase_neg         = 0;

	ptr16 = bufResult;

	Nbase_pos = (* history_pos & 0x7FFF);
	Nbase_neg = (* history_neg & 0x7FFF);

	for (n = 3; n < 5; n++)
	{
		if (0 == ptr16[n])
		{
			ptr16[n] = 0xFFFF;
			Nbase_pos += AllowedStepWithE ;
			Nbase_neg = (Nbase_neg > AllowedStepWithE) ?  (Nbase_neg - AllowedStepWithE) : 0;
			continue;
		}

		if(BasicDeviationLimit > (ptr16[n] & 0x7FFF))
		{
			//测距值小于基准误差不做处理
			Nbase_pos = (ptr16[n] & 0x7FFF) + AllowedStepWithE;
			Nbase_neg = ((ptr16[n] & 0x7FFF) > AllowedStepWithE) ?  ((ptr16[n] & 0x7FFF) - AllowedStepWithE) : 0;
			continue;
		}


		if(((ptr16[n] & 0x7FFF) >= Nbase_pos) || ((ptr16[n] & 0x7FFF) <= Nbase_neg ))
		{
			ptr16[n] = 0xFFFF;
			Nbase_pos += AllowedStepWithE ;
			Nbase_neg = (Nbase_neg > AllowedStepWithE)  ?  (Nbase_neg - AllowedStepWithE) : 0;
		}
		else
		{
			Nbase_pos = (ptr16[n] & 0x7FFF) + AllowedStepWithE;
			Nbase_neg = ((ptr16[n] & 0x7FFF) > AllowedStepWithE) ?  ((ptr16[n] & 0x7FFF) - AllowedStepWithE) : 0;
			ptr16[n] = ptr16[n] - BasicDeviationLimit;
		}

	}

	*history_pos = Nbase_pos;
	*history_neg = Nbase_neg;
	return 0;
}

//For any value that is fixed, it will be set to 1 for the highest bit
//static int distance_check_and_fixup_with_5_data(uint16_t* last_history_data, uint16_t* indata)
int ranging_data_fixup(uint16_t* last_history_data, uint16_t* indata)
{

	int i = 0;
	int j = 0;
	uint16_t Distanbuff[6] = { 0 };

	if ((0xFFFF == indata[3]) && (0xFFFF == indata[4]))
	{
		*last_history_data = 0xFFFF;
		return 0;
	}

	Distanbuff[0] = *last_history_data;
	Distanbuff[1] = indata[0];
	Distanbuff[2] = indata[1];
	Distanbuff[3] = indata[2];
	Distanbuff[4] = indata[3];
	Distanbuff[5] = indata[4];

	//处理结束端
	for (i = 5; i > 0; i--)
	{
		if (0xFFFF != Distanbuff[i])
		{
			j = 5;
			while (j > i)
			{
				Distanbuff[j] = Distanbuff[i] | 0x8000;
				j--;
			}
			break;
		}
	}

	//处理开始端
	if (0xFFFF == Distanbuff[0])
	{
		for (i = 0; i < 6; i++)
		{
			if (0xFFFF != Distanbuff[i])
			{
				j = 0;
				while (j < i)
				{
					Distanbuff[j] = Distanbuff[i] | 0x8000;
					j++;
				}
				break;
			}
		}
	}

	if ((0xFFFF == Distanbuff[0]) || (0xFFFF == Distanbuff[5]))
	{
		*last_history_data = 0xFFFF;
		return 0;
	}

	//处理中间
	int FFCnt = 0;
	int ffidx = 0;
	int notffidx = 0;
	int diff = 0;
	uint16_t stepw = 0;

	for (i = 1; i < 6; i++)
	{
		if (0xFFFF == Distanbuff[i])
		{
			FFCnt++;
			if (0 == ffidx)
			{
				ffidx = i;
			}
		}
		else
		{
			if (0 != ffidx)
			{
				if ((Distanbuff[i]&0x7FFF) == (Distanbuff[ffidx - 1]&0x7FFF))
				{
					for (int x = 0; x < FFCnt; x++)
					{
						Distanbuff[ffidx + x] = Distanbuff[i] | 0x8000;
					}
				}
				else if ((Distanbuff[i]&0x7FFF) > (Distanbuff[ffidx - 1]&0x7FFF))
				{
					diff = (Distanbuff[i] & 0x7FFF) - (Distanbuff[ffidx - 1] & 0x7FFF);

					if (1 == FFCnt)
					{
						stepw = diff / 2;
					}
					else
					{
						stepw = diff / FFCnt;
					}

					for (int x = 0; x < FFCnt; x++)
					{
						Distanbuff[ffidx + x] = ((Distanbuff[ffidx + x - 1] & 0x7FFF) + stepw) | 0x8000;
					}
				}
				else
				{
					diff = (Distanbuff[ffidx - 1] & 0x7FFF) - (Distanbuff[i] & 0x7FFF);
					if (1 == FFCnt)
					{
						stepw = diff / 2;
					}
					else
					{
						stepw = diff / FFCnt;
					}
					for (int x = 0; x < FFCnt; x++)
					{
						if ((Distanbuff[ffidx + x - 1] & 0x7FFF) > stepw)
						{
							Distanbuff[ffidx + x] = ((Distanbuff[ffidx + x - 1] & 0x7FFF) - stepw) | 0x8000;
						}
						else
						{
							Distanbuff[ffidx + x] = Distanbuff[ffidx + x - 1] | 0x8000;
						}
					}
				}

				FFCnt = 0;
				ffidx = 0;
			}
		}
	}

	*last_history_data = Distanbuff[2] & 0x7FFF;
	indata[0] = Distanbuff[1];
	indata[1] = Distanbuff[2];
	indata[2] = Distanbuff[3];
	indata[3] = Distanbuff[4];
	indata[4] = Distanbuff[5];

	return 0;
}

void ranging_queue_refresh(ST_Ranging_Data* pst_ranging_dat, uint16_t new_data)
{
	if(0 == pst_ranging_dat)
		return ;

	pst_ranging_dat->bufDistQueue[0] = pst_ranging_dat->bufDistQueue[1];
	pst_ranging_dat->bufDistQueue[1] = pst_ranging_dat->bufDistQueue[2];
	pst_ranging_dat->bufDistQueue[2] = pst_ranging_dat->bufDistQueue[3];
	pst_ranging_dat->bufDistQueue[3] = pst_ranging_dat->bufDistQueue[4];
	pst_ranging_dat->bufDistQueue[4] = new_data;

}
