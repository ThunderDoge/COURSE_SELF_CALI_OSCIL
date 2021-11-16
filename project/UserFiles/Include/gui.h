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

void LcdTest(void);
void LcdMenu(void);
void LcdData(void);

void ConfigGain(uint8_t selection_number);
void ConfigFreqDiv(uint8_t selection_number);

#endif // _GUI_H
