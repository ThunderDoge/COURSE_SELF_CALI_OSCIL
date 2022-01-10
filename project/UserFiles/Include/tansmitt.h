#ifndef _TANSMITT_H
#define _TANSMITT_H

#include "stdint.h"

#define DATA_POINTS_LEN 500

//#if DATA_POINTS_LEN > ADC_BUFFER_SIZE
//    #error DATA TO SEND is larger than data source: ADC buffer
//#endif

enum TypesOfFrame
{
    // Upper to Lower
    START_ALL_SCALE_CALI,   //
    END_CALI,
    CALI_ON_1V_SCALE,
    CALI_ON_2V_SCALE,
    CALI_ON_5V_SCALE,
    CALI_ON_10V_SCALE,

    APPLY_GAIN_1V,      // gain
    APPLY_GAIN_2V,
    APPLY_GAIN_5V,
    APPLY_GAIN_10V,
	
	FREQ,

    APPLY_BIAS_1V,      // offset 
    APPLY_BIAS_2V,
    APPLY_BIAS_5V,
    APPLY_BIAS_10V,


    // Lower to Upper
    RMS_ON_1V_SCALE,
    RMS_ON_2V_SCALE,
    RMS_ON_5V_SCALE,
    RMS_ON_10V_SCALE,

    CMD_SEQUENCE_ERR,

    END_OF_TypesOfFrame
};

typedef struct 
{
	uint16_t para_frame_header;
	
    uint32_t type_of_frame;
    float value_of_frame;

	uint16_t frame_tail;
}ONE_PARAMETER_TO_SEND;

typedef struct  
{
	uint16_t data_frame_header;
	uint16_t data[DATA_POINTS_LEN];
	uint16_t frame_tail;
}DATA_POINTS_TO_SEND;

extern ONE_PARAMETER_TO_SEND Parameter;
extern DATA_POINTS_TO_SEND Data;

extern int safe_buffer_pending[CMD_SEQUENCE_ERR + 1];
extern float safe_buffer[CMD_SEQUENCE_ERR + 1];



void Para_Struct_Init(ONE_PARAMETER_TO_SEND* s);
void Data_Struct_Init(DATA_POINTS_TO_SEND* s);
void TranmittDataPointsAna(void *buffer_receive, uint32_t length);
void LoadStruct(ONE_PARAMETER_TO_SEND* frame, enum TypesOfFrame type,  float value);

#endif
//end
