#include "tansmitt.h"
#include "string.h"
#ifdef STM32F407xx
#define mybuffer_length 100
#else
#define mybuffer_length 10000
#endif
#define data_to_plot_buffer_length 500

uint16_t mybuffer[mybuffer_length];

float safe_buffer[CMD_SEQUENCE_ERR + 1];
int safe_buffer_pending[CMD_SEQUENCE_ERR + 1];
int data_pending;

uint16_t data_to_plot_buffer[data_to_plot_buffer_length];
int start_point = 0; //index of first free location

void Para_Struct_Init(ONE_PARAMETER_TO_SEND *Parameter)
{
	Parameter->para_frame_header = 0x4455;
	Parameter->type_of_frame = 0U;
	Parameter->value_of_frame = 0.0f;
	Parameter->frame_tail = 0xAABB;
}

void Data_Struct_Init(DATA_POINTS_TO_SEND *Data)
{
	int i;
	Data->data_frame_header = 0x5544;
	for (i = 0; i < DATA_POINTS_LEN; i++)
	{
		Data->data[i] = 0;
	}
	Data->frame_tail = 0xAABB;
}

//  Find a valid frame of ONE_PARAMETER_TO_SEND.
// buffer : where to search
// length : length of [buffer]
// ender_index : this var will store index of ender
// return val = {   >=0 : the index of head the valid frame.
//                  -1  : no valid frame.
int Find_Parameter_Header(uint16_t *buffer, uint32_t length, int *ender_index)
{
	int i;
	int para_head_result = -1;
	// Find the header: 0x4455
	for (i = 0; i < length; i++)
	{
		if (buffer[i] == 0x4455)
		{
			para_head_result = i;
			break;
		}
	}
	if(para_head_result != -1)
	{
		// Verify the frame tail.
		ONE_PARAMETER_TO_SEND *sptr = (ONE_PARAMETER_TO_SEND *)(&buffer[para_head_result]);
		if (sptr->frame_tail == 0XAABB)
		{
			*ender_index = ((uint16_t *)(&(sptr->frame_tail)) - (uint16_t *)buffer); // gives the distance between 2 pointer {header, buffer}.
			if (*ender_index >= mybuffer_length)									 // check if ender_index is a exceeded access.
			{
				return -1;
			}

			return para_head_result;
		}
		else // ender error.
		{
			return -1;
		}
	}
	else
		return -1;
	
}

//  Find a valid frame of DATA_POINTS_TO_SEND.
// buffer : where to search
// length : length of [buffer]
// ender_index : this var will store index of ender
// return val = {   >=0 : the index of head the valid frame.
//                  -1  : no valid frame.
int Find_Data_Header(uint16_t *buffer, uint32_t length, int *ender_index)
{
	int i;
	int data_head_result = -1;
	// Find the header: 0x5544
	for (i = 0; i < length; i++)
	{
		if (buffer[i] == 0x5544)
		{
			data_head_result = i;
			break;
		}
	}

	if(data_head_result != -1)
	{
		// Verify the frame tail.
		DATA_POINTS_TO_SEND *sptr = (DATA_POINTS_TO_SEND *)(&buffer[data_head_result]);
		if (sptr->frame_tail == 0XAABB)
		{
			*ender_index = ((uint16_t *)(&(sptr->frame_tail)) - (uint16_t *)buffer); // gives the distance between 2 pointer {header, buffer}.
			if (*ender_index >= mybuffer_length)									 // check if ender_index is a exceeded access.
			{
				return -1;
			}

			return data_head_result;
		}
		else // ender error.
		{
			return -1;
		}	
	}
	else 
		return -1;
}
// move buffer[ender_index+1 ~ mybuffer_length] to head of mybuffer
void ClearFrameFromHeadTo(uint16_t *buffer, int ender_index)
{
	memset(buffer, 0, sizeof(*buffer) * ender_index);
	for (int i = ender_index + 1; i < mybuffer_length; i++)
	{
		buffer[i - ender_index - 1] = buffer[i];
	}
}

void ParameterAna(ONE_PARAMETER_TO_SEND *sptr)
{
	safe_buffer[sptr->type_of_frame] = sptr->value_of_frame;
    safe_buffer_pending[sptr->type_of_frame] = 1;
}

void DataAna(DATA_POINTS_TO_SEND *sptr)
{
	memcpy(data_to_plot_buffer, &(sptr->data), sizeof(data_to_plot_buffer));
    data_pending = 1;
}

// Get a received data into buffer and analyze it.
// buffer_receive : new data add into buffer.
// length           : length of new data, count by [byte]
void TranmittDataPointsAna(void *buffer_receive, uint32_t length)
{
	if (length + start_point < mybuffer_length)
	{
		memcpy(mybuffer + start_point, buffer_receive, length); // use memcpy for fast copy.
		start_point += (length);									// update
	}

	uint32_t flag;
	int ender_index_p;
	int ender_index_d;
	int index_p;
	int index_d;
	do
	{
		index_p = Find_Parameter_Header((void *)mybuffer, start_point, &ender_index_p);
		index_d = Find_Data_Header((void *)mybuffer, start_point, &ender_index_d);

		flag = (((uint32_t)(index_p != -1)) << 4) + (uint32_t)(index_d != -1);

		switch (flag)
		{
		case 0x00:
			start_point = 0;
			break;
		case 0x01:
			DataAna((void *)&mybuffer[index_d]);
			ClearFrameFromHeadTo(mybuffer, ender_index_d);
			start_point -= ender_index_d;
			break;
		case 0x10:
			ParameterAna((void *)&mybuffer[index_p]);
			ClearFrameFromHeadTo(mybuffer, ender_index_p);
			start_point -= ender_index_p;
			break;
		case 0x11:
		{
			if (index_p > index_d)
			{
				DataAna((void *)&mybuffer[index_d]);
				ClearFrameFromHeadTo(mybuffer, ender_index_d);
				start_point -= ender_index_d;
			}
			else // index_p < index_d
			{
				ParameterAna((void *)&mybuffer[index_p]);
				ClearFrameFromHeadTo(mybuffer, ender_index_p);
				start_point -= ender_index_p;
			}
		}
		break;
		}
	} while (flag != 0U);
}

void LoadStruct(ONE_PARAMETER_TO_SEND *frame, enum TypesOfFrame type, float value)
{
	frame->para_frame_header = 0x4455;
	frame->type_of_frame = type;
	frame->value_of_frame = value;
	frame->frame_tail = 0xAABB;
}
