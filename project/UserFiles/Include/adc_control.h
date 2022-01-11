/**
 * @file      adc_control.h
 * @brief     
 * @details   
 * @author   ThunderDoge
 * @date      
 * @version   
 * @par       
 * Using encoding: utf-8
 */

#ifndef __ADC_CONTROL
#define __ADC_CONTROL

#include "main.h"
#include "adc.h"
#include "tim.h"
#include "stm32f4xx_hal_tim.h"

#define ADC_UPBOUND 3800
#define ADC_LOWBOUND 0
#define ADC_MID ((ADC_UPBOUND+ADC_LOWBOUND)/2)
#define ADC_RANGE (ADC_UPBOUND - ADC_LOWBOUND)
#define ADC_HALF_RANGE (ADC_UPBOUND - ADC_MID)

//#define BIAS_TO_INTEGER_BIAS(bias_volt,gain_lvl) (bias_volt  / GainLvlToRange(gain_lvl) * ADC_HALF_RANGE )
//#define FACTOR_SAMP_VAL_TO_VOLT(gain_lvl,gain_offset) ((GainLvlToRange(gain_lvl) / ADC_HALF_RANGE) * (1.0f + gain_offset))

#define TOO_MANY_EDGES_CRITERIA 40
#define TOO_LESS_EDGES_CRITERIA 5
#define HIGH_VALUE_CRITERIA ADC_UPBOUND-100
#define LOW_VALUE_EDGES_CRITERIA ADC_LOWBOUND+100
#define STALL_SCALE_CRITERIA 2047
#define WAVE_SLIDE_LENGTH 10
typedef enum GainLevel_e { 
    Gain_10x ,
    Gain_1x  ,
    Gain_2x  ,
    Gain_5x  ,
    END_OF_GAIN_LEVEL
} GainLevel_t;

typedef enum SampFreqLvl{
    f1MSaps		= 1000,
    f500kSaps	= 500,
    f250kSaps	= 250,
    f100kSaps	= 100,
    f50kSaps	= 50,
    f25kSaps	= 25,
    f10kSaps	= 10,
    f5kSaps		= 5,
    f2kSaps		= 2,
    f1kSaps		= 1
}SampFreqLvl_t;

extern uint16_t SampFreqLvlToDivNumber[10];

enum AdcMode
{
    AdcContinuousSampling=0,
    AdcCalibration
};

typedef enum WaveType_e { 
    UNKNOWN,
    DC,
    AC
} WaveType;


// 1=TRUE. 0=FALSE.
typedef struct WaveEvalueType_s
{
    uint8_t GOOD;
    uint8_t FAST;
    uint8_t SLOW;
    uint8_t LOW;
    uint8_t HIGH;
    uint8_t STALL; //means scale not fully used.
} WaveEvalueType;


//频率采样的结构体
typedef struct caliFreqType_s
{
	uint32_t shortTimeNum;
	uint16_t longTimeNum;
	uint16_t overTime;
}	caliFreqType;

// real waveform statistics.
typedef struct WaveformStats_s
{
    float maximum;  //	(Volt)
    float minimum;  //	(Volt)
    uint32_t edges;
    float RmS;      // 	(Volt)
    float average;  //	(Volt)
    float freq;     //	(Hz)
	uint32_t period;//	(0.25us)
	caliFreqType caliFreqStruct;

    WaveEvalueType Evalue;
    WaveType ACDCType;
} WaveformStats;


// Real volt = (volt - bias) * gain.
typedef struct
{
    float gain;
    float bias;
}AdcOffsetSheet;

typedef struct WaveMeasureConfig_s
{
    uint16_t sampling_freq_kHz;
    GainLevel_t gain_level;
    AdcOffsetSheet offset[END_OF_GAIN_LEVEL];
	uint8_t freq_Autoflag;
}WaveMeasureConfig_t;

typedef struct wsb
{
    uint32_t queue_count;
    uint32_t next_position;
    WaveformStats wave_buf[WAVE_SLIDE_LENGTH];
    WaveformStats output;
}WaveformSlidingBuffer;

typedef struct ct
{
    uint32_t times;
    WaveMeasureConfig_t conf;
    WaveformStats wave;
}CalibrationTickit;

// USER DEFINES
#define Intro_Size 4
#define ADC_BUFFER_SIZE 1500
#define TRIGGER_TIM htim2

// Global variables.
extern WaveformStats GlobalWave; 
extern WaveformStats TempWave;
extern WaveMeasureConfig_t GlobalConf;

extern WaveformSlidingBuffer wave_buffer;

extern uint8_t flag_adc_buffer_ready[2];
extern uint8_t flag_adc_buffer_processing[2];
extern uint8_t flag_adc_sampling;
extern uint8_t flag_in_calibration;


extern uint16_t adc_buffer_0[ADC_BUFFER_SIZE + Intro_Size];
extern uint16_t adc_buffer_1[ADC_BUFFER_SIZE + Intro_Size];

// Save/Load your configurations with Flash-on-Chip
int SaveMeasureConfig(WaveMeasureConfig_t* conf);
int LoadMeasureConfig(WaveMeasureConfig_t* conf);
typedef enum clt
{
    CaliNone,
    CaliGain,
    CaliBias
}CaliType;

// Calibration
int RequestCalibration(GainLevel_t gain_lvl, CaliType type);
int FeedCalibration(WaveformStats * wave, WaveformSlidingBuffer * buffer);
int GetCalibration(void);
void ResetCalibration(void);

void InitializeWaveformSlidingBuffer(WaveformSlidingBuffer* buffer);

// Measurement
int RegularMeasure(uint16_t *buffer, uint32_t buf_length, WaveMeasureConfig_t *conf, WaveformStats *wave);
int FeedRegularSlidingBuffer(WaveformStats* wave, WaveformSlidingBuffer* buffer);
void GetRegularSlidingOutput(WaveformStats* wave, WaveformSlidingBuffer* buffer);
// Manual config
void ConfigGain(GainLevel_t selection_number);
void ConfigFreqDiv(uint16_t freq_kHz);
void ConfigFreqDivAuto(uint32_t period);

// Start/Stop TIM-triggered ADC convertion.
void Start_TIM_tiggered_ADC_DMA(void);
void Stop_TIM_tigger(void);




#endif // !__ADC_CONTROL
