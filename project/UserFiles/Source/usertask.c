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
#include "string.h"

#define RESET_CALI_PENDING                          \
    {                                               \
        safe_buffer_pending[CALI_ON_1V_SCALE] = 0;  \
        safe_buffer_pending[CALI_ON_2V_SCALE] = 0;  \
        safe_buffer_pending[CALI_ON_5V_SCALE] = 0;  \
        safe_buffer_pending[CALI_ON_10V_SCALE] = 0; \
    }
#define RESET_APPLY_BIAS_PENDING                 \
    {                                            \
        safe_buffer_pending[APPLY_BIAS_1V] = 0;  \
        safe_buffer_pending[APPLY_BIAS_2V] = 0;  \
        safe_buffer_pending[APPLY_BIAS_5V] = 0;  \
        safe_buffer_pending[APPLY_BIAS_10V] = 0; \
    }
#define RESET_APPLY_GAIN_PENDING                 \
    {                                            \
        safe_buffer_pending[APPLY_GAIN_1V] = 0;  \
        safe_buffer_pending[APPLY_GAIN_2V] = 0;  \
        safe_buffer_pending[APPLY_GAIN_5V] = 0;  \
        safe_buffer_pending[APPLY_GAIN_10V] = 0; \
    }

int DEBUG_FLAG = 0;

ONE_PARAMETER_TO_SEND temp_frame;
uint8_t cali_flag = 0;
enum TypesOfFrame cali_scale = RMS_ON_10V_SCALE;
uint16_t temp_buf[(sizeof(DATA_POINTS_TO_SEND) + sizeof(ONE_PARAMETER_TO_SEND))/sizeof(uint16_t)];

	
// Short funtion. force to send a END_CALI frame.
void SendEndCali(void)
{
	LoadStruct(&temp_frame, END_CALI, GlobalWave.RmS);
	ETH_SendData(sizeof(temp_frame) / 2, &temp_frame, 0);
}

