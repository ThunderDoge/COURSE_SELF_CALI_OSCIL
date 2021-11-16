#include "adc_control.h"
#include "cmsis_os.h"
#include "math.h"

// User Global Variale
uint32_t buffer_blocked_count = 0;

// Global config & statistic storage.
WaveformStats Wave1;
AdcOffsetSheet OffsetConf1;

// ADC data buffers
uint16_t adc_buffer_0[ADC_BUFFER_SIZE];
uint16_t adc_buffer_1[ADC_BUFFER_SIZE];

uint16_t data_scan_var;
uint8_t flag_adc_buffer_ready[2];
uint8_t flag_adc_buffer_processing[2];
uint32_t adc_buffer_time_stamp[2][2] = {0};



// List: from selection_number (SampFreqLvl) to TIM Divider number
// When TIM Set to 1MHz
uint16_t SampFreqLvlToDivNumber[10] =
{
    0,
    1,
    3,
    9,
    19,
    39,
    99,
    199,
    499,
    999
};

float GainLvlToRange[4]={10.0f, 1.0f, 2.0f, 5.0f};


float RmS_RawWaveform(uint16_t *pbuffer, uint32_t start_index, uint32_t end_index, WaveMeasureConfig_t* conf);

void InitializeWaveformStats(WaveformStats *wave)
{
    wave->maximum = 0;
    wave->minimum = 0;
    wave->edges = 0;
    wave->RmS = 0;
    wave->freq = 0;

    wave->Evalue.GOOD = 0;
    wave->Evalue.FAST = 0;
    wave->Evalue.SLOW = 0;
    wave->Evalue.LOW = 0;
    wave->Evalue.HIGH = 0;
    wave->ACDCType = UNKNOWN;
}


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
void Start_TIM_tiggered_ADC_DMA(void)
{
    uint32_t *pdest = NULL;
    uint8_t dest_buf_no = 2;
    portENTER_CRITICAL();

    HAL_TIM_Base_Stop(&TRIGGER_TIM);

    // Set the TIM AutoReload Value.

    // set DMA destination address & number to transmit (bytewide defined by register & CubeMX)
    //
    if (!flag_adc_buffer_ready[0])
    {
        pdest = (void*)adc_buffer_0;
        dest_buf_no = 0;
    }
    else if (!flag_adc_buffer_ready[1])
    {
        pdest = (void*)adc_buffer_1;
        dest_buf_no = 1;
    }
    else
    {
        buffer_blocked_count += 1;
    }

    if (pdest != NULL)
    {
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)pdest, ADC_BUFFER_SIZE);
        adc_buffer_time_stamp[dest_buf_no][1] = HAL_GetTick();
    }
    // but ADC not started yet.

    portEXIT_CRITICAL();

    // get TIM2 started. TIM2 Channel 2 Config as PWM gen, which will trigger ADC sampling automatically.
    HAL_TIM_PWM_Start(&TRIGGER_TIM, TIM_CHANNEL_2);
}

/**
  * @brief  Stop trigger TIM
  * @details  
  * @param[in]  
  * @retval  
  */
void Stop_TIM_tigger(void)
{
    HAL_TIM_PWM_Stop_IT(&TRIGGER_TIM, TIM_CHANNEL_2);
}

#define __GET_ADC_DMA_PAR(__HANDLE__)   (void*)(__HANDLE__->DMA_Handle->Instance->PAR)


    /**
* @brief Callback function tiggered when ADC-DMA transmition all (ADC_BUFFER_SIZE times) completed. 
* @retval None
*/
    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {

        if (__GET_ADC_DMA_PAR(hadc) == adc_buffer_0)
        {
            flag_adc_buffer_ready[0] = 1;
        }
        if (__GET_ADC_DMA_PAR(hadc) == adc_buffer_1)
        {
            flag_adc_buffer_ready[1] = 1;
        }
        
        Stop_TIM_tigger();
    }
}

void ConfigGain(uint8_t selection_number)
{
    if (selection_number >= 1 && selection_number <= 5)
    {
        HAL_GPIO_WritePin(PORT_GAIN_A_GPIO_Port, PORT_GAIN_A_Pin, ((selection_number & 0x1) ? GPIO_PIN_SET : GPIO_PIN_RESET));
        HAL_GPIO_WritePin(PORT_GAIN_B_GPIO_Port, PORT_GAIN_B_Pin, ((selection_number & 0x2) ? GPIO_PIN_SET : GPIO_PIN_RESET));
    }
}

/**
 * @brief  Set trigger timer freq 1MHz div by ?
 * @details  
 * @param[in]  
 * @retval  
 */
void ConfigFreqDiv(uint8_t selection_number)
{
    assert_param(selection_number <= 10 && selection_number >= 1);
    uint16_t n = SampFreqLvlToDivNumber[selection_number - 1];

    __HAL_TIM_SET_AUTORELOAD(&TRIGGER_TIM, n);
    __HAL_TIM_SET_COMPARE(&TRIGGER_TIM, TIM_CHANNEL_2, (n - 1) / 2);
}

