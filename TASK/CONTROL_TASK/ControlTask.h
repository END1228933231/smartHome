#ifndef __CONTROL_TASK_H
#define __CONTROL_TASK_H
/*======================�ⲿ�ӿ�======================*/
void Control_Task(void *pvParameters);
u8 get_ASR_Len(void);

/*======================�ڲ�����======================*/
static void Board_text(u8 Code_Val);
//����ִ�в�������
static void Glide_LED(void);
static void Flicker_LED(void);
static void Key_LED(void);
static void Off_LED(void);
static void Jt_LED(void);
#endif