/**
* @brief Function implementing the ADCHandleTask thread.
* @param argument: Not used
* @retval None
*/
void ADCHandleTaskFunction(void const *argument)
{

    static uint32_t wave_Period_Last = 0;
    static uint8_t lostfreqflag = 0;
    /* USER CODE BEGIN ADCHandleTaskFunction */
    uint32_t tick = osKernelSysTick();

    // Initiate Statistic structures. ------------
    InitializeWaveformSlidingBuffer(&wave_buffer);

    // init as 1MHz / 1000 = 10kHz sampling freq.

    ConfigFreqDiv(f100kSaps);
    ConfigGain(Gain_10x);

    Start_TIM_tiggered_ADC_DMA();

    for (;;)
    {
        		for(int i=0; i<=END_OF_TypesOfFrame; i++)
        		{
        			if( safe_buffer_pending[i] )
        			{
        				switch(i)
        				{
        					case CALI_ON_1V_SCALE   : RequestCalibration(Gain_10x, CaliBias);   cali_scale = RMS_ON_1V_SCALE;   RESET_CALI_PENDING; break;
        					case CALI_ON_2V_SCALE   : RequestCalibration(Gain_5x,  CaliBias);   cali_scale = RMS_ON_2V_SCALE;   RESET_CALI_PENDING; break;
        					case CALI_ON_5V_SCALE   : RequestCalibration(Gain_2x,  CaliBias);   cali_scale = RMS_ON_5V_SCALE;   RESET_CALI_PENDING; break;
        					case CALI_ON_10V_SCALE  : RequestCalibration(Gain_1x,  CaliBias);   cali_scale = RMS_ON_10V_SCALE;  RESET_CALI_PENDING; break;
							
							case APPLY_BIAS_1V		: GlobalConf.offset[GlobalConf.gain_level].bias = safe_buffer[APPLY_BIAS_1V]; 		cali_scale = APPLY_BIAS_1V;		RESET_APPLY_BIAS_PENDING;	break;
							case APPLY_BIAS_2V		: GlobalConf.offset[GlobalConf.gain_level].bias = safe_buffer[APPLY_BIAS_2V]; 		cali_scale = APPLY_BIAS_2V;		RESET_APPLY_BIAS_PENDING;	break;
							case APPLY_BIAS_5V		: GlobalConf.offset[GlobalConf.gain_level].bias = safe_buffer[APPLY_BIAS_5V]; 		cali_scale = APPLY_BIAS_5V;		RESET_APPLY_BIAS_PENDING;	break;
							case APPLY_BIAS_10V		: GlobalConf.offset[GlobalConf.gain_level].bias = safe_buffer[APPLY_BIAS_10V];		cali_scale = APPLY_BIAS_10V;	RESET_APPLY_BIAS_PENDING;	break;

        					case APPLY_GAIN_1V      : GlobalConf.offset[GlobalConf.gain_level].gain = safe_buffer[APPLY_GAIN_1V];      cali_scale = APPLY_GAIN_1V;     RESET_APPLY_GAIN_PENDING; break;
        					case APPLY_GAIN_2V      : GlobalConf.offset[GlobalConf.gain_level].gain = safe_buffer[APPLY_GAIN_2V];      cali_scale = APPLY_GAIN_2V;     RESET_APPLY_GAIN_PENDING; break;
        					case APPLY_GAIN_5V      : GlobalConf.offset[GlobalConf.gain_level].gain = safe_buffer[APPLY_GAIN_5V];      cali_scale = APPLY_GAIN_5V;     RESET_APPLY_GAIN_PENDING; break;
        					case APPLY_GAIN_10V     : GlobalConf.offset[GlobalConf.gain_level].gain = safe_buffer[APPLY_GAIN_10V];     cali_scale = APPLY_GAIN_10V;    RESET_APPLY_GAIN_PENDING; break;
                            case END_CALI           : ResetCalibration();safe_buffer_pending[END_CALI] = 0;		cali_scale = END_CALI;			SendEndCali();			break;
        					default:	break;
        				}
        			}
                }
        for (size_t i = 0; i < 2; i++)
        {
            if (flag_adc_buffer_ready[i])
            {
                flag_adc_buffer_processing[i] = 1;
                //自动改变采样频率
                if (GlobalConf.freq_Autoflag == 1)
                {
                    if ((wave_Period_Last > (100 + GlobalWave.period)) || ((wave_Period_Last + 100) < GlobalWave.period))
                    {
                        lostfreqflag = 0;
                        wave_Period_Last = GlobalWave.period;
                        if ((wave_Period_Last > 1900) && (wave_Period_Last < 65500))
                        {
                            ConfigFreqDivAuto(wave_Period_Last);
                        }
                        else
                        {
                            ConfigFreqDiv(f10kSaps);
                        }
                    }
                    else if (GlobalWave.freq < 0)
                    {
                        if (lostfreqflag == 0)
                        {
                            lostfreqflag = 1;
                            ConfigFreqDiv(f10kSaps);
                        }
                    }
                }
                else
                {
                    lostfreqflag = 0;
                }
                // Change the sampling freq and do again.
                Start_TIM_tiggered_ADC_DMA();

                // do 1 time Data_Get-Analyse loop
                if (i == 0)
                {
                    if (flag_in_calibration == 0)
                    { //非校准模式下发送波形
                        /* JiangHaoDong Classic
						adc_buffer_0[0] = 0xAA66;
						adc_buffer_0[1] = ((int16_t*)(&GlobalWave.freq))[0];
						adc_buffer_0[2] = ((int16_t*)(&GlobalWave.freq))[1];
						adc_buffer_0[3] = ADC_BUFFER_SIZE;
						ETH_SendData(ADC_BUFFER_SIZE + Intro_Size, adc_buffer_0, 0XAA66);
                        */

                        if (ETH_isReadyToSendData())
                        {
                            // Load data.
                            DATA_POINTS_TO_SEND *dptr = (void *)temp_buf;
                            Data_Struct_Init(dptr);
                            if (DEBUG_FLAG)
                            {
                                for (int i = 0; i < DATA_POINTS_LEN; i++) // DATA_POINTS_LEN == size data[]
                                {
                                    dptr->data[i] = i + 1000;
                                }
                            }
                            else
                            {
                                memcpy(&(dptr->data), (void *)(adc_buffer_0 + Intro_Size) , DATA_POINTS_LEN * sizeof(uint16_t));
                            }

                            // Load parameter.
                            ONE_PARAMETER_TO_SEND *pptr = (void *)(temp_buf + sizeof(DATA_POINTS_TO_SEND)/(sizeof(temp_buf[0])) );
                            Para_Struct_Init(pptr);
                            LoadStruct(pptr, FREQ, GlobalWave.freq);

                            ETH_SendData(sizeof(temp_buf) / 2, temp_buf, 1U);
                        }
                    }
                    RegularMeasure(adc_buffer_0 + Intro_Size, ADC_BUFFER_SIZE, &GlobalConf, &GlobalWave);
                    if (flag_in_calibration == 1)
                    { //校准模式下仅发送RMS和档位
                        if (ETH_isReadyToSendData())
                        {
                            LoadStruct(&temp_frame, cali_scale, GlobalWave.RmS);
                            ETH_SendData(sizeof(temp_frame) / 2, &temp_frame, 0);
                        }
                        if(manual_cali_flag)
                        {
                            GetCalibration();
                            manual_cali_flag = 0;
                        }
                    }
                }
                else
                {
                    if (flag_in_calibration == 0)
                    { //非校准模式下发送波形
                        /*
						adc_buffer_1[0] = 0xAA66;
						adc_buffer_1[1] = ((int16_t*)(&GlobalWave.freq))[0];
						adc_buffer_1[2] = ((int16_t*)(&GlobalWave.freq))[1];
						adc_buffer_1[3] = ADC_BUFFER_SIZE;
						ETH_SendData(ADC_BUFFER_SIZE + Intro_Size, adc_buffer_1, 0X5566);
                        */
                        if (ETH_isReadyToSendData())
                        {
                            // Load data.
                            DATA_POINTS_TO_SEND *dptr = (void *)temp_buf;
                            Data_Struct_Init(dptr);
                            if (DEBUG_FLAG)
                            {
                                for (int i = 0; i < DATA_POINTS_LEN; i++) // DATA_POINTS_LEN == size data[]
                                {
                                    dptr->data[i] = i + 2000;
                                }
                            }
                            else
                            {
                                memcpy(&(dptr->data), (void *)(adc_buffer_1 + Intro_Size), DATA_POINTS_LEN * sizeof(uint16_t));
                            }

                            // Load parameter.
                            ONE_PARAMETER_TO_SEND *pptr = (void *)(temp_buf + sizeof(DATA_POINTS_TO_SEND)/(sizeof(temp_buf[0])) );
                            Para_Struct_Init(pptr);
                            LoadStruct(pptr, FREQ, GlobalWave.freq);

                            ETH_SendData(sizeof(temp_buf) / 2, temp_buf, 1U);
                        }
                    }
                    RegularMeasure(adc_buffer_1 + Intro_Size, ADC_BUFFER_SIZE, &GlobalConf, &GlobalWave);
                    if (flag_in_calibration == 1)
                    { //校准模式下仅发送RMS和档位
                        if (ETH_isReadyToSendData())
                        {
                            LoadStruct(&temp_frame, cali_scale, GlobalWave.RmS);
                            ETH_SendData(sizeof(temp_frame) / 2, &temp_frame, 0);
                        }
                        if(manual_cali_flag)
                        {
                            GetCalibration();
                            manual_cali_flag = 0;
                        }
                    }
                }
                flag_adc_buffer_processing[i] = 0;
                flag_adc_buffer_ready[i] = 0;
                // Every 5ms :
            }
            else if (!flag_adc_sampling)
            {
                Start_TIM_tiggered_ADC_DMA();
            }
        }
        //		LoadStruct(&temp_frame, cali_scale,  GlobalWave.RmS);
        //		ETH_SendData(sizeof(temp_frame) /2 , &temp_frame, 0);
        osDelayUntil(&tick, 1U);
    }
}

