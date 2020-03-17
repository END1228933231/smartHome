#ifndef LD3320_TASK_H
#define LD3320_TASK_H
//ʶ���루�ͻ��޸Ĵ�������Ϊ0
#define CODE_NAME	1	//��ˮ��
#define CODE_LSD	2	//��ˮ��
#define CODE_SS	  	3	//��˸
#define CODE_AJCF	4	//��������
#define CODE_QM	  	5	//ȫ��
#define CODE_JT		6  	//״̬
#define CODE_NH		7  	//���


/*======================�ⲿ�ӿ�======================*/
//������
void LD3320_Task(void *pvParameters);
//MP3����
void LD_Play_MP3(const char *path,const u8 Volume);

/*======================�ڲ�����======================*/
//�û��޸ĺ���
void LD3320_Running(void);
u8 LD_AsrAddFixed(void);

//�жϴ�����
void ProcessInt(void);

//�м��
u8 RunASR(void);
void LD_AsrStart(void);
u8 LD_Check_ASRBusyFlag_b2(void);

//�Ĵ�������
u8 spi_send_byte(u8 byte);
void LD_WriteReg(u8 add,u8 data);
u8 LD_ReadReg(u8 reg_add);
u8 LD_GetResult(void);
u8 LD_AsrRun(void);
void LD_Init_Common(void);
void LD_Init_ASR(void);

//MP3����
void LD_AdjustMIX2SPVolume(u8 val);
void LD_Init_MP3(void);
void LD_Play(void);
void Fill_the_Fifo(void);
void MP3_END_Reset(void);
void LD_ReloadMp3Data(void);
#endif
