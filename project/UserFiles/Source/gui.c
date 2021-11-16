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

#define TITLE_Y0 350
#define TITLE_Y1 380
#define TITLE_X0 30

#define SELECTION_Y0 380
#define SELECTION_Y1 400
#define SELECTION_X0 150

#define REACT_Y0 400
#define REACT_Y1 420
#define REACT_X0 150

void LcdTest(void)
{
    LCD_Clear(GREEN);
    LCD_ShowString(30, 40, 210, 24, 24, "What a nice day!");
    LCD_ShowString(30, 70, 200, 16, 16, "TFTLCD TEST");
    LCD_ShowString(30, 90, 200, 16, 16, "2020/7/29");
}

#define PREV_NUM(num, l, r)                  \
    {                                        \
        num = (num - 1 < l ? num : num - 1); \
    }
#define NEXT_NUM(num, l, r)                  \
    {                                        \
        num = (num + 1 > r ? num : num + 1); \
    }

void LcdDisplayParamOnce(void)
{
    LCD_Fill(30, 0, 300, 300, GREEN);
    LCD_ShowString(30, 30, 200, 16, 16, "WAVE STATUS");
}

void LcdVertical(void)
{
    uint8_t selection_number = 1;

    LCD_Fill(30, 300, 300, 500, GREEN);
    LCD_ShowString(30, 300, 200, 16, 16, "KEY_UP=BACK KEY_1=ENTER");
    LCD_ShowString(30, 320, 200, 16, 16, "KEY_0=NEXT KEY_2=PREV");
    LCD_ShowString(30, 350, 300, 24, 24, "VERTICAL CONFIG");
    LCD_ShowString(30, 380, 200, 16, 16, "SELECT -> ");

    while (1)
    {
        switch (selection_number)
        {
        case 1:
            LCD_ShowString(150, 380, 200, 16, 16, "10V");
            break;
        case 2:
            LCD_ShowString(150, 380, 200, 16, 16, "1V");
            break;
        case 3:
            LCD_ShowString(150, 380, 200, 16, 16, "2V");
            break;
        case 4:
            LCD_ShowString(150, 380, 200, 16, 16, "5V");
            break;

        default:
            break;
        }

        switch (KEY_Scan(0))
        {
        case KEY_0_PRES:
            NEXT_NUM(selection_number, 1, 4);
            LCD_Fill(150, 400, 350, 430, GREEN); // Clear 'APPLYED'
            LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.
            break;
        case KEY_1_PRES:
            ConfigGain(selection_number);
            LCD_ShowString(150, 400, 200, 16, 16, "APPLYED");
            break;
        case KEY_2_PRES:
            PREV_NUM(selection_number, 1, 4);
            LCD_Fill(150, 400, 300, 450, GREEN); // Clear 'APPLYED'
            LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.
            break;
        case KEY_UP_PRES:
            LCD_Fill(150, 400, 300, 450, GREEN);
            return;
            break;

        default:
            break;
        }
        osDelay(1);
    }
}



