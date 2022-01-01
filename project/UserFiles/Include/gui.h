/**
 * @file      gui.h
 * @brief     
 * @details   
 * @author   ThunderDoge
 * @date      
 * @version   
 * @par       
 * Using encoding: utf-8
 */

#ifndef _GUI_H
#define _GUI_H

#include "key.h"
#include "ILI93xx.h"
#include "cmsis_os.h"
#include "adc_control.h"
#include "stdio.h"
#include "tcp.h"
#include "string.h"
#include "myLWIPAPP.h"



void LcdTest(void);
void LcdMenu(void);
void LcdData(void);
void LcdDisplayParam(void);

void ConfigGain(GainLevel_t selection_number);
//void ConfigFreqDiv(SampFreqLvl_t selection_number);

#endif // _GUI_H
