#ifndef __APP_TASK_H
//������������õ�����ֵ�����ڲ���
#define LED1ON	51
#define LED1OFF	52
#define LED0ON	53
#define LED0OFF	54
#define COMMANDERR	0XFF
/*======================�ⲿ�ӿ�======================*/
void App_Task(void *pvParameters);

/*======================�ڲ�����======================*/
static void AppCotrolRunning(void);
static u8 CommandProcess(u8 *str);
static void LowerToCap(u8 *str,u8 len);
#endif