void LcdHorizontal(void)
{
    uint8_t selection_number = 1;

    LCD_Fill(30, 300, 300, 600, GREEN);
    LCD_ShowString(30, 300, 200, 16, 16, "KEY_UP=BACK KEY_1=ENTER");
    LCD_ShowString(30, 320, 200, 16, 16, "KEY_0=NEXT KEY_2=PREV");
    LCD_ShowString(30, 350, 300, 24, 24, "HORIZONTAL CONFIG");
    LCD_ShowString(30, 380, 200, 16, 16, "SELECT -> ");

    while (1)
    {
        switch (selection_number)
        {
        case 1:
            LCD_ShowString(150, 380, 200, 16, 16, "1MSa/s");
            break;
        case 2:
            LCD_ShowString(150, 380, 200, 16, 16, "500kSa/s");
            break;
        case 3:
            LCD_ShowString(150, 380, 200, 16, 16, "250kSa/s");
            break;
        case 4:
            LCD_ShowString(150, 380, 200, 16, 16, "100kSa/s");
            break;
        case 5:
            LCD_ShowString(150, 380, 200, 16, 16, "50kSa/s");
            break;
        case 6:
            LCD_ShowString(150, 380, 200, 16, 16, "25kSa/s");
            break;
        case 7:
            LCD_ShowString(150, 380, 200, 16, 16, "10kSa/s");
            break;
        case 8:
            LCD_ShowString(150, 380, 200, 16, 16, "5kSa/s");
            break;
        case 9:
            LCD_ShowString(150, 380, 200, 16, 16, "2kSa/s");
            break;
        case 10:
            LCD_ShowString(150, 380, 200, 16, 16, "1kSa/s");
            break;

        default:
            break;
        }

        switch (KEY_Scan(0))
        {
        case KEY_0_PRES:
            NEXT_NUM(selection_number, 1, 10);
            LCD_Fill(150, 400, 300, 450, GREEN); // Clear 'APPLYED'
            LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.
            break;
        case KEY_1_PRES:
            ConfigFreqDiv(selection_number);
            LCD_ShowString(150, 400, 200, 16, 16, "APPLYED");
            break;
        case KEY_2_PRES:
            PREV_NUM(selection_number, 1, 10);
            LCD_Fill(150, 400, 300, 450, GREEN); // Clear 'APPLYED'
            LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.
            break;
        case KEY_UP_PRES:
            LCD_Fill(150, 400, 300, 450, GREEN);
            return;
            break;

        default:
            break;
        }
        osDelay(1);
    }
}

/**
  * @brief  GUI Calibration menu.
  * @details  
  * @param[in]  
  * @retval  
  */

void LcdCalibrationDC(void)
{
    uint8_t selection_number = 1;

    LCD_Fill(30, 300, 300, 300, GREEN);
    LCD_ShowString(30, 300, 200, 16, 16, "KEY_UP=BACK KEY_1=ENTER");
    LCD_ShowString(30, 320, 200, 16, 16, "KEY_0=NEXT KEY_2=PREV");
    LCD_ShowString(30, 350, 300, 24, 24, "CALIBRATION / DC OFFSET");
    LCD_ShowString(30, 380, 200, 16, 16, "SELECT -> ");

    while (1)
    {
        LCD_ShowString(150, 380, 200, 16, 16, "PRES ENTER TO CALI");

        switch (KEY_Scan(0))
        {

        case KEY_1_PRES:
            // ------------------------------------------------------ APPLY YOUR CONFIG HERE

            LCD_ShowString(150, 400, 200, 16, 16, "REQUESTED");
            break;
        case KEY_UP_PRES:
            LCD_Fill(150, 400, 300, 450, GREEN);
            return;
            break;

        default:
            break;
        }
        osDelay(1);
    }
}

void LcdCalibrationGain(void)
{
    uint8_t selection_number = 1;

    LCD_Fill(30, 300, 300, 300, GREEN);
    LCD_ShowString(30, 300, 200, 16, 16, "KEY_UP=BACK KEY_1=ENTER");
    LCD_ShowString(30, 320, 200, 16, 16, "KEY_0=NEXT KEY_2=PREV");
    LCD_ShowString(30, 350, 300, 24, 24, "CALIBRATION / GAIN OFFSET");
    LCD_ShowString(30, 380, 200, 16, 16, "SELECT -> ");

    while (1)
    {
        LCD_ShowString(150, 380, 200, 16, 16, "PRES ENTER TO CALI");

        switch (KEY_Scan(0))
        {

        case KEY_1_PRES:
            // ------------------------------------------------------ APPLY YOUR CONFIG HERE

            LCD_ShowString(150, 400, 200, 16, 16, "REQUESTED");
            break;
        case KEY_UP_PRES:
            LCD_Fill(150, 400, 300, 450, GREEN);
            return;
            break;

        default:
            break;
        }
        osDelay(1);
    }
}
void LcdCalibration(void)
{
    uint8_t selection_number = 1;

    LCD_Fill(30, 300, 300, 300, GREEN);
    LCD_ShowString(30, 300, 200, 16, 16, "KEY_UP=BACK KEY_1=ENTER");
    LCD_ShowString(30, 320, 200, 16, 16, "KEY_0=NEXT KEY_2=PREV");
    LCD_ShowString(30, 350, 300, 24, 24, "MANUAL CALIBRATION");
    LCD_ShowString(30, 380, 200, 16, 16, "SELECT -> ");

    while (1)

    {
        LCD_Fill(150, 380, 150 + 200, 380 + 16, GREEN);
        switch (selection_number)
        {
        case 1:
            LCD_ShowString(150, 380, 200, 16, 16, "DC OFFSET CALI");
            break;
        case 2:
            LCD_ShowString(150, 380, 200, 16, 16, "GAIN OFFSET CALI");
            break;

        default:
            break;
        }

        switch (KEY_Scan(0))
        {
        case KEY_0_PRES:
            NEXT_NUM(selection_number, 1, 2);
            LCD_Fill(150, 400, 350, 430, GREEN);
            break;
        case KEY_1_PRES:
            // ------------------------------------------------------ APPLY YOUR CONFIG HERE
            switch (selection_number)
            {
            case 1:
                LcdCalibrationDC();
                break;
            case 2:
                LcdCalibrationGain();
                break;

            default:
                break;
            }
            LCD_ShowString(150, 400, 200, 16, 16, "APPLYED");
            break;
        case KEY_2_PRES:
            PREV_NUM(selection_number, 1, 2);
            LCD_Fill(150, 400, 300, 450, GREEN);
            break;
        case KEY_UP_PRES:
            LCD_Fill(150, 400, 300, 450, GREEN);
            return;
            break;

        default:
            break;
        }
        osDelay(1);
    }
}

