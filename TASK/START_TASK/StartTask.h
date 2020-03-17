#ifndef __STARTTASK_H
#define __STARTTASK_H

void Start_Task(void);

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
void start_task(void *pvParameters);

//�������ȼ�
#define LD3320_TASK_PRIO	3
//�����ջ��С	
#define LD3320_STK_SIZE 	512  


//�������ȼ�
#define APP_TASK_PRIO		4
//�����ջ��С	
#define APP_STK_SIZE 		128


//�������ȼ�
#define CONTROL_TASK_PRIO		8
//�����ջ��С	
#define CONTROL_STK_SIZE 		512

extern QueueHandle_t xHandleQueue;
extern SemaphoreHandle_t AppSemaphore;


#endif
