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

/**
  * @brief  Data display at head of screen.
  * @details  
  * @param[in]  
  * @retval  
  */
void LcdDisplayParam(void)
{
    LCD_Fill(30, 0, 300, 300, GREEN);
    LCD_ShowString(30, 30, 200, 24, 24, "WAVE STATUS");
    LCD_ShowString(30, 60, 200, 16, 16, "MAX = ");
    LCD_ShowString(30, 80, 200, 16, 16, "MIN = ");
    LCD_ShowString(30, 100, 200, 16, 16, "RMS = ");
    LCD_ShowString(30, 120, 200, 16, 16, "AVG = ");
    LCD_ShowString(30, 140, 200, 16, 16, "FREQ = ");
    LCD_ShowString(30, 160, 200, 16, 16, "IN CALI ");
    LCD_ShowString(30, 180, 200, 16, 16, "SCALE ");
    LCD_ShowString(30, 200, 200, 16, 16, "SAMP FREQ");
    LCD_ShowString(30, 220, 200, 16, 16, "IS AUTO FREQ");

    char max[30];
    char min[30];
    char rms[30];
    char avg[30];
    char freq[30];
    char cali[30];
    char scl[30];
    char sa_freq[30];
    char is_autofreq[30];
    char samp_val[30];
    

    while (1)
    {
        sprintf(max,    "%+#10.4f",    GlobalWave.maximum);
        sprintf(min,    "%+#10.4f",    GlobalWave.minimum);
        sprintf(rms,    "%+#10.4f",    GlobalWave.RmS);
        sprintf(avg,    "%+#10.4f",    GlobalWave.average);
        sprintf(freq,   "%+#10.4f",    GlobalWave.freq);
        sprintf(samp_val,   "%4d",    HAL_ADC_GetValue(&hadc1));

        if(flag_in_calibration)
            sprintf(cali, "YES      ");
        else
            sprintf(cali, "NO       ");

        sprintf(sa_freq, "%d",          GlobalConf.sampling_freq_kHz);

        switch (GlobalConf.gain_level)
        {
            case Gain_10x   :   sprintf(scl, "Gain_10x"); break;
            case Gain_1x    :   sprintf(scl, "Gain_1x"); break;
            case Gain_2x    :   sprintf(scl, "Gain_2x"); break;
            case Gain_5x    :   sprintf(scl, "Gain_5x"); break;
        default:
            sprintf(scl, "BAD PARAM");
            break;
        }

        if(GlobalConf.freq_Autoflag)
            sprintf(is_autofreq, "YES      ");
        else
            sprintf(is_autofreq, "NO       ");

        
//        taskENTER_CRITICAL();
		vTaskSuspendAll();
        LCD_ShowString(150, 60,  200, 16, 16,    max );
        LCD_ShowString(150, 80,  200, 16, 16,    min );
        LCD_ShowString(150, 100, 200, 16, 16,    rms );
        LCD_ShowString(150, 120, 200, 16, 16,    avg );
        LCD_ShowString(150, 140, 200, 16, 16,    freq );
        LCD_ShowString(150, 160, 200, 16, 16,    cali );
        LCD_ShowString(150, 180, 200, 16, 16,    scl );
        LCD_ShowString(150, 200, 200, 16, 16,    sa_freq );
        LCD_ShowString(150, 220, 200, 16, 16,    is_autofreq );
        LCD_ShowString(150, 240, 200, 16, 16,    samp_val );

//        taskEXIT_CRITICAL();
		xTaskResumeAll();
        osDelay(100);
    }
    

}

void LcdVertical(void)
{
    uint8_t selection_number = GlobalConf.gain_level + 1;

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
            LCD_ShowString(150, 380, 200, 16, 16, "1V");
            break;
        case 2:
            LCD_ShowString(150, 380, 200, 16, 16, "10V");
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
            switch (selection_number)
            {
            case  1  :   ConfigGain(Gain_10x);   break;
            case  2  :   ConfigGain(Gain_1x);    break;
            case  4  :   ConfigGain(Gain_2x);    break;
            case  3  :   ConfigGain(Gain_5x);    break;
            
            default:
                break;
            }
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
        osDelay(10);
    }
}