/**
  * @brief  GUI main menu.
  * @details  
  * @param[in]  
  * @retval  
  */
void LcdMenu(void)
{
    uint8_t selection_number = 1;
    uint8_t selected = 0;
    LCD_Clear(GREEN);
    LCD_ShowString(30, 300, 200, 16, 16, "KEY_UP=BACK KEY_1=ENTER");
    LCD_ShowString(30, 320, 200, 16, 16, "KEY_0=NEXT KEY_2=PREV");
    LCD_ShowString(30, 350, 200, 24, 24, "MANUAL CONFIG");
    LCD_ShowString(30, 380, 200, 16, 16, "SELECT -> ");

    while (1)
    {
        switch (selection_number)
        {
        case 1:
            LCD_ShowString(150, 380, 200, 16, 16, "VERTICAL");
            break;
        case 2:
            LCD_ShowString(150, 380, 200, 16, 16, "HORIZONTAL");
            break;
        case 3:
            LCD_ShowString(150, 380, 200, 16, 16, "CALIBRATION");
            break;
        case 4:
            LCD_ShowString(150, 380, 200, 16, 16, "CONNECTION");
            break;

        default:
            break;
        }

        switch (KEY_Scan(0))
        {
        case KEY_0_PRES:
            NEXT_NUM(selection_number, 1, 4);
            break;
        case KEY_1_PRES:
            switch (selection_number)
            {
            case 1:
                LcdVertical();
                LCD_Fill(30, 350, 400, 378, GREEN);
                LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.

                LCD_ShowString(30, 350, 200, 24, 24, "MANUAL CONFIG");
                break;
            case 2:
                LcdHorizontal();
                LCD_Fill(30, 350, 400, 378, GREEN);
                LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.

                LCD_ShowString(30, 350, 200, 24, 24, "MANUAL CONFIG");
                break;
            case 3:
                LcdCalibration();
                LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.

                LCD_Fill(30, 350, 400, 378, GREEN);
                LCD_ShowString(30, 350, 200, 24, 24, "MANUAL CONFIG");
                break;
            case 4:

                LCD_Fill(30, 350, 400, 378, GREEN);
                LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.

                LCD_ShowString(30, 350, 200, 24, 24, "MANUAL CONFIG");
                break;

            default:
                break;
            }
            break;
        case KEY_2_PRES:
            PREV_NUM(selection_number, 1, 4);
            break;
        case KEY_UP_PRES:
            break;

        default:
            break;
        }
        osDelay(1);
    }
}

/**
  * @brief  Data display at head of screen.
  * @details  
  * @param[in]  
  * @retval  
  */
void LcdData(void)
{

}