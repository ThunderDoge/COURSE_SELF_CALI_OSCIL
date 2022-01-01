/**
 * @file      UserInit.c
 * @brief     用户更新
 * @details   
 * @author   ThunderDoge
 * @date      
 * @version   
 * @par
 * Using encoding: utf-8
 */

#include "userinit.h"

void userinit(void)
{
	// Init LCD screen.
	TFTLCD_Init();
	__HAL_TIM_ENABLE_IT(&htim9, TIM_IT_UPDATE|TIM_IT_CC1|TIM_IT_CC2);
	HAL_TIM_IC_Start(&htim9,TIM_CHANNEL_2);
	HAL_TIM_Base_Start(&htim9);
	TIM9->CCR1 = 0xFFFF;
}

