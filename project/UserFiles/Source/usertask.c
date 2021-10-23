/**
 * @file      usertask.c
 * @brief     
 * @details   
 * @author   ThunderDoge
 * @date      
 * @version   
 * @par
 * Using encoding: utf-8
 */

#include "usertask.h"

// USER DEFINES
#define ADC_BUFFER_SIZE 2048

// User Global Variale

WaveformStats Wave1;
AdcOffsetSheet OffsetConf1;



void WaveformDataAnalyze(uint16_t pbuffer, uint32_t buf_length, WaveformStats *wave, AdcOffsetSheet *offset)
{
    sampled_max=0;
    sampled_min=0xFFF;

    for(uint32_t i =0;i < buf_length;i++)   // find min and max;
    {
        if( pbuffer[i] > sampled_max )
            sampled_max = pbuffer[i];
        if( pbuffer[i] < sampled_min )
            sampled_min = pbuffer[i];
    }

    // now measure frequncy.
    uint16_t trig_voltage = (sampled_max + sampled_min) / 2; // set the trigger volt at middle.

    uint8_t is_cnt_risedge = 1;                 // here to select u wanna risedge or falledge.

    uint32_t cnt_of_risedge = 0;

    //count of trigger-volt crossing.
    for(uint32_t i =0;i < buf_length-1;i++)   // left off: pbuffer[buf_length-1], the last data point.
    {
        if(is_cnt_risedge)
        {
            if( pbuffer[i] <= trig_voltage && pbuffer[i+1] > trig_voltage)
                cnt_of_risedge ++;
        }
        else
        {
            if( pbuffer[i] >= trig_voltage && pbuffer[i+1] < trig_voltage)
                cnt_of_risedge ++;
        }
    }

    // data validation - condition -> 
}

void 

/**
* @brief Function implementing the ADCHandleTask thread.
* @param argument: Not used
* @retval None
*/
void ADCHandleTaskFunction(void const * argument)
{
    

    /* USER CODE BEGIN ADCHandleTaskFunction */
	uint16_t * pbuffer = malloc(sizeof(uint16_t) * ADC_BUFFER_SIZE);	// alloc the buffer.
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)pbuffer, ADC_BUFFER_SIZE/2);	// set DMA destination address, and start the ADC.
	HAL_TIM_Base_Start(&htim2);											// get TIM2 started.
	
	// the ADC convertion will trigger by TIM2 TRGO event.
	// TIM2 TRGO event = TIM2 update.
	
    for(;;)
    {
        osDelay(1);
    }
  
}

/**
* @brief Function implementing the UIHandleTask thread.
* @param argument: Not used
* @retval None
*/
void UIHandleTaskFunction(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}

/**
* @brief Function implementing the LwIPHandleTask thread.
* @param argument: Not used
* @retval None
*/
void LwIPHandleTaskFunction(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}