void UI2ndDataDisplayTaskFunction(void const *argument)
{
    while (1)
    {
        LcdDisplayParam();
        osDelay(10);
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

    osThreadDef(UI2ndDataDisplayTask, UI2ndDataDisplayTaskFunction, osPriorityHigh, 0, 512);
    osThreadCreate(osThread(UI2ndDataDisplayTask), NULL);
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
        //		int i = 0;
        //		for(i=0; i<=END_OF_TypesOfFrame; i++)
        //		{
        //			if( safe_buffer_pending[i] )
        //			{
        //				switch(i)
        //				{
        //					case CALI_ON_1V_SCALE    : ConfigGain(Gain_10x);  cali_flag = 1; cali_scale = i;  RESET_CALI_PENDING; break;
        //					case CALI_ON_2V_SCALE    : ConfigGain(Gain_5x);   cali_flag = 1; cali_scale = i;  RESET_CALI_PENDING; break;
        //					case CALI_ON_5V_SCALE    : ConfigGain(Gain_2x);   cali_flag = 1; cali_scale = i;  RESET_CALI_PENDING; break;
        //					case CALI_ON_10V_SCALE   : ConfigGain(Gain_1x);   cali_flag = 1; cali_scale = i;  RESET_CALI_PENDING; break;
        //					case APPLY_BIAS : GlobalConf.offset[GlobalConf.gain_level].bias = safe_buffer[APPLY_BIAS]; RESET_APPLY_BIAS_PENDING; break;
        //					case APPLY_GAIN_1V       : GlobalConf.offset[GlobalConf.gain_level].gain = safe_buffer[APPLY_BIAS]; RESET_APPLY_GAIN_PENDING; break;
        //					case APPLY_GAIN_2V       : GlobalConf.offset[GlobalConf.gain_level].gain = safe_buffer[APPLY_BIAS]; RESET_APPLY_GAIN_PENDING; break;
        //					case APPLY_GAIN_5V       : GlobalConf.offset[GlobalConf.gain_level].gain = safe_buffer[APPLY_BIAS]; RESET_APPLY_GAIN_PENDING; break;
        //					case APPLY_GAIN_10V      : GlobalConf.offset[GlobalConf.gain_level].gain = safe_buffer[APPLY_BIAS]; RESET_APPLY_GAIN_PENDING; break;
        //					default:	break;
        //				}
        //			}
        //            if(cali_flag == 1)
        //            {
        //                cali_flag = 0;
        //                osDelay(500);
        //                switch (cali_scale)
        //                {
        //                    case CALI_ON_1V_SCALE    :  break;
        //                    case CALI_ON_2V_SCALE    :  break;
        //                    case CALI_ON_5V_SCALE    :  break;
        //                    case CALI_ON_10V_SCALE   :  break;
        //						default:	break;
        //                }
        //                LoadStruct(&temp_frame, cali_scale,  GlobalWave.RmS);
        //                ETH_SendData(sizeof(temp_frame) /2 , &temp_frame, 0);
        //            }
        //		}
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
