#include "main.h"

#define APP_TASK_INIT_TIME 300
#define TASK_DELAY_MS 200
#define TICK_WAIT     50
//�û��޸����� - ��ӡ��Ϣ
static const u8 ASR_LEN = 7;    //ָ����Ŀ��printfCmd�����MP3Name����һά��ֵ-1
const char* printfCmd[] = {
	"No command\r\n",
	"system name\r\n",
	"Running water light success\r\n",
	"Light flashing success\r\n",
	"button success\r\n",
	"all off success\r\n",
	"state success\r\n",
	"hello success\r\n"
};

const char* MP3Name[] = {
	"tingBuQing.mp3",
	"zhuRenWoZai.mp3",
	"liuShuiDeng.mp3",
	"ShanShuo.mp3",
	"",
	"",
	"",
	""
};

static u8 preResVal = 0;

void Control_Task(void *pvParameters)
{
	u8 resVal;
	
	portBASE_TYPE xStatus;

	printf("Control task start\r\n");
	
	while(1)
	{
		//�ȴ���Ϣ �ȴ�TICK_WAIT������(ms)
		xStatus = xQueueReceive( xHandleQueue, &resVal, (TickType_t)TICK_WAIT ); 
		
		if(xStatus==pdPASS)                                                      //����ɹ����մ���Ϣ
		{
			//��ӡ��Ϣ����
			if(resVal != 0)
			{
				//ASR�����������50����51��ʼΪAPP���в���ָ��
				if(resVal <= ASR_LEN && resVal <= 50 )
				{
					printf(printfCmd[resVal]);
					LD_Play_MP3(MP3Name[resVal], 1);
				}
				
				preResVal = resVal;
			}
			Board_text(resVal);
		}
		else                                                                     //���û�н��յ���Ϣ
		{
			if(preResVal != 0)
			{
				Board_text(preResVal);
				vTaskDelay(10);
			}
		}
	}	
	
}

u8 get_ASR_Len(void)	//��ȡָ������
{
	return ASR_LEN;
}

static void Board_text(u8 Code_Val)
{
	static u8 preCodeVal = 0;
	switch(Code_Val)  //�Խ��ִ����ز���
	{
		case CODE_NAME:
			break;
		case CODE_LSD:  //�����ˮ�ơ�
			Glide_LED();
			break;
		case CODE_SS:	  //�����˸��
			Flicker_LED();
			break;
		case CODE_AJCF:	//�������������
			Key_LED();
			break;
		case CODE_QM:		//���ȫ��
			Off_LED();
			break;
		case CODE_JT:		//���״̬��
			Jt_LED();
			break;
		case CODE_NH:		//�����á�
			Flicker_LED();
			break;
		case LED1ON: 
			if(LED1 != ON)
			{
				LED1=ON;
			}
			break;
		case LED1OFF:
			if(LED1 != OFF)
			{
				LED1=OFF;
			}
			break;
		case LED0ON:
			if(LED0 != ON)
			{
				LED0=ON;
			}
			break;
		case LED0OFF:
			if(LED0 != OFF)
			{
				LED0=OFF;
			}
			break;
		
		default:	
			LD_Play_MP3(MP3Name[0], 1);
			preResVal = preCodeVal;	//������һ������״̬
			return;
	}	
	preCodeVal = Code_Val;	//������һ������״̬
}

static void Glide_LED(void)
{
	LED0=ON;			
	LED1=OFF;			
	delay_ms(500);
	LED0=OFF;			
	LED1=ON;			
	delay_ms(500);
}

static void Flicker_LED(void)
{
	LED0=ON;			
	LED1=ON;			
	delay_ms(500);
	LED0=OFF;			
	LED1=OFF;			
	delay_ms(1500);
}

static void Key_LED(void)
{
	LED0=ON;		
	delay_ms(500);
	LED0=OFF;			
	delay_ms(500);
}

static void Off_LED(void)
{
	LED1=OFF;
	LED0=OFF;
}

static void Jt_LED(void)
{
	LED1=ON;		
	delay_ms(500);
	LED1=OFF;		
	delay_ms(500);
}
