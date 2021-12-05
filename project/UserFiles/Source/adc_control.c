#include "adc_control.h"
#include "cmsis_os.h"
#include "math.h"

// User Global Variale
uint32_t buffer_blocked_count = 0;

// Global config & statistic storage.
WaveformStats GlobalWave;
WaveformStats TempWave;
WaveMeasureConfig_t GlobalConf;

WaveformSlidingBuffer wave_buffer;

// ADC data buffers
uint16_t adc_buffer_0[ADC_BUFFER_SIZE];
uint16_t adc_buffer_1[ADC_BUFFER_SIZE];

uint16_t data_scan_var;
uint8_t flag_adc_buffer_ready[2];
uint8_t flag_adc_buffer_processing[2];
uint8_t flag_adc_sampling;
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
        999};

float GainLvlToRange[4] = {10.0f, 1.0f, 2.0f, 5.0f};

float RmS_RawWaveform(uint16_t *pbuffer, uint32_t start_index, uint32_t end_index, WaveMeasureConfig_t *conf);
void InitializeWaveformSlidingBuffer(WaveformSlidingBuffer *buffer);

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
    if (flag_adc_sampling)
    {
        return;
    }

    uint32_t *pdest = NULL;
    uint8_t dest_buf_no = 2;
    portENTER_CRITICAL();

    HAL_TIM_Base_Stop(&TRIGGER_TIM);
    HAL_TIM_PWM_Stop(&TRIGGER_TIM, TIM_CHANNEL_2);

    // Set the TIM AutoReload Value.

    // set DMA destination address & number to transmit (bytewide defined by register & CubeMX)
    //
    if (!flag_adc_buffer_ready[0])
    {
        pdest = (void *)adc_buffer_0;
        dest_buf_no = 0;
    }
    else if (!flag_adc_buffer_ready[1])
    {
        pdest = (void *)adc_buffer_1;
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
        flag_adc_sampling = 1;
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

#define __GET_ADC_DMA_M0AR(__HANDLE__) (void *)(__HANDLE__->DMA_Handle->Instance->M0AR)

/**
* @brief Callback function tiggered when ADC-DMA transmition all (ADC_BUFFER_SIZE times) completed. 
* @retval None
*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {

        if (__GET_ADC_DMA_M0AR(hadc) == adc_buffer_0)
        {
            flag_adc_buffer_ready[0] = 1;
            flag_adc_sampling = 0;
        }
        if (__GET_ADC_DMA_M0AR(hadc) == adc_buffer_1)
        {
            flag_adc_buffer_ready[1] = 1;
            flag_adc_sampling = 0;
        }

        Stop_TIM_tigger();
    }
}

void ConfigGain(GainLevel_t selection_number)
{
    HAL_GPIO_WritePin(PORT_GAIN_A_GPIO_Port, PORT_GAIN_A_Pin, ((selection_number & 0x1) ? GPIO_PIN_SET : GPIO_PIN_RESET));
    HAL_GPIO_WritePin(PORT_GAIN_B_GPIO_Port, PORT_GAIN_B_Pin, ((selection_number & 0x2) ? GPIO_PIN_SET : GPIO_PIN_RESET));
    GlobalConf.gain_level = (GainLevel_t)selection_number;
    InitializeWaveformSlidingBuffer(&wave_buffer);
}

/**
 * @brief  Set trigger timer freq 1MHz div by ?
 * @details  
 * @param[in]  
 * @retval  
 */
void ConfigFreqDiv(SampFreqLvl_t selection_number)
{
    GlobalConf.sampling_freq = (SampFreqLvl_t)selection_number;
    InitializeWaveformSlidingBuffer(&wave_buffer);
    
    uint16_t n = SampFreqLvlToDivNumber[selection_number];

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
        sum_of_sqr += v * v;
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
float RmS_RawWaveform(uint16_t *pbuffer, uint32_t start_index, uint32_t end_index, WaveMeasureConfig_t *conf)
{
    float result;
    int v;
    int t;
    long long sum_of_sqr = 0;

    int integer_bias = conf->offset.bias * 2047 / GainLvlToRange[(uint32_t)conf->gain_level];
    float gain = (1.0f + conf->offset.gain) * GainLvlToRange[(uint32_t)conf->gain_level] ;

    // get sum of squares (bias substracted)
    for (size_t i = start_index; i <= end_index; i++)
    {
        v = pbuffer[i] - 2047 - integer_bias;
        t = v * v;
        sum_of_sqr += t;
    }

    int cnt = end_index - start_index;

    float mean = sum_of_sqr / cnt;

    // fix gain offset.
    return result = sqrt(mean) * (gain) / 2047.0f;
}

/**
 * @brief Calculate average.
 * @details  
 * @param[in]  
 * @retval  
 */
float Average_RawWaveformRawWaveform(uint16_t *pbuffer, uint32_t start_index, uint32_t end_index, WaveMeasureConfig_t *conf)
{
    float result;

    int integer_bias = conf->offset.bias * 2047 / GainLvlToRange[(uint32_t)conf->gain_level];
    float scale = GainLvlToRange[(uint32_t)conf->gain_level];
    float gain = (1.0f + conf->offset.gain) * scale / 2047.0f;

    int sum = 0;

    for (size_t i = start_index; i <= end_index; i++)
    {
        int v = pbuffer[i] - 2047 - integer_bias;
        sum += v;
    }

    int cnt = end_index - start_index;

    result = (sum / (float)cnt);

    // result *= gain;

    result = result * gain;

    return result;
}

/**
 * @brief  Analyze the waveform and write statistic to the stucture;
 * @details  
 * @param[in]  
 * @retval  
 */
void WaveformDataAnalyze(uint16_t *pbuffer, uint32_t buf_length, WaveformStats *wave, WaveMeasureConfig_t *config)
{
    int sampled_max = 0;
    int sampled_min = 4096;

    for (uint32_t i = 3; i < buf_length; i++) // find min and max;
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

        if (is_cnt_risedge)
        {
            if (pbuffer[i] <= trig_voltage && pbuffer[i + 1] > trig_voltage) // detect risedge
            {
                cnt_of_risedge++;
                if (first_edging_index == -1)
                {
                    first_edging_index = i;
                }

                last_edging_index = i;
            }
        }
        else
        {
            if (pbuffer[i] >= trig_voltage && pbuffer[i + 1] < trig_voltage) // detect falledge
            {
                cnt_of_risedge++;
                // record edging index
                if (first_edging_index == -1)
                {
                    first_edging_index = i;
                }

                last_edging_index = i;
            }
        }
    }

    uint32_t dots_per_edges;

    // register edges.
    if (cnt_of_risedge < 2)
    {
        dots_per_edges = 0XFFFF;
        wave->ACDCType = DC;
    }
    else
    {
        if (first_edging_index != -1 && last_edging_index != -1)
            dots_per_edges = (last_edging_index - first_edging_index) / (cnt_of_risedge - 1);
        else
            dots_per_edges = 0XFFFF;
    }



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
    if (sampled_max - sampled_min < STALL_SCALE_CRITERIA)
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

    wave->edges = cnt_of_risedge;
    if (cnt_of_risedge <= 2)
    {
        wave->freq = 0;
    }
    else
    {
        uint32_t count_per_edge = last_edging_index - first_edging_index / (cnt_of_risedge - 1);
        wave->freq = 1000000 / (SampFreqLvlToDivNumber[(uint32_t)config->sampling_freq] + 1) / count_per_edge;
    }

    // wave type.

    if (wave->freq <= 100)
    {
        wave->ACDCType = DC;
    }
    else
    {
        wave->ACDCType = AC;
    }

    // statistics.

    int integer_bias = config->offset.bias * 4095 / GainLvlToRange[(uint32_t)config->gain_level];
    //    float gain = (1.0f + config->offset.gain) * GainLvlToRange[(uint32_t)config->gain_level];

    // WARNING: the compiler MAY interpreter '*' in '* GainLvlToRange[config->gain_level]' into 'unpointerlize' rather than 'multiplate'.
    // so I have to do this to avoid ambiguousness.
    // FXXKING COMPILER.
    float scale = (float)GainLvlToRange[(uint32_t)config->gain_level];
    float gain_offset = (1.0f + config->offset.gain);

    wave->maximum = (sampled_max - 2047 - integer_bias) * scale * gain_offset / 4096.0f;
    //    wave->minimum = (float)(sampled_min - 2047 - integer_bias) * GainLvlToRange[(uint32_t)config->gain_level] * (1.0f + config->offset.gain) / 4096.0f;
    wave->minimum = (sampled_min - 2047 - integer_bias) * scale * gain_offset / 4096.0f;
    //BIG wave form statistics:
    if (first_edging_index == -1 || last_edging_index == -1)
    {
        first_edging_index = 0;
        last_edging_index = buf_length - 1;
    }

    if (wave->ACDCType == AC)
    {
        wave->RmS = RmS_RawWaveform(pbuffer, first_edging_index, last_edging_index, config);
        wave->average = Average_RawWaveformRawWaveform(pbuffer, first_edging_index, last_edging_index, config);
    }
    else
    {
        wave->RmS = RmS_RawWaveform(pbuffer, 0, buf_length - 1, config);
        wave->average = Average_RawWaveformRawWaveform(pbuffer, 0, buf_length - 1, config);
    }

    // next measure configuration
}

/**
 * @brief  Execute once regular measurement.
 * @details  
 * @param[in]  
 * @retval  0: OK
 */
int RegularMeasure(uint16_t *buffer, uint32_t buf_length, WaveMeasureConfig_t *conf, WaveformStats *wave)
{
    WaveformDataAnalyze(buffer, buf_length, wave, conf);
    return 0;
}

#define LOOP_NEXT(i, max) (i + 1 > max ? 0 : i + 1)
#define LOOP_PREV(i, max) (i == 0 ? max : i - 1)

/**
 * @brief  Feed Sliding buffer. Get average value.
 * @details  
 * @param[in]  wave the wave to feed (addr).
 * @param[in]  buffer the buffer addr.
 * @retval  0: not filled. 1:filled.
 */
int FeedRegularSlidingBuffer(WaveformStats *wave, WaveformSlidingBuffer *buffer)
{
    int retval = 0;
    buffer->wave_buf[buffer->next_position] = *wave;

    if (buffer->queue_count < WAVE_SLIDE_LENGTH)
    {
        buffer->queue_count++;
    }

    uint32_t n = buffer->next_position;
    {
        buffer->output.maximum += buffer->wave_buf[n].maximum;
        buffer->output.minimum += buffer->wave_buf[n].minimum;
        buffer->output.edges += buffer->wave_buf[n].edges;
        buffer->output.RmS += buffer->wave_buf[n].RmS;
        buffer->output.freq += buffer->wave_buf[n].freq;
        buffer->output.average += buffer->wave_buf[n].average;
    }

    if (buffer->queue_count == WAVE_SLIDE_LENGTH)
    {
        uint32_t n_1 = LOOP_PREV((buffer->next_position), (WAVE_SLIDE_LENGTH - 1));
        buffer->output.maximum -= buffer->wave_buf[n_1].maximum;
        buffer->output.minimum -= buffer->wave_buf[n_1].minimum;
        buffer->output.edges -= buffer->wave_buf[n_1].edges;
        buffer->output.RmS -= buffer->wave_buf[n_1].RmS;
        buffer->output.freq -= buffer->wave_buf[n_1].freq;
        buffer->output.average -= buffer->wave_buf[n_1].average;
        retval = 1;
    }
    // make it a loop queue.
    buffer->next_position = LOOP_NEXT((buffer->next_position), (WAVE_SLIDE_LENGTH - 1));

    return retval;
}

/**
 * @brief  Get output of that sliding buffer
 * @details  Notice that output shall div by WAVE_SLIDE_LENGTH
 * @param[in]  wave where to write to
 * @param[in]  buffer the sliding buffer
 * @retval  
 */
void GetRegularSlidingOutput(WaveformStats *wave, WaveformSlidingBuffer *buffer)
{
    wave->maximum = buffer->output.maximum / WAVE_SLIDE_LENGTH;
    wave->minimum = buffer->output.minimum / WAVE_SLIDE_LENGTH;
    wave->edges = buffer->output.edges / WAVE_SLIDE_LENGTH;
    wave->RmS = buffer->output.RmS / WAVE_SLIDE_LENGTH;
    wave->average = buffer->output.average / WAVE_SLIDE_LENGTH;
    wave->freq = buffer->output.freq / WAVE_SLIDE_LENGTH;
}

void InitializeWaveformSlidingBuffer(WaveformSlidingBuffer *buffer)
{
    buffer->queue_count = 0;
    buffer->next_position = 0;
    InitializeWaveformStats(&(buffer->output));
}

uint8_t flag_in_calibration=0;
CaliType global_cali_type = CaliNone;
/**
 * @brief  Request a new calibration.
 * @details  
 * @param[in]  gain_lvl
 * @retval  1: Request accept;
 *          0 Request unacceptable cuz another calibration in process.
 */
int RequestCalibration(GainLevel_t gain_lvl, CaliType type)
{
    if(flag_in_calibration)
    {
        return 0;
    }
    else
    {
        ConfigGain(gain_lvl);
        flag_in_calibration = 1;
        global_cali_type = type;
        return 1;
    }
}

/**
 * @brief  
 * @details  
 * @param[in]  wave the waveform statistic struct that enqueue the buffer
 * @param[in]  buffer the sliding buffer
 * @retval  1 buffer full, OK to get.
 *          0 buffer not yet filler.
 *          -1 !!!CALIBRATION NOT INITIALIZED.
 */
int FeedCalibration(WaveformStats * wave, WaveformSlidingBuffer * buffer)
{
    if(flag_in_calibration)
    {
        if(FeedRegularSlidingBuffer(&TempWave, &wave_buffer))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

/**
 * @brief  Get calibration data to GlobalConf the global configuration.
 * @details  
 * @retval  
 */
int GetCalibration(void)
{
    if(flag_in_calibration)
    {
        if (global_cali_type == CaliGain)
        {
            GlobalConf.offset.gain = wave_buffer.output.RmS / WAVE_SLIDE_LENGTH;
            ResetCalibration();
            return 1;
        }
        if (global_cali_type == CaliBias)
        {
            GlobalConf.offset.bias = wave_buffer.output.average / WAVE_SLIDE_LENGTH;
            ResetCalibration();
            return 1;
        }
        return -1;
    }
    else
    {return 0;}
}

void ResetCalibration(void)
{
    flag_in_calibration=0;
    global_cali_type = CaliNone;
    InitializeWaveformSlidingBuffer(&wave_buffer);
}


#undef LOOP_NEXT
#undef LOOP_PREV
