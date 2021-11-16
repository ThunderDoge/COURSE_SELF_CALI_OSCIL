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



/**
* @brief Function implementing the ADCHandleTask thread.
* @param argument: Not used
* @retval None
*/
void ADCHandleTaskFunction(void const *argument)
{

    /* USER CODE BEGIN ADCHandleTaskFunction */
    uint32_t tick = osKernelSysTick();

    // Initiate Statistic structures. ------------

    // init as 1MHz / 1000 = 10kHz sampling freq.
    Start_TIM_tiggered_ADC_DMA();

    for (;;)
    {
        for (size_t i = 0; i < 2; i++)
        {
            if (flag_adc_buffer_ready[i])
            {
                flag_adc_buffer_processing[i] = 1;
                // Change the sampling freq and do again.
                Start_TIM_tiggered_ADC_DMA();


                // do 1 time Data_Get-Analyse loop

                flag_adc_buffer_processing[i] = 0;
                flag_adc_buffer_ready[i] = 0;
                // Every 5ms :
                if (osKernelSysTick() - tick > 5U)
                {
                    // Enter Task switching
                    osDelayUntil(&tick, 1U);
                }
            }
            
        }
        
    }
}


/**
* @brief Function implementing the UIHandleTask thread.
* @param argument: Not used
* @retval None
*/
void UIHandleTaskFunction(void const *argument)
{
	LcdTest();    
	osDelay(500);
	/* USER CODE BEGIN WHILE */
    while (1)
    {
		LcdMenu();
        osDelay(10);
    }
}

/**
* @brief Function implementing the LwIPHandleTask thread.
* @param argument: Not used
* @retval None
*/
void LwIPHandleTaskFunction(void const *argument)
{
    /* Infinite loop */
    for (;;)
    {
        osDelay(1);
    }
}

/**
* @brief Function implementing the DACHandleTask thread.
* @param argument: Not used
* @retval None
*/
void DACHandleTaskFunction(void const *argument)
{

    __HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE);
    HAL_TIM_Base_Start(&htim7);

    for (;;)
    {
        osDelay(1);
    }
}

