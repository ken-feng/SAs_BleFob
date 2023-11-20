#include <stdint.h>
#include "./uwb_filter_optimization.h"

//1. 结构体类型定义
typedef struct
{
	float LastP;//上次估算协方差 初始化值为0.02
	float Now_P;//当前估算协方差 初始化值为0
	float out;//卡尔曼滤波器输出 初始化值为0
	float Kg;//卡尔曼增益 初始化值为0
	float Q;//过程噪声协方差 初始化值为0.001
	float R;//观测噪声协方差 初始化值为0.543
}KFP;//Kalman Filter parameter

//2. 以高度为例 定义卡尔曼结构体并初始化参数
/*
KFP KFP_height={
	0.02,//上次估算协方差
	0,//当前估算协方差
	0,//滤波器输出
	0,//增益
	0.001,//过程噪声
	0.543//观测噪声
};
*/


/** *卡尔曼滤波器 *@param KFP *kfp 卡尔曼结构体参数 * float input 需要滤波的参数的测量值（即传感器的采集值） *@return 滤波后的参数（最优值） */
static float KalmanFilter(KFP *kfp,float input)
{

	//预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
	kfp->Now_P = kfp->LastP + kfp->Q;
	//卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
	kfp->Kg = kfp->Now_P / (kfp->Now_P + kfp->R);
	//更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
	kfp->out = kfp->out + kfp->Kg * (input -kfp->out);//因为这一次的预测值就是上一次的输出值
	//更新协方差方程: 本次的系统协方差付给 kfp->LastP 威下一次运算准备。
	kfp->LastP = (1-kfp->Kg) * kfp->Now_P;
	return kfp->out;
}
/** *调用卡尔曼滤波器 实践 */
//int height;
//int kalman_height=0;
//kalman_height = kalmanFilter(&KFP_height,(float)height);

static KFP KFP_UWBDistance;

uint16_t UWBKalmanFilter(uint16_t* unfiltered, uint32_t unfiltered_size)
{

	KFP_UWBDistance.LastP 	= 0.02;//上次估算协方差 初始化值为0.02
	KFP_UWBDistance.Now_P	= 0;//当前估算协方差 初始化值为0
	//KFP_UWBDistance.out		= 0;//卡尔曼滤波器输出 初始化值为0
	//KFP_UWBDistance.out		= unfiltered[0];
	KFP_UWBDistance.Kg		= 0;//卡尔曼增益 初始化值为0
	KFP_UWBDistance.Q		= 0.001;//过程噪声协方差 初始化值为0.001
	KFP_UWBDistance.R		= 0.543;//观测噪声协方差 初始化值为0.543

	uint8_t validcnt = 0;
	uint16_t res[5] = {0};
	uint16_t mean = 0;


	for(uint32_t j = 0; j < unfiltered_size; j++)
	{
		mean += unfiltered[j];
		if(0 != unfiltered[j])
		{
			validcnt ++;
		}
	}

	if(mean == 0)
		return 0;

	if(validcnt == 0)
		return 0;

	mean = mean/validcnt;

	KFP_UWBDistance.out = mean;

	for(uint32_t i = 0; i < validcnt; i++)
	{
		if(0 != unfiltered[i])
		{
			KalmanFilter(&KFP_UWBDistance,unfiltered[i]);
		}
	}

	return (uint16_t)KFP_UWBDistance.out;
}
