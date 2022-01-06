/**
 * @file      myLWIPAPP.c
 * @brief     
 * @details   
 * @author   Foubingjian
 * @date      
 * @version   
 * @par       
 * Using encoding: GB2312
 */
 
#include "myLWIPAPP.h"
 
struct tcp_pcb* tcp_Echoserver_PCB = NULL;


uint8_t tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];

instructmentStruct dataSend;


//TCP Server ����ȫ��״̬��Ǳ���
//bit7:0,û������Ҫ����;1,������Ҫ����
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û�пͻ���������;1,�пͻ�����������.
//bit4~0:����
uint8_t tcp_server_flag = 0;
void* tcp_server_sendbuf= NULL;

//LWIP tcp_server_recv�ص�����
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	err_t ret_err;
	uint32_t data_len = 0;
	struct pbuf *q;
  	struct tcp_server_struct *es;
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_server_struct *)arg;
	if(p==NULL) //�ӿͻ��˽��յ�������
	{
//		es->state=ES_TCPSERVER_CLOSING;//��Ҫ�ر�TCP ������
		es->p=p; 
		ret_err=ERR_BUF;
	}else if(err!=ERR_OK)	//�ӿͻ��˽��յ�һ���ǿ�����,��������ĳ��ԭ��err!=ERR_OK
	{
		if(p)pbuf_free(p);	//�ͷŽ���pbuf
		ret_err=err;
	}else if(es->state==ES_TCPSERVER_ACCEPTED) 	//��������״̬
	{
		if(p!=NULL)  //����������״̬���ҽ��յ������ݲ�Ϊ��ʱ�����ӡ����
		{
			memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
			for(q=p;q!=NULL;q=q->next)  //����������pbuf����
			{
				//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
				//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
				if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
				else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
			}
			tcp_server_flag |= 1U<<6;
 			tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
			pbuf_free(p);  	//�ͷ��ڴ�
			ret_err=ERR_OK;
			TranmittDataPointsAna((uint16_t*)tcp_client_recvbuf, data_len);
		}
	}else//�������ر���
	{
		tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
		es->p=NULL;
		pbuf_free(p); //�ͷ��ڴ�
		ret_err=ERR_OK;
	}
	return ret_err;
}

//lwIP tcp_err�����Ļص�����
void tcp_server_error(void *arg,err_t err)
{  
	char txt[50];
	LWIP_UNUSED_ARG(err);  
	sprintf(txt,"tcp err:%x\r\n",(uint32_t)arg);
	LCD_ShowString(30, 420, 200, 16, 16, txt);
	if(arg!=NULL)mem_free(arg);//�ͷ��ڴ�
} 
//lwIP tcp_poll�Ļص�����
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_server_struct *es; 
	es=(struct tcp_server_struct *)arg; 
	if(es!=NULL)
	{
		if(tcp_server_flag&(1<<7)&&dataSend.len>0)	//�ж��Ƿ�������Ҫ����
		{
			es->p=pbuf_alloc(PBUF_TRANSPORT, (dataSend.len)*sizeof(uint16_t),PBUF_POOL);
			pbuf_take(es->p,dataSend.instrumentData,(dataSend.len)*sizeof(uint16_t));
			tcp_server_senddata(tpcb,es); 		//��ѯ��ʱ����Ҫ���͵�����
			tcp_server_flag&=~(1<<7);  			//������ݷ��ͱ�־λ
			dataSend.len = 0;
			if(es->p!=NULL)pbuf_free(es->p); 	//�ͷ��ڴ�	
		}else if(es->state==ES_TCPSERVER_CLOSING)//��Ҫ�ر�����?ִ�йرղ���
		{
			tcp_server_connection_close(tpcb,es);//�ر�����
		}
		ret_err=ERR_OK;
	}else
	{
		tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
		ret_err=ERR_ABRT; 
	}
	return ret_err;
} 
//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)
//err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
//{
//	struct tcp_server_struct *es;
//	LWIP_UNUSED_ARG(len); 
//	es = (struct tcp_server_struct *) arg;
//	if(es->p)tcp_server_senddata(tpcb,es);//��������
//	return ERR_OK;
//} 
//�˺���������������
void tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	struct pbuf *ptr;
	uint16_t plen;
	err_t wr_err=ERR_OK;
	 while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb)))
	 {
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,2); //��Ҫ���͵����ݼ��뷢�ͻ��������
		if(wr_err==ERR_OK)
		{ 
			plen=ptr->len;
			es->p=ptr->next;			//ָ����һ��pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf��ref��һ
			pbuf_free(ptr);
			tcp_recved(tpcb,plen); 		//����tcp���ڴ�С
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);   //�����ͻ�������е����ݷ��ͳ�ȥ
	 }
} 

err_t tcp_Echoserver_Accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	err_t ret_err;
	struct tcp_server_struct *es; 
 	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	tcp_setprio(newpcb,TCP_PRIO_MIN);//�����´�����pcb���ȼ�
	es=(struct tcp_server_struct*)mem_malloc(sizeof(struct tcp_server_struct)); //�����ڴ�
 	if(es!=NULL) //�ڴ����ɹ�
	{
		es->state=ES_TCPSERVER_ACCEPTED;  	//��������
		es->pcb=newpcb;
		es->p=NULL;
		
		tcp_arg(newpcb,es);
		tcp_recv(newpcb,tcp_server_recv);	//��ʼ��tcp_recv()�Ļص�����
		tcp_err(newpcb,tcp_server_error); 	//��ʼ��tcp_err()�ص�����
		tcp_poll(newpcb,tcp_server_poll,1);	//��ʼ��tcp_poll�ص�����
//		tcp_sent(newpcb,tcp_server_sent);  	//��ʼ�����ͻص�����
		tcp_server_flag|=1<<5;				//����пͻ���������
		
		ret_err=ERR_OK;
	}else ret_err=ERR_MEM;
	return ret_err;
}

//�ر�tcp����
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	tcp_close(tpcb);
	tcp_arg(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_recv(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);
	if(es)mem_free(es); 
	tcp_server_flag&=~(1<<5);//������ӶϿ���
}

void tcp_server_close(struct tcp_pcb *tpcb)
{
	tcp_close(tpcb);
	tcp_arg(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_recv(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);
	tcp_server_flag&=~(1<<5);//������ӶϿ���
}

err_t tcp_Connect(void)
{
	err_t err;
	tcp_Echoserver_PCB = tcp_new();
	if(tcp_Echoserver_PCB != NULL)
	{
		/* ��tcp_Echo�󶨵��˿�8088 */
		err = tcp_bind(tcp_Echoserver_PCB,IP_ADDR_ANY,8088);
		if(err == ERR_OK)
		{
			/* ��ʼ����tcp_Echo */
			tcp_Echoserver_PCB = tcp_listen(tcp_Echoserver_PCB);
			
			/* ע��LWIP tcp_Echo�ص����� */
			tcp_accept(tcp_Echoserver_PCB, tcp_Echoserver_Accept);
		}
		else
		{
			/* �ͷ�pcb */
			memp_free(MEMP_TCP_PCB, tcp_Echoserver_PCB);
		}
	}
	else
	{
		err = ERR_BUF;
	}
	return err;
}

void ETH_SendData(int16_t dataNum, void* data, uint16_t description)
{
	if(tcp_server_flag & (1<<5))
	{
		dataSend.description = description;
		dataSend.len = dataNum;
		dataSend.instrumentData = data;
		tcp_server_flag|=1<<7;
	}
	else
	{
		return;
	}
	return;
}

//void ETH_SendWaveData(int16_t dataNum, void* data)
//{
//	
//}

//end
