/**
 * @file      usertask.h
 * @brief     
 * @details   
 * @author   ThunderDoge
 * @date      
 * @version   
 * @par       
 * Using encoding: utf-8
 */

#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "cmsis_os.h"
#include "ILI93xx.h"

#define _USE_MATH_DEFINES

#include "math.h"

typedef struct
{
    float vertical_scale_factor;
    float horizontal_scale_factor;

    float maximum;
    float minimum;
    float RmS;
    float freq;
}WaveformStats;

typedef struct
{
    float gain;
    float offset;
}AdcOffsetSheet;

extern WaveformStats Wave1;
extern AdcOffsetSheet OffsetConf1;


