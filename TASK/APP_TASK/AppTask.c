#include "main.h"
#include "string.h"
#include "malloc.h"
#define APP_TASK_INIT_TIME 300
#define TASK_DELAY_MS 200
void AppCommandRunning(void);
void App_Task(void *pvParameters)
{
	BaseType_t err=pdFALSE;
	
	vTaskDelay(APP_TASK_INIT_TIME);
	printf("App task start\r\n");
	
	while(1)
	{
		if(AppSemaphore!=NULL)
		{
			err=xSemaphoreTake(AppSemaphore,portMAX_DELAY);	//��ȡ�ź���
			if(err==pdTRUE)										//��ȡ�ź����ɹ�
			{
				//AppCotrolRunning();	
				AppCommandRunning();
			}
		}
		else if(err==pdFALSE)
		{
			vTaskDelay(10);      //��ʱ10ms��Ҳ����10��ʱ�ӽ���	
		}
	}
	
}

void AppCommandRunning(void)
{
	u8 len=0;
	u16 i;
	u8 cmd;
	portBASE_TYPE xStatus;
	
	len=USART_RX_STA&0x3fff;				//�õ��˴ν��յ������ݳ���
	for(i = 0; i < len; i++)
	{
		if(USART_RX_BUF[i] == 0xA1)
		{
			cmd = USART_RX_BUF[i+1];
			
			//��������ȴ���Ϣ���е���Ϣ
			xStatus = xQueueOverwrite(xHandleQueue, &cmd);
			
			//���������Ϣʧ��
			if( xStatus != pdPASS )                                        
			{
			  printf("Data can't to send to the queue.\r\n");
			}
			break;
		}
		
	}
	USART_RX_STA=0;
}

static void AppCotrolRunning(void)
{
	u8 len=0;
	u8 CommandValue=COMMANDERR;
	u8 *CommandStr;
	portBASE_TYPE xStatus;
	
	len=USART_RX_STA&0x3fff;				//�õ��˴ν��յ������ݳ���
	CommandStr=mymalloc(SRAMIN,len+1);		//�����ڴ�
	sprintf((char*)CommandStr,"%s",USART_RX_BUF);
	CommandStr[len]='\0';					//�����ַ�����β����
	LowerToCap(CommandStr,len);				//���ַ���ת��Ϊ��д		
	CommandValue=CommandProcess(CommandStr);//�������
	if(CommandValue!=COMMANDERR)
	{
		if(CommandValue > 50)
		{
			printf("Command:%s\r\n",CommandStr);
		}
		//��������ȴ���Ϣ���е���Ϣ
		xStatus = xQueueOverwrite(xHandleQueue, &CommandValue);
		
		//���������Ϣʧ��
		if( xStatus != pdPASS )                                        
		{
		  printf("Data can't to send to the queue.\r\n");
		}
	}
	else
	{
		printf("Error command!!\r\n");
	}
	USART_RX_STA=0;
	memset(USART_RX_BUF,0,USART_REC_LEN);	//���ڽ��ջ���������
	myfree(SRAMIN,CommandStr);				//�ͷ��ڴ�
	
}

//������������ַ�������ת��������ֵ
//str������
//����ֵ: 0XFF�������������ֵ������ֵ
static u8 CommandProcess(u8 *str)
{
	u8 CommandValue=COMMANDERR;
	if(strcmp((char*)str,"LED1ON")==0) CommandValue=LED1ON;
	else if(strcmp((char*)str,"LED1OFF")==0) CommandValue=LED1OFF;
	else if(strcmp((char*)str,"LED0ON")==0) CommandValue=LED0ON;
	else if(strcmp((char*)str,"LED0OFF")==0) CommandValue=LED0OFF;
	return CommandValue;
}

//���ַ����е�Сд��ĸת��Ϊ��д
//str:Ҫת�����ַ���
//len���ַ�������
static void LowerToCap(u8 *str,u8 len)
{
	u8 i;
	for(i=0;i<len;i++)
	{
		if((96<str[i])&&(str[i]<123))	//Сд��ĸ
		str[i]=str[i]-32;				//ת��Ϊ��д
	}
}




