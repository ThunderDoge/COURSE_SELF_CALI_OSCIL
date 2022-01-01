#include "tansmitt.h"
#define mybuffer_length 10000

uint32_t start_point = 0;
uint16_t mybuffer[mybuffer_length];   //��λ���յ�֡ͷ��RMS_ON_1V_SCALE��RMS_ON_2V_SCALE��RMS_ON_5V_SCALE��
					   				//RMS_ON_10V_SCALE��CMD_SEQUENCE_ERR��֡β


//float safe_buffer[END_OF_TypesOfFrame+1];
//uint8_t safe_buffer_pending[END_OF_TypesOfFrame+1];



void Copy_to_End_of(uint16_t * to_buffer, uint16_t * from_buffer, uint32_t from_buffer_length)
{
	int i;
	/*if(start_point + from_buffer_length > mybuffer_length)
	{
		
	}*/
	for(i = 0; i < from_buffer_length; i++)
	{
		to_buffer[i + start_point] = from_buffer[i];
	}
	start_point = start_point + from_buffer_length; 
}

//��֡ͷ0x4455��index
int Find_Header(uint16_t * buffer, uint32_t length)
{
	int i;
	int head_result = 0;
	for(i = 0; i < length; i++)
	{
		if(buffer[i] == 0x4455)
		{
			head_result = i;
			break;
		}
	}
	return head_result;
}

//��֡β0xAABB��index 
int Find_Ender(uint16_t * buffer, uint32_t length)
{
	int end_result = -1;
	int head_result = Find_Header(buffer, length);
	if(buffer[head_result + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(float)] == 0xAABB)
	{
		end_result = head_result + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(float); 
	}
	else
	{
		end_result = -1;
	}
	return end_result;
}

//��ӦУ׼����������ʹ������ݸ�ʽ
void set_Cali_State(uint16_t cmd)
{
	switch((enum TypesOfFrame)cmd)
	{
		case(START_ALL_SCALE_CALI):
		{
			flag_in_calibration = 1;
			break;
		}
		case(CALI_ON_1V_SCALE):
		{
			flag_in_calibration = 1;
			ConfigGain(Gain_10x);
			cali_scale = RMS_ON_1V_SCALE;
			break;
		}
		case(CALI_ON_2V_SCALE):
		{
			flag_in_calibration = 1;
			ConfigGain(Gain_5x);
			cali_scale = RMS_ON_2V_SCALE;
			break;
		}
		case(CALI_ON_5V_SCALE):
		{
			flag_in_calibration = 1;
			ConfigGain(Gain_2x);
			cali_scale = RMS_ON_5V_SCALE;
			break;
		}
		case(CALI_ON_10V_SCALE):
		{
			flag_in_calibration = 1;
			ConfigGain(Gain_1x);
			cali_scale = RMS_ON_10V_SCALE;
			break;
		}
		default:
		{
			break;
		}
	}
}

//���ݽ�������λ���������ã�
void TranmittDataAna(uint16_t *buffer_receive, uint32_t length)
{
	int i;
	static uint32_t buffer = 0;
	static uint16_t caliCmd = 0xFFFF;
	static uint16_t unPackState;
	for(i = 0; i < length; i++)
	{
		switch(unPackState)
		{
			case(0):
			{
				if(buffer_receive[i] == 0x4455)
				{
					unPackState = 1;
				}
				break;
			}
			case(1):
			{
				caliCmd = buffer_receive[i];
				unPackState = 2;
				break;
			}
			case(2):
			{
				buffer = (uint32_t)buffer_receive[i] << 16;
				unPackState = 3;
				break;
			}
			case(3):
			{
				buffer |= (uint32_t)buffer_receive[i];
				unPackState = 4;
				break;
			}
			case(4):
			{
				if(buffer_receive[i] == 0xAABB)
				{
					set_Cali_State(caliCmd);
					flag_in_calibration = 1;
				}
				else
				{
					;
				}
				buffer = 0;
				caliCmd = 0xFFFF;
				unPackState = 0;
				break;
			}
			default:
			{
				buffer = 0;
				caliCmd = 0xFFFF;
				unPackState = 0;
				break;
			}
		}
	}
//	int header_index;
//	int ender_index;
//    Copy_to_End_of(mybuffer, buffer_receive, length);

//    // Data Ana
//    header_index = Find_Header(mybuffer, mybuffer_length);   // Find 0x4455
//    ender_index = Find_Ender(mybuffer, mybuffer_length);     //  0xAABB

//    ONE_PARAMETER_TO_SEND *t;
//    t = (void*)(& mybuffer[header_index]);
//	
//	// Copy ur data to somewhere safe.
//	safe_buffer[t->type_of_frame] = t->value_of_frame;
//	safe_buffer_pending[t->type_of_frame] = 1;
//	
//    // Erase mybuffer[0] ~ mybuffer[ender_index];
//	for(i = 0; i <= ender_index; i++)
//	{
//		mybuffer[i] = 0;
//	}
//	
//    // Copy mybuffer[ender_index + 1] -> mybuffer[0];
//	for(i = 0; i < mybuffer_length - sizeof(ONE_PARAMETER_TO_SEND); i++)
//	{
//		mybuffer[i] = mybuffer[i + ender_index + 1];  
//	}
//	
//	start_point = start_point - sizeof(ONE_PARAMETER_TO_SEND); 
}

//װ�ؽṹ�壨��λ���������ã�
void LoadStruct(ONE_PARAMETER_TO_SEND* frame, enum TypesOfFrame type,  float value)
{
	frame -> frame_header = 0x4455;
    frame -> type_of_frame = type;
    frame -> value_of_frame = value;
	frame -> frame_tail = 0xAABB;
}
