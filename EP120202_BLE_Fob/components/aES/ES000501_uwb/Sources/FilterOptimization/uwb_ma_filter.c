//Mathematical Average
#include <stdint.h>
#include "./uwb_filter_optimization.h"

// 1. 基本误差 20CM
// 2. 行动误差20CM


static uint8_t BubbleSort(uint16_t* intput_sequence, uint32_t sequence_size)
{
	uint32_t j = 0;
	uint32_t i = 0;
	uint32_t tem = 0;

	if(0 == sequence_size)
	{
		return 1;
	}

	for (i = 0; i < sequence_size -1; i ++)//sequence_size -1是因为不用与自己比较，所以比的数就少一个
	{
		int count = 0;
		for (j = 0; j < sequence_size -1 - i; j++)	//sequence_size -1-i是因为每一趟就会少一个数比较
		{
			if (intput_sequence[j] > intput_sequence[j+1])  //这是升序排法，前一个数和后一个数比较，如果前数大则与后一个数换位置
			{
				tem = intput_sequence[j];
				intput_sequence[j] = intput_sequence[j+1];
				intput_sequence[j+1] = tem;
				count = 1;
			}
		}
		if (count == 0)			//如果某一趟没有交换位置，则说明已经排好序，直接退出循环
			return 0;
	}

	return 0;
}

uint16_t RangingResultFitelMedianMean(uint16_t* intput_sequence, uint32_t sequence_size)
{
	uint32_t divisor = 0;
	uint16_t result = 0;

	if(0 != BubbleSort(intput_sequence, sequence_size))
	{
		result = 0;
	}

	for(uint32_t i = 1; i < sequence_size - 1 ; i++)
	{
		divisor = divisor + intput_sequence[i];
	}

	result = divisor/(sequence_size -2);

	return result;
}

