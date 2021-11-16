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

typedef enum GainLevel_e { 
    Gain_10x =0x00,
    Gain_1x  =0x01,
    Gain_5x  =0x02,
    Gain_2x  =0x03
} GainLevel_t;

typedef enum SampFreqLvl{
    f1MSaps=1,
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
    float edges;
    float RmS;      // (Volt)
    float freq;     // (Hz)

    WaveEvalueType Evalue;
    WaveType ACDCType;
} WaveformStats;

    

#define TOO_MANY_EDGES_CRITERIA 30
#define TOO_LESS_EDGES_CRITERIA 10
#define HIGH_VALUE_CRITERIA 3900
#define LOW_VALUE_EDGES_CRITERIA 200
#define STALL_SCALE_CRITERIA 2047

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

// USER DEFINES
#define ADC_BUFFER_SIZE 3000
#define TRIGGER_TIM htim2

// Global variables.
extern WaveformStats Wave[2];
extern WaveMeasureConfig_t MeasureConf[2];

extern uint8_t flag_adc_buffer_ready[2];
extern uint8_t flag_adc_buffer_processing[2];

extern uint16_t adc_buffer_0[ADC_BUFFER_SIZE];
extern uint16_t adc_buffer_1[ADC_BUFFER_SIZE];

// Save/Load your configurations with Flash-on-Chip
int SaveMeasureConfig(WaveMeasureConfig_t* conf);
int LoadMeasureConfig(WaveMeasureConfig_t* conf);

// Calibration
int RequestCalibration(GainLevel_t gain_lvl);
int FeedCalibration(uint16_t* buffer);

// Measurement
int RegularMeasure(uint16_t* buffer);

// Manual config
void ConfigGain(uint8_t selection_number);
void ConfigFreqDiv(uint8_t selection_number);

// Start/Stop TIM-triggered ADC convertion.
void Start_TIM_tiggered_ADC_DMA(void);
void Stop_TIM_tigger(void);




#endif // !__ADC_CONTROL
