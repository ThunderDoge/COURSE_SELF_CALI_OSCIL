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

#define TOO_MANY_EDGES_CRITERIA 40
#define TOO_LESS_EDGES_CRITERIA 5
#define HIGH_VALUE_CRITERIA 3800
#define LOW_VALUE_EDGES_CRITERIA 300
#define STALL_SCALE_CRITERIA 2047
#define WAVE_SLIDE_LENGTH 10
typedef enum GainLevel_e { 
    Gain_10x =0x00,
    Gain_1x  =0x01,
    Gain_2x  =0x02,
    Gain_5x  =0x03
} GainLevel_t;

typedef enum SampFreqLvl{
    f1MSaps=0,
    f500kSaps,
    f250kSaps,
    f100kSaps,
    f50kSaps,
    f25kSaps,
    f10kSaps,
    f5kSaps,
    f2kSaps,
    f1kSaps
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


// real waveform statistics.
typedef struct WaveformStats_s
{
    float maximum;  // (Volt)
    float minimum;  // (Volt)
    uint32_t edges;
    float RmS;      // (Volt)
    float average;  // (Volt)
    float freq;     // (Hz)

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
    SampFreqLvl_t sampling_freq;
    GainLevel_t gain_level;
    AdcOffsetSheet offset;
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
#define ADC_BUFFER_SIZE 3000
#define TRIGGER_TIM htim2

// Global variables.
extern WaveformStats GlobalWave; 
extern WaveMeasureConfig_t GlobalConf;

extern WaveformSlidingBuffer wave_buffer;

extern uint8_t flag_adc_buffer_ready[2];
extern uint8_t flag_adc_buffer_processing[2];
extern uint8_t flag_adc_sampling;


extern uint16_t adc_buffer_0[ADC_BUFFER_SIZE];
extern uint16_t adc_buffer_1[ADC_BUFFER_SIZE];

// Save/Load your configurations with Flash-on-Chip
int SaveMeasureConfig(WaveMeasureConfig_t* conf);
int LoadMeasureConfig(WaveMeasureConfig_t* conf);

// Calibration
int RequestCalibration(GainLevel_t gain_lvl, uint32_t times, CalibrationTickit* tickit);
int FeedCalibration(uint16_t* buffer, CalibrationTickit* tickit);
int GetCalibration(CalibrationTickit* tickit);

void InitializeWaveformSlidingBuffer(WaveformSlidingBuffer* buffer);

// Measurement
int RegularMeasure(uint16_t *buffer, uint32_t buf_length, WaveMeasureConfig_t *conf, WaveformStats *wave);
int FeedRegularSlidingBuffer(WaveformStats* wave, WaveformSlidingBuffer* buffer);
void GetRegularSlidingOutput(WaveformStats* wave, WaveformSlidingBuffer* buffer);
// Manual config
void ConfigGain(GainLevel_t selection_number);
void ConfigFreqDiv(SampFreqLvl_t selection_number);

// Start/Stop TIM-triggered ADC convertion.
void Start_TIM_tiggered_ADC_DMA(void);
void Stop_TIM_tigger(void);




#endif // !__ADC_CONTROL
