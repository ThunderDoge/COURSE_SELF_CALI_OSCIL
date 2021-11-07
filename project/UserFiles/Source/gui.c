/**
 * @file      gui.c
 * @brief     
 * @details   
 * @author   ThunderDoge
 * @date      
 * @version   
 * @par       
 * Using encoding: utf-8
 */

#include "gui.h"

void LcdTest(void)
{
    LCD_Clear(GREEN);
    LCD_ShowString(30, 40, 210, 24, 24, "What a nice day!");
    LCD_ShowString(30, 70, 200, 16, 16, "TFTLCD TEST");
    LCD_ShowString(30, 90, 200, 16, 16, "2020/7/29");
}

void LcdMenu(void)
{
    
}
