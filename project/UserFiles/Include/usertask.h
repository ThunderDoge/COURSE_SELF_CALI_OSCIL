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
#include "gui.h"
#include "myLWIPAPP.h"
#include "tansmitt.h"

#define _USE_MATH_DEFINES

#include "math.h"

#include "adc_control.h"

extern uint8_t cali_flag;
extern enum TypesOfFrame cali_scale;

