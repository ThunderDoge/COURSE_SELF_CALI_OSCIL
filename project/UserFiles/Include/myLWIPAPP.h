/**
 * @file      myLWIPAPP.c.h
 * @brief     
 * @details   
 * @author   Foubingjian
 * @date      
 * @version   
 * @par       
 * Using encoding: GB2312
 */

#include "ILI93xx.h"
#include "cmsis_os.h"
#include "adc_control.h"
#include "stdio.h"
#include "tcp.h"
#include "string.h"
#include "tansmitt.h"
 
#ifndef _MYLWIPAPP_H
#define _MYLWIPAPP_H
 
#define TCP_CLIENT_RX_BUFSIZE 32

typedef struct{
	uint8_t tcp_Connect_Status;
	uint8_t tcp_Connect_CMD;
	uint16_t* sendData;
	uint16_t* recvData;
	uint8_t upper_Host_cmd;
	uint8_t upper_Host_cmd_data;
	float caliParamater;
}	myAPPStatus;

 //tcp����������״̬
enum tcp_server_states
{
	ES_TCPSERVER_NONE = 0,		//û������
	ES_TCPSERVER_ACCEPTED,		//�пͻ����������� 
	ES_TCPSERVER_CLOSING,		//�����ر�����
}; 
//LWIP�ص�����ʹ�õĽṹ��
struct tcp_server_struct
{
	uint8_t state;               //��ǰ����״
	struct tcp_pcb *pcb;    //ָ��ǰ��pcb
	struct pbuf *p;         //ָ�����/�����pbuf
}; 
//������Ϣ�ṹ�壬���ȡ�����������
typedef struct instructmentStruct{
	uint16_t len;
	uint16_t description;
	uint16_t* instrumentData;
}instructmentStruct;

extern uint8_t tcp_server_flag;
extern uint8_t tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];
extern struct tcp_pcb *tcp_Echoserver_PCB;

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void tcp_server_error(void *arg,err_t err);
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
void tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct *es);
err_t tcp_Echoserver_Accept(void *arg, struct tcp_pcb *newpcb, err_t err);
void ETH_SendData(int16_t dataNum, void* data, uint16_t description);
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es);
void tcp_server_close(struct tcp_pcb *tpcb);

int ETH_isReadyToSendData(void);


 
#endif
 
//end
