#include "dac_control.h"

extern TIM_HandleTypeDef htim7;

#define LOOP_NEXT(i, max) (i + 1 > max ? 0 : i + 1)
#define LOOP_PREV(i, max) (i == 0 ? max : i - 1)

uint16_t offset = 2047;
uint16_t a = 1800; 
uint16_t dac_div = 1000;
uint32_t out_val;
uint32_t t=0;


void SetDacSin(uint16_t f10kHZ_div)
{
    dac_div = f10kHZ_div;
    __HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE);
    HAL_TIM_Base_Start(&htim7);
}



void TIM7_Update_DAC_IT()
{
    t = LOOP_NEXT(t, 10000);
    out_val = offset + a* ceil(sinf( t *2 * 3.14159f / dac_div ));

    out_val = (out_val > 0 ? out_val : 0);
    out_val = (out_val < 4095 ? out_val : 4095);

    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, out_val);
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
}