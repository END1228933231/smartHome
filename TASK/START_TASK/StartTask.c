#include "main.h"
//������
TaskHandle_t StartTask_Handler;		//��ʼ����
TaskHandle_t LD3320Task_Handler;	//LD3320
TaskHandle_t AppTask_Handler;		//APP����
TaskHandle_t ControlTask_Handler;	//ʵ�ʲ�������
QueueHandle_t xHandleQueue = NULL;	//ʵ�ʲ������ƶ���
//��ֵ�ź������
SemaphoreHandle_t AppSemaphore;		//��ֵ�ź������

void Start_Task(void)
{
	//������ʼ����
	xTaskCreate((TaskFunction_t)start_task,          //������
				(const char *)"start_task",          //��������
				(uint16_t)START_STK_SIZE,            //�����ջ��С
				(void *)NULL,                        //���ݸ��������Ĳ���
				(UBaseType_t)START_TASK_PRIO,        //�������ȼ�
				(TaskHandle_t *)&StartTask_Handler); //������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); //�����ٽ���
	
	//������ֵ�ź���
	AppSemaphore = xSemaphoreCreateBinary();
	
	/* ������Ϣ���� */
	xHandleQueue = xQueueCreate(1,sizeof(u8));
	
	if(xHandleQueue == NULL)     /* �����Ϣ���д����ɹ� */
	{
		printf("Queue create fail!");
		while(1);
	}
	printf("Queue create success!\r\n");
	
	//����LD3320_Task����
    xTaskCreate((TaskFunction_t)LD3320_Task,
                (const char *)"LD3320_Task",
                (uint16_t)LD3320_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LD3320_TASK_PRIO,
                (TaskHandle_t *)&LD3320Task_Handler);
	//����App��������
    xTaskCreate((TaskFunction_t)App_Task,
                (const char *)"App_Task",
                (uint16_t)APP_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)APP_TASK_PRIO,
                (TaskHandle_t *)&AppTask_Handler);
	//����ʵ�ʿ�������
    xTaskCreate((TaskFunction_t)Control_Task,
                (const char *)"Control_Task",
                (uint16_t)CONTROL_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)CONTROL_TASK_PRIO,
                (TaskHandle_t *)&ControlTask_Handler);
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}