/**
 * @brief  Calculate RmS with RAW data, assming that the waveform fixed the offsets.
 * @details  
 * @param[in]  pbuffer      data buffer
 * @param[in]  start_index  start on pbuffer[start_index]
 * @param[in]  end_index    end on pbuffer[end_index]
 * @retval  
 */
float RmS_FixedWaveform(uint16_t *pbuffer, uint32_t start_index, uint32_t end_index)
{
    float result;

    uint32_t sum_of_sqr = 0;

    for (size_t i = start_index; i <= end_index; i++)
    {
        int v = pbuffer[i] - 2047;
        sum_of_sqr += v*v ;
    }

    int cnt = end_index - start_index;

    float mean = sum_of_sqr / (cnt > 0 ? cnt : 1);

    return result = sqrt(mean);
}

/**
 * @brief  Calculate RmS with RAW data, need offsets infomation.
 * @details  
 * @param[in]  pbuffer      data buffer
 * @param[in]  start_index  start on pbuffer[start_index]
 * @param[in]  end_index    end on pbuffer[end_index]
 * @retval  
 */
float RmS_RawWaveform(uint16_t *pbuffer, uint32_t start_index, uint32_t end_index, WaveMeasureConfig_t* conf)
{
    float result;

    uint32_t sum_of_sqr = 0;


    int integer_bias = conf->offset.bias * 4095 / GainLvlToRange[(uint32_t)conf->gain_level] ;
    float gain = conf->offset.gain;

    // get sum of squares (bias substracted)
    for (size_t i = start_index; i <= end_index; i++)
    {
        int v = pbuffer[i] - 2047 - integer_bias ;
        sum_of_sqr += v*v ;
    }

    int cnt = end_index - start_index;

    float mean = sum_of_sqr / (cnt > 0 ? cnt : 1);

    // fix gain offset.
    return result = sqrt(mean) * gain;
}

/**
 * @brief  Analyze the waveform and write statistic to the stucture;
 * @details  
 * @param[in]  
 * @retval  
 */
void WaveformDataAnalyze(uint16_t *pbuffer, int buf_length, WaveformStats *wave, WaveMeasureConfig_t *config)
{
    uint32_t sampled_max = 0;
    uint32_t sampled_min = 0xFFF;

    for (uint32_t i = 0; i < buf_length; i++) // find min and max;
    {
        if (pbuffer[i] > sampled_max)
            sampled_max = pbuffer[i];
        if (pbuffer[i] < sampled_min)
            sampled_min = pbuffer[i];
    }

    // now measure frequncy.
    uint16_t trig_voltage = (sampled_max + sampled_min) / 2; // set the trigger volt at middle.

    uint16_t is_cnt_risedge = 1; // here to select u wanna rising-edge or falling-edge.

    uint32_t cnt_of_risedge = 0;

    int first_edging_index = -1;
    int last_edging_index = -1;

    // count of trigger-volt crossing.
    // and record FIRST & LAST crossing index.
    for (uint32_t i = 3; i < buf_length - 1; i++) // left off: pbuffer[buf_length-1], the last data point.
    {

        if (pbuffer[i] <= trig_voltage && pbuffer[i + 1] > trig_voltage) // detect risedge
        {
            if (is_cnt_risedge)
            {
                cnt_of_risedge++;
            }

            // record edging index
            if (first_edging_index == -1)
            {
                first_edging_index = i;
            }

            last_edging_index = i;
        }

        if (pbuffer[i] >= trig_voltage && pbuffer[i + 1] < trig_voltage) // detect falledge
        {
            if (!is_cnt_risedge)
            {
                cnt_of_risedge++;
            }

            // record edging index
            if (first_edging_index == -1)
            {
                first_edging_index = i;
            }

            last_edging_index = i;
        }
    }


    // register edges.
    wave->edges = cnt_of_risedge;
    if(cnt_of_risedge==0)
        cnt_of_risedge = 1;
    uint32_t dots_per_edges = (last_edging_index-first_edging_index) / cnt_of_risedge;

    // data validation - evaluation condition ->

    InitializeWaveformStats(wave);

    if (cnt_of_risedge > TOO_MANY_EDGES_CRITERIA)
        wave->Evalue.FAST = 1;
    if (cnt_of_risedge < TOO_LESS_EDGES_CRITERIA)
        wave->Evalue.SLOW = 1;
    if (sampled_max > HIGH_VALUE_CRITERIA)
        wave->Evalue.HIGH = 1;
    if (sampled_min < LOW_VALUE_EDGES_CRITERIA)
        wave->Evalue.LOW = 1;
    if (sampled_max-sampled_min < STALL_SCALE_CRITERIA)
        wave->Evalue.STALL = 1;

    if (wave->Evalue.FAST ||
        wave->Evalue.SLOW ||
        wave->Evalue.HIGH ||
        wave->Evalue.LOW)
    {
        wave->Evalue.GOOD = 0;
    }
    else
    {
        wave->Evalue.GOOD = 1;
    }

    // statistics.


    wave->freq = (cnt_of_risedge > 0 ? 1.0f / cnt_of_risedge : 0.0f);

    //BIG wave form statistics:
    wave->RmS = RmS_RawWaveform(pbuffer, first_edging_index, last_edging_index, config);

    

    // next measure configuration

}



