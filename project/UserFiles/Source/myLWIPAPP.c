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


//TCP Server 测试全局状态标记变量
//bit7:0,没有数据要发送;1,有数据要发送
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有客户端连接上;1,有客户端连接上了.
//bit4~0:保留
uint8_t tcp_server_flag = 0;
void* tcp_server_sendbuf= NULL;

//LWIP tcp_server_recv回调函数
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	err_t ret_err;
	uint32_t data_len = 0;
	struct pbuf *q;
  	struct tcp_server_struct *es;
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_server_struct *)arg;
	if(p==NULL) //从客户端接收到空数据
	{
//		es->state=ES_TCPSERVER_CLOSING;//需要关闭TCP 连接了
		es->p=p; 
		ret_err=ERR_BUF;
	}else if(err!=ERR_OK)	//从客户端接收到一个非空数据,但是由于某种原因err!=ERR_OK
	{
		if(p)pbuf_free(p);	//释放接收pbuf
		ret_err=err;
	}else if(es->state==ES_TCPSERVER_ACCEPTED) 	//处于连接状态
	{
		if(p!=NULL)  //当处于连接状态并且接收到的数据不为空时将其打印出来
		{
			memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //数据接收缓冲区清零
			for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
			{
				//判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
				//的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
				if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//拷贝数据
				else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
			}
			tcp_server_flag |= 1U<<6;
 			tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
			pbuf_free(p);  	//释放内存
			ret_err=ERR_OK;
			TranmittDataPointsAna((uint16_t*)tcp_client_recvbuf, data_len);
		}
	}else//服务器关闭了
	{
		tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
		es->p=NULL;
		pbuf_free(p); //释放内存
		ret_err=ERR_OK;
	}
	return ret_err;
}

//lwIP tcp_err函数的回调函数
void tcp_server_error(void *arg,err_t err)
{  
	char txt[50];
	LWIP_UNUSED_ARG(err);  
	sprintf(txt,"tcp err:%x\r\n",(uint32_t)arg);
	LCD_ShowString(30, 420, 200, 16, 16, txt);
	if(arg!=NULL)mem_free(arg);//释放内存
} 
//lwIP tcp_poll的回调函数
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_server_struct *es; 
	es=(struct tcp_server_struct *)arg; 
	if(es!=NULL)
	{
		if(tcp_server_flag&(1<<7)&&dataSend.len>0)	//判断是否有数据要发送
		{
			es->p=pbuf_alloc(PBUF_TRANSPORT, (dataSend.len)*sizeof(uint16_t),PBUF_POOL);
			pbuf_take(es->p,dataSend.instrumentData,(dataSend.len)*sizeof(uint16_t));
			tcp_server_senddata(tpcb,es); 		//轮询的时候发送要发送的数据
			tcp_server_flag&=~(1<<7);  			//清除数据发送标志位
			dataSend.len = 0;
			if(es->p!=NULL)pbuf_free(es->p); 	//释放内存	
		}else if(es->state==ES_TCPSERVER_CLOSING)//需要关闭连接?执行关闭操作
		{
			tcp_server_connection_close(tpcb,es);//关闭连接
		}
		ret_err=ERR_OK;
	}else
	{
		tcp_abort(tpcb);//终止连接,删除pcb控制块
		ret_err=ERR_ABRT; 
	}
	return ret_err;
} 
//lwIP tcp_sent的回调函数(当从远端主机接收到ACK信号后发送数据)
//err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
//{
//	struct tcp_server_struct *es;
//	LWIP_UNUSED_ARG(len); 
//	es = (struct tcp_server_struct *) arg;
//	if(es->p)tcp_server_senddata(tpcb,es);//发送数据
//	return ERR_OK;
//} 
//此函数用来发送数据
void tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	struct pbuf *ptr;
	uint16_t plen;
	err_t wr_err=ERR_OK;
	 while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb)))
	 {
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,2); //将要发送的数据加入发送缓冲队列中
		if(wr_err==ERR_OK)
		{ 
			plen=ptr->len;
			es->p=ptr->next;			//指向下一个pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf的ref加一
			pbuf_free(ptr);
			tcp_recved(tpcb,plen); 		//更新tcp窗口大小
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);   //将发送缓冲队列中的数据发送出去
	 }
} 

err_t tcp_Echoserver_Accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	err_t ret_err;
	struct tcp_server_struct *es; 
 	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	tcp_setprio(newpcb,TCP_PRIO_MIN);//设置新创建的pcb优先级
	es=(struct tcp_server_struct*)mem_malloc(sizeof(struct tcp_server_struct)); //分配内存
 	if(es!=NULL) //内存分配成功
	{
		es->state=ES_TCPSERVER_ACCEPTED;  	//接收连接
		es->pcb=newpcb;
		es->p=NULL;
		
		tcp_arg(newpcb,es);
		tcp_recv(newpcb,tcp_server_recv);	//初始化tcp_recv()的回调函数
		tcp_err(newpcb,tcp_server_error); 	//初始化tcp_err()回调函数
		tcp_poll(newpcb,tcp_server_poll,1);	//初始化tcp_poll回调函数
//		tcp_sent(newpcb,tcp_server_sent);  	//初始化发送回调函数
		tcp_server_flag|=1<<5;				//标记有客户端连上了
		
		ret_err=ERR_OK;
	}else ret_err=ERR_MEM;
	return ret_err;
}

//关闭tcp连接
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	tcp_close(tpcb);
	tcp_arg(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_recv(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);
	if(es)mem_free(es); 
	tcp_server_flag&=~(1<<5);//标记连接断开了
}

void tcp_server_close(struct tcp_pcb *tpcb)
{
	tcp_close(tpcb);
	tcp_arg(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_recv(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);
	tcp_server_flag&=~(1<<5);//标记连接断开了
}

err_t tcp_Connect(void)
{
	err_t err;
	tcp_Echoserver_PCB = tcp_new();
	if(tcp_Echoserver_PCB != NULL)
	{
		/* 将tcp_Echo绑定到端口8088 */
		err = tcp_bind(tcp_Echoserver_PCB,IP_ADDR_ANY,8088);
		if(err == ERR_OK)
		{
			/* 开始监听tcp_Echo */
			tcp_Echoserver_PCB = tcp_listen(tcp_Echoserver_PCB);
			
			/* 注册LWIP tcp_Echo回调函数 */
			tcp_accept(tcp_Echoserver_PCB, tcp_Echoserver_Accept);
		}
		else
		{
			/* 释放pcb */
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
