/**
 * @file      transmit_pack.h
 * @brief     definition of structure for transmit
 * @details   
 * @author   ThunderDoge
 * @date      
 * @version   
 * @par       
 * Using encoding: utf-8
 */

#ifndef __TRANSMIT_STRUCT
#define __TRANSMIT_STRUCT

Struct __packed
{
	unsigned int frame_header = 0x4455;//帧头
	
	float Voltage_Max;//电压最大值
	float Voltage_Min;//电压最小值
	float RMS;//电压有效值
	float Frequence;//频率
	
	float bias_compensation; //偏置补偿 (V，下同)
	float gain_compensation_1V; //1V增益补偿
	float gain_compensation_2V; //2V增益补偿
	float gain_compensation_5V; //5V增益补偿
	float gain_compensation_10V; //10V增益补偿
	
	int start_auto_calibration_flag; //该变量为1，代表开始所有挡位的自动校准开始
	int start_1V_flag; //该变量为1，代表开始1V档的校准
	int start_2V_flag; //该变量为1，代表开始2V档的校准 
	int start_5V_flag; //该变量为1，代表开始5V档的校准 
	int start_10V_flag; //该变量为1，代表开始10V档的校准
	
	unsigned int frame_tail = 0xAABB;//帧尾 
}ParameterToSend



#endif // !__TRANSMIT_STRUCT

