#include "main.h"

/** 
* @project  ���ܼҾ�����ʶ��ϵͳ����λ����
* @brief    ����STM32F407ZG������ʶ��ģ��ΪLD3320
* @details  This is the detail description. 
* @author   ����
* @par 		qq_19257541(CSDN ID)
* @par 		page:https://me.csdn.net/qq_19257541
* @date     2020.3.9
* @version  V1.4 
*/ 
FATFS Fat_SD;//����SD

int main(void)
{ 
	System_Init();			//ϵͳ��ʼ��
	printf("\r\nSystem start...\r\n");
	LD_Play_MP3("SystemStart.mp3", 1);
	Start_Task();			//��ʼ����						
	vTaskStartScheduler();  //�����������
	
	while (1);
}

//ϵͳ��ʼ��
static void System_Init(void){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);	//����ϵͳ�ж����ȼ�����4
	delay_init(168);							   	//��ʼ����ʱ����
	uart_init(115200);     							//��ʼ������
	LED_Init();		        						//��ʼ��LED�˿�	       
	LD3320_init();									//LD3320ִ�к���
	delay_ms(100);
	
	printf("\r\n---\r\n");
	SD_Fatfs_Init();
}

static void SD_Fatfs_Init(void)
{
	FIL fil;
	char line[100];
	while(SD_Init())
	{
		//��ⲻ��SD��
		printf("SD Card Error!\r\n");
		delay_ms(500);					
		printf("Please Check! \r\n");
		delay_ms(500);
		LED0=!LED0;//DS0��˸
	}
	
	//���SD���ɹ� 
	printf("SD Card OK\r\n");
	
  	f_mount(&Fat_SD,"0:",1); 					//����SD��
	f_open(&fil, "first.txt", FA_READ);
	while (f_gets(line, sizeof line, &fil)) {
        printf(line);
    }
	f_close(&fil);
}