void LcdHorizontal(void)
{
//    uint8_t selection_number = 1;
	uint16_t freq_kHz = GlobalConf.sampling_freq_kHz;
	
	char txt[10];
    LCD_Fill(30, 300, 300, 600, GREEN);
    LCD_ShowString(30, 300, 200, 16, 16, "KEY_UP=BACK KEY_1=ENTER");
    LCD_ShowString(30, 320, 200, 16, 16, "KEY_0=NEXT KEY_2=PREV");
    LCD_ShowString(30, 350, 300, 24, 24, "HORIZONTAL CONFIG");
    LCD_ShowString(30, 380, 200, 16, 16, "SELECT -> ");

    while (1)
    {
		if(freq_kHz>100)
		{
			sprintf(txt,"%-4dkSa/s",GlobalConf.sampling_freq_kHz);
			LCD_ShowString(150, 380, 200, 16, 16, txt);
		}
		else
		{
			sprintf(txt,"%-4dkSa/s",freq_kHz);
			LCD_ShowString(150, 380, 200, 16, 16, txt);
		}
//        switch (selection_number)
//        {
//        case 1:
//            LCD_ShowString(150, 380, 200, 16, 16, "1MSa/s");
//            break;
//        case 2:
//            LCD_ShowString(150, 380, 200, 16, 16, "500kSa/s");
//            break;
//        case 3:
//            LCD_ShowString(150, 380, 200, 16, 16, "250kSa/s");
//            break;
//        case 4:
//            LCD_ShowString(150, 380, 200, 16, 16, "100kSa/s");
//            break;
//        case 5:
//            LCD_ShowString(150, 380, 200, 16, 16, "50kSa/s");
//            break;
//        case 6:
//            LCD_ShowString(150, 380, 200, 16, 16, "25kSa/s");
//            break;
//        case 7:
//            LCD_ShowString(150, 380, 200, 16, 16, "10kSa/s");
//            break;
//        case 8:
//            LCD_ShowString(150, 380, 200, 16, 16, "5kSa/s");
//            break;
//        case 9:
//            LCD_ShowString(150, 380, 200, 16, 16, "2kSa/s");
//            break;
//        case 10:
//            LCD_ShowString(150, 380, 200, 16, 16, "1kSa/s");
//            break;

//        default:
//            break;
//        }

        switch (KEY_Scan(0))
        {
        case KEY_0_PRES:
            NEXT_NUM(freq_kHz, 1, 101);
            LCD_Fill(150, 400, 300, 450, GREEN); // Clear 'APPLYED'
            LCD_Fill(150, 380, 350, 400, GREEN); // Clear selection line.
            break;
        case KEY_1_PRES:
			GlobalConf.freq_Autoflag = 0;
            ConfigFreqDiv(freq_kHz);
            LCD_ShowString(150, 400, 200, 16, 16, "APPLYED");
            break;
        case KEY_2_PRES:
            PREV_NUM(freq_kHz, 1, 101);
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
        osDelay(10);
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
            RequestCalibration(GlobalConf.gain_level, CaliBias);
            manual_cali_flag = 1;
            LCD_ShowString(150, 400, 200, 16, 16, "REQUESTED");
            break;
        case KEY_UP_PRES:
            LCD_Fill(150, 400, 300, 450, GREEN);
            return;
            break;

        default:
            break;
        }
        osDelay(10);
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
            RequestCalibration(GlobalConf.gain_level, CaliGain);
            LCD_ShowString(150, 400, 200, 16, 16, "REQUESTED");
            break;
        case KEY_UP_PRES:
            LCD_Fill(150, 400, 300, 450, GREEN);
            return;
            break;

        default:
            break;
        }
        osDelay(10);
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
        osDelay(10);
    }
}



void Connection(void)
{
	uint16_t i;
//	uint32_t myIPAddress;
	uint16_t databuf[100];
	char txt[20];
	LCD_Fill(30, 300, 300, 300, GREEN);
    LCD_ShowString(30, 300, 200, 16, 16, "KEY_UP=BACK KEY_1=Connection");
	LCD_ShowString(30, 320, 200, 16, 16, "KEY_0=Test_Read KEY_2=Test_Send");
	while(1)
	{
		//LCD_Fill(150, 380, 150 + 200, 380 + 16, GREEN);
		switch (KEY_Scan(0))
        {
        case KEY_0_PRES:
//			myIPAddress = ip4_addr_get_u32(&(tcp_Echoserver_PCB->local_ip));
//			sprintf(txt,"%u.%u.%u.%u",(*(((uint8_t*)&myIPAddress)+0)), (*(((uint8_t*)&myIPAddress)+1)), 
//							(*(((uint8_t*)&myIPAddress)+2)), (*(((uint8_t*)&myIPAddress)+3)));
			if(tcp_server_flag & (1<<6))
			{
				tcp_server_flag &= ~(1<<6);
				sprintf(txt,"%x%x%x%x%x",((uint16_t*)tcp_client_recvbuf)[0], ((uint16_t*)tcp_client_recvbuf)[1], ((uint16_t*)tcp_client_recvbuf)[2], \
											((uint16_t*)tcp_client_recvbuf)[3], ((uint16_t*)tcp_client_recvbuf)[4]);
				LCD_ShowString(150, 400, 300, 16, 16, txt);
			}
			else
			{
				LCD_ShowString(150, 400, 200, 16, 16, "nodata");
			}
            break;
        case KEY_1_PRES:
			LCD_ShowString(150, 400, 200, 16, 16, "wait");;
			if((tcp_server_flag &(1<<5)) == 0x00)
			{
				if(tcp_Connect() == ERR_OK)
				{
					//tcp_server_flag |= 1<<5;
					LCD_ShowString(150, 400, 200, 16, 16, "OK  ");
				}
				else
				{
					LCD_ShowString(150, 400, 200, 16, 16, "ERR ");
				}
			}
			else
			{
				tcp_close(tcp_Echoserver_PCB);
				tcp_server_flag &= ~(1<<5);
				LCD_ShowString(150, 400, 200, 16, 16, "Done");
			}
            break;
        case KEY_2_PRES:
			// send the FXXKING array of waveform here	
			if((tcp_server_flag &(1<<5)) != 0x00)
			{
//				for(i=0;i<100;i++)
//				{
//					databuf[i] = i;
//	//				tcp_server_sendbuf[i] = (uint16_t)(2000 + (1000 * sinf( (float)(i) / 30 *(3.14159f) )));
//				}
				databuf[0] = 0x4455;
				databuf[1] = 3;
				databuf[2] = 0;
				databuf[3] = 0;
				databuf[4] = 0xAABB;
				ETH_SendData(5,databuf,0xff00U);
				LCD_ShowString(150, 400, 200, 16, 16, "send");
			}
			else
			{
				LCD_ShowString(150, 400, 200, 16, 16, "noip");
			}
            break;
        case KEY_UP_PRES:
			LCD_Fill(150, 400, 300, 450, GREEN);
            return;
            break;

        default:
            break;
        }
		osDelay(10);
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
				Connection();
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
        osDelay(10);
    }
}

