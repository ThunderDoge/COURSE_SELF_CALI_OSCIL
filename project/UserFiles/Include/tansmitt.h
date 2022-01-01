#ifndef _TANSMITT_H
#define _TANSMITT_H

#include "main.h"
#include "adc_control.h"
#include "usertask.h"

enum TypesOfFrame
{
    // Upper to Lower
    START_ALL_SCALE_CALI,   // 开始所有档�?
    CALI_ON_1V_SCALE,
    CALI_ON_2V_SCALE,
    CALI_ON_5V_SCALE,
    CALI_ON_10V_SCALE,

    APPLY_BIAS,

    APPLY_GAIN_1V,
    APPLY_GAIN_2V,
    APPLY_GAIN_5V,
    APPLY_GAIN_10V,

    // Lower to Upper
    RMS_ON_1V_SCALE,
    RMS_ON_2V_SCALE,
    RMS_ON_5V_SCALE,
    RMS_ON_10V_SCALE,

    CMD_SEQUENCE_ERR,
	
	END_OF_TypesOfFrame	// LET THIS SYMBOL BE THE END!
};

typedef __packed struct
{
	uint16_t frame_header; // = 0x4455;//帧头
	
    uint16_t type_of_frame;
    float value_of_frame;

	uint16_t frame_tail; // = 0xAABB;//帧尾 
}ONE_PARAMETER_TO_SEND;

extern float safe_buffer[END_OF_TypesOfFrame+1];
extern uint8_t safe_buffer_pending[END_OF_TypesOfFrame+1];

void LoadStruct(ONE_PARAMETER_TO_SEND* frame, enum TypesOfFrame type,  float value);
void TranmittDataAna(uint16_t *buffer_receive, uint32_t length);

#endif
//end
