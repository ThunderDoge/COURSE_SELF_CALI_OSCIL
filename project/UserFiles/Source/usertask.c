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
#define ADC_BUFFER_SIZE 3000
#define TRIGGER_TIM htim2

// User Global Variale

WaveformStats Wave1;
AdcOffsetSheet OffsetConf1;

uint16_t adc_buffer[ADC_BUFFER_SIZE];

uint16_t data_scan_var;

// scan data on 'data_scan_var', for Ozone to view.
void scan_data(uint16_t *pbuffer, uint32_t num)
{
    for (uint32_t i = 0; i < num; i++)
    {
        data_scan_var = pbuffer[i];
    }
}

/**
* @brief Start TIM and ADC-DMA sampling. this function is polling mode, DO NOT use this in IRQs.
* @param timer_period_div : the TIM AutoReload Value +1. to control the source trigger freq 1MHz(config in CubeMX) div by [?]
* @retval None
*/
void Start_TIM_tiggered_ADC_DMA(uint32_t timer_period_div)
{
    portENTER_CRITICAL();

    HAL_TIM_Base_Stop(&TRIGGER_TIM);

    // Set the TIM AutoReload Value.
    __HAL_TIM_SetAutoreload(&TRIGGER_TIM, timer_period_div - 1);
    __HAL_TIM_SET_COMPARE(&TRIGGER_TIM, TIM_CHANNEL_2, (timer_period_div - 1) / 2);

    // set DMA destination address & number to transmit (bytewide defined by register & CubeMX)
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE);
    // but ADC not started yet.

    portEXIT_CRITICAL();

    // get TIM2 started. TIM2 Channel 2 Config as PWM gen, which will trigger ADC sampling automatically.
    HAL_TIM_PWM_Start(&TRIGGER_TIM, TIM_CHANNEL_2);
}

void Stop_TIM_tigger(void)
{
    HAL_TIM_PWM_Stop_IT(&TRIGGER_TIM, TIM_CHANNEL_2);
}

uint8_t flag_adc_buffer_ready;

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
    Start_TIM_tiggered_ADC_DMA(100);

    for (;;)
    {
        if (flag_adc_buffer_ready)
        {
            // do 1 time Data_Get-Analyse loop

            scan_data(adc_buffer, ADC_BUFFER_SIZE);
            flag_adc_buffer_ready = 0;

            // Change the sampling freq and do again.
            Start_TIM_tiggered_ADC_DMA(100);

            // Every 5ms :
            if (osKernelSysTick() - tick > 5U)
            {
                // Enter Task switching
                osDelayUntil(&tick, 1U);
            }
        }
    }
}

/**
* @brief Callback function tiggered when ADC-DMA transmition all (ADC_BUFFER_SIZE times) completed. 
* @retval None
*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {
        if (flag_adc_buffer_ready == 0)
            flag_adc_buffer_ready = 1;

        Stop_TIM_tigger();
    }
}

/**
* @brief Function implementing the UIHandleTask thread.
* @param argument: Not used
* @retval None
*/
void UIHandleTaskFunction(void const *argument)
{
    LCD_Clear(GREEN);
    LCD_ShowString(30, 40, 210, 24, 24, "What a nice day!");
    LCD_ShowString(30, 70, 200, 16, 16, "TFTLCD TEST");
    LCD_ShowString(30, 90, 200, 16, 16, "2020/7/29");
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        osDelay(500);
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
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

