#include "main.h"
#include "string.h"
#define LD3320_TASK_INIT_TIME 200
#define TASK_DELAY_MS 100
/************************************************************************************
//	nAsrStatus ������main�������б�ʾ�������е�״̬������LD3320оƬ�ڲ���״̬�Ĵ���
//	LD_ASR_NONE:			��ʾû������ASRʶ��
//	LD_ASR_RUNING��		��ʾLD3320������ASRʶ����
//	LD_ASR_FOUNDOK:		��ʾһ��ʶ�����̽�������һ��ʶ����
//	LD_ASR_FOUNDZERO:	��ʾһ��ʶ�����̽�����û��ʶ����
//	LD_ASR_ERROR:			��ʾһ��ʶ��������LD3320оƬ�ڲ����ֲ���ȷ��״̬
*********************************************************************************/
static u8 nAsrStatus = 0;
static u8 nLD_Mode = LD_MODE_IDLE;//������¼��ǰ���ڽ���ASRʶ�����ڲ���MP3
static u8 ucRegVal;
static u8 nAsrRes;
FIL fsrc;//�ļ�����
DWORD nMp3Size=0;;//MP3�Ĵ�С
DWORD nMp3Pos; //MP3���ŵĳ���
u8 bMp3Play=0;//�Ƿ������MP3����
u8 ucSPVol=15; // MAX=15 MIN=0		//	Speaker�������������

void LD3320_Task(void *pvParameters)
{
	//��ȡ��������ʱ����Ĳ���	
	vTaskDelay(LD3320_TASK_INIT_TIME);
	printf("LD3320 task start\r\n");
	nAsrRes=0;
	while(1)
	{
		LD3320_Running();
		//ϵͳ��ʱ
		vTaskDelay(TASK_DELAY_MS);
	}
	
}

//�û��޸�
void LD3320_Running(void)
{
	static u8 isAwake = 0;
	portBASE_TYPE xStatus;
	switch(nAsrStatus)
	{
		case LD_ASR_RUNING:
		case LD_ASR_ERROR:
		case LD_MP3_PLAY:
				 break;
		case LD_ASR_NONE:
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)//����һ��ASRʶ�����̣�ASR��ʼ����ASR��ӹؼ��������ASR����
				{		
					nAsrStatus = LD_ASR_ERROR;   //ASR���̴���
					printf("ASR error\r\n");
				}
				else
				{
					//printf("ASR success\r\n");
				}
				break;
		case LD_ASR_FOUNDOK:
				nAsrRes = LD_GetResult( );//һ��ASRʶ�����̽�����ȥȡASRʶ����										 
				printf("id code:%d, ", nAsrRes);
				if(nAsrRes == CODE_NAME)
				{
					printf("hear my name\r\n");
					LD_Play_MP3("zhuRenWoZai.mp3", 1);
					isAwake = 1;
				}
				else
				{
					if(!isAwake)	//û�л���
					{
						printf("I'm sleep\r\n");
						nAsrStatus = LD_ASR_NONE;
						return;
					}
					if(nAsrRes <= get_ASR_Len())	//�ж��Ƿ���ָ�Χ�ڣ���
					{
						printf("I'm reset\r\n");
						isAwake = 0;	//����״̬
					}
					//��������ȴ���Ϣ���е���Ϣ
					xStatus = xQueueOverwrite(xHandleQueue, &nAsrRes);
					
					//���������Ϣʧ��
					if( xStatus != pdPASS )                                        
					{
					  printf("Data can't to send to the queue.\r\n");
					}		
				}
				
				nAsrStatus = LD_MP3_PLAY;			
				break;
		case LD_ASR_FOUNDZERO:
		default:
			nAsrStatus = LD_ASR_NONE;
			break;
		}//switch
	
		
	
}

//�жϴ�����
void ProcessInt(void)
{
	u8 nAsrResCount=0;
	u8 ucHighInt;
	u8 ucLowInt;
	ucRegVal = LD_ReadReg(0x2B);

	if(nLD_Mode == LD_MODE_ASR_RUN&&(!bMp3Play))
	{
		// ����ʶ��������ж�
		//�����������룬����ʶ��ɹ���ʧ�ܶ����жϣ�
		LD_WriteReg(0x29,0) ;
		LD_WriteReg(0x02,0) ;

		if((ucRegVal & 0x10) && LD_ReadReg(0xb2)==0x21 && LD_ReadReg(0xbf)==0x35)		
		{	 
				nAsrResCount = LD_ReadReg(0xba);

				if(nAsrResCount>0 && nAsrResCount<=4) 
				{
					nAsrStatus=LD_ASR_FOUNDOK; 				
				}
				else
				{
					nAsrStatus=LD_ASR_FOUNDZERO;
				}	
		}
		else
		{
			nAsrStatus=LD_ASR_FOUNDZERO;//ִ��û��ʶ��
		}

		LD_WriteReg(0x2b,0);
		LD_WriteReg(0x1C,0);//д0:ADC������
		return;
	}
	
	//MP3����ģʽ
	ucHighInt=LD_ReadReg(0x29);
	ucLowInt=LD_ReadReg(0x02);
	LD_WriteReg(0x29,0) ;		//�ж����� FIFO �ж����� 0��ʾ������ 
	LD_WriteReg(0x02,0) ;		// FIFO�ж�����	 FIFO_DATA FIFO_EXT�ж�  ������
	//�����������
	if(LD_ReadReg(0xBA)&CAUSE_MP3_SONG_END)
	{
		LD_WriteReg(0x2B, 0); 
		LD_WriteReg(0xBA,0x00);
		LD_WriteReg(0xBC,0x00);
		LD_WriteReg(0x08,0x01); 
		LD_WriteReg(0x08,0x00); 
		LD_WriteReg(0x33,0x00); 
		bMp3Play=0;//��־���Ž���
		MP3_END_Reset();
		return ;
	}
	//���������ѷ�����ϡ�
	if(nMp3Pos>=nMp3Size)
	{ 
		LD_WriteReg(0xBC, 0x01); 
		LD_WriteReg(0x29, 0x10);
		bMp3Play=0;//��־���Ž���
		MP3_END_Reset();
		return; 
	}
	//����������ʱ��Ҫ���꣬��Ҫ�����µ����ݡ�	
	LD_ReloadMp3Data();		//����MP3
	LD_WriteReg(0x29,ucHighInt);
	LD_WriteReg(0x02,ucLowInt) ; 			
	nLD_Mode = LD_MODE_ASR_RUN;//�ص�����ʶ��״̬
	nAsrStatus = LD_ASR_NONE;
}

//��ӹؼ���
u8 LD_AsrAddFixed(void)
{
	u8 k, flag;
	u8 nAsrAddLength;
	#define DATE_A 7    //�����ά��ֵ
	#define DATE_B 20	//����һά��ֵ
	//��ӹؼ��ʣ��û��޸�
	u8  sRecog[DATE_A][DATE_B] = {
		    "xiao peng you",\
			"liu shui deng",\
			"shan shuo",\
			"an jian chu fa",\
			"quan mie",\
			"zhuang tai",\
			"ni hao"\
	};	
	u8  pCode[DATE_A] = {
		CODE_NAME,  \
		CODE_LSD,	\
		CODE_SS,	\
		CODE_AJCF,  \
		CODE_QM,	\
		CODE_JT	,	\
		CODE_NH\
	};	//���ʶ���룬�û��޸�
	flag = 1;
	for (k=0; k<DATE_A; k++)
	{			
		if(LD_Check_ASRBusyFlag_b2() == 0)
		{
			flag = 0;
			printf("LD_Check_ASRBusyFlag_b2 is error\r\n");
			break;
		}

		LD_WriteReg(0xc1, pCode[k] );
		LD_WriteReg(0xc3, 0);
		LD_WriteReg(0x08, 0x04);
		LD3320_delay(1);
		LD_WriteReg(0x08, 0x00);
		LD3320_delay(1);

		for (nAsrAddLength=0; nAsrAddLength<DATE_B; nAsrAddLength++)
		{
			if (sRecog[k][nAsrAddLength] == 0)
				break;
			LD_WriteReg(0x5, sRecog[k][nAsrAddLength]);
		}
		LD_WriteReg(0xb9, nAsrAddLength);
		LD_WriteReg(0xb2, 0xff);
		LD_WriteReg(0x37, 0x04);
	}	 
	return flag;
}

//�û��޸� end

//�м��
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line12)!= RESET ) 
	{
		printf("enter IRQ 12\r\n");
		ProcessInt(); 
 		printf("out IRQ 12\r\n");	
		EXTI_ClearFlag(EXTI_Line12);
		EXTI_ClearITPendingBit(EXTI_Line12);//���LINE�ϵ��жϱ�־λ  
	} 
}

u8 RunASR(void)
{
	u8 i=0;
	u8 asrflag=0;
	for (i=0; i<5; i++)		//��ֹ����Ӳ��ԭ����LD3320оƬ����������������һ������5������ASRʶ������
	{
		LD_AsrStart();			//��ʼ��ASR
		LD3320_delay(100);
		if (LD_AsrAddFixed()==0)	//��ӹؼ����ﵽLD3320оƬ��
		{
			printf("LD_AsrAddFixed is error\r\n");
			LD3320_reset();				//LD3320оƬ�ڲ����ֲ���������������LD3320оƬ
			LD3320_delay(50);	//���ӳ�ʼ����ʼ����ASRʶ������
			continue;
		}
		LD3320_delay(10);
		if (LD_AsrRun() == 0)
		{
			printf("LD_AsrRun is error\r\n");
			LD3320_reset();			 //LD3320оƬ�ڲ����ֲ���������������LD3320оƬ
			LD3320_delay(50);//���ӳ�ʼ����ʼ����ASRʶ������
			continue;
		}
		asrflag=1;
		break;						//ASR���������ɹ����˳���ǰforѭ������ʼ�ȴ�LD3320�ͳ����ж��ź�
	}	
	return asrflag;
}


void LD_AsrStart(void)
{
	LD_Init_ASR();
}

u8 LD_Check_ASRBusyFlag_b2(void)
{
	u8 j;
	u8 flag = 0;
	for (j=0; j<10; j++)
	{
		if (LD_ReadReg(0xb2) == 0x21)
		{
			flag = 1;
			break;
		}
//		printf("reg(0xb2) data is %x\r\n",LD_ReadReg(0xb2));
		LD3320_delay(10);		
	}
	return flag;
}
//�м�� end

//�Ĵ�������
u8 spi_send_byte(u8 byte)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1,byte);
	while (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}

void LD_WriteReg(u8 add,u8 data)
{
	LD3320_CS=0;
	__nop();
	__nop();
	__nop();
	LD3320_WR=0;
	spi_send_byte(0x04);
	spi_send_byte(add);
	spi_send_byte(data);
	LD3320_CS=1;
}

u8 LD_ReadReg(u8 reg_add)
{
	u8 i;
	LD3320_CS=0;
	__nop();
	__nop();
	__nop();
	LD3320_WR=0;
	spi_send_byte(0x05);
	spi_send_byte(reg_add);
	i=spi_send_byte(0x00);
	LD3320_CS=1;
	return(i);
}

u8 LD_GetResult(void)
{
	return LD_ReadReg(0xc5);
}

u8 LD_AsrRun(void)
{
	LD_WriteReg(0x35, MIC_VOL);//����MIC����
	LD_WriteReg(0x1C, 0x09);   //ADC����д�뱣����
	LD_WriteReg(0xBD, 0x20);   //��ʼ�����ƼĴ���д�뱣����
	LD_WriteReg(0x08, 0x01);   //���FIFO_DATA
	LD3320_delay( 5 );
	LD_WriteReg(0x08, 0x00);   //���֮��Ҫ��дһ��0x00
	LD3320_delay( 5);

	if(LD_Check_ASRBusyFlag_b2() == 0)  //DSPæµ
	{
		return 0;  //����ʧ��״̬
	}

	LD_WriteReg(0xB2, 0xff);	//DSPæµ״̬�Ĵ�����ʼ��
	LD_WriteReg(0x37, 0x06);  //֪ͨDSP��ʼ����ʶ��
	LD3320_delay(5);
	LD_WriteReg(0x1C, 0x0b);  //ADC����,MIC�������
	LD_WriteReg(0x29, 0x10);  //ͬ���ж�����
	LD_WriteReg(0xBD, 0x00);  //����ΪASRģ��
	return 1;
}



void LD_Init_Common(void)
{
	LD_ReadReg(0x06);  
	LD_WriteReg(0x17, 0x35); //��LD3320������λ
	LD3320_delay(5);
	LD_ReadReg(0x06);  

	LD_WriteReg(0x89, 0x03);  //ģ���·���Ƴ�ʼ��
	LD3320_delay(5);
	LD_WriteReg(0xCF, 0x43);  //�ڲ�ʡ��ģʽ��ʼ��
	LD3320_delay(5);
	LD_WriteReg(0xCB, 0x02);
	
	/*PLL setting*/
	LD_WriteReg(0x11, LD_PLL_11); 	
	if (nLD_Mode == LD_MODE_MP3)
	{
		LD_WriteReg(0x1E, 0x00); 
		//!!ע��,���������Ĵ���,���澧��Ƶ�ʱ仯�����ò�ͬ
		//!!ע��,�����ʹ�õľ���Ƶ���޸Ĳο������е� CLK_IN
		LD_WriteReg(0x19, LD_PLL_MP3_19);   
		LD_WriteReg(0x1B, LD_PLL_MP3_1B);   
		LD_WriteReg(0x1D, LD_PLL_MP3_1D);
	}
	else
	{
		LD_WriteReg(0x1E,0x00);
		//!!ע��,���������Ĵ���,���澧��Ƶ�ʱ仯�����ò�ͬ
		//!!ע��,�����ʹ�õľ���Ƶ���޸Ĳο������е� CLK_IN
		LD_WriteReg(0x19, LD_PLL_ASR_19); 
		LD_WriteReg(0x1B, LD_PLL_ASR_1B);		
	  LD_WriteReg(0x1D, LD_PLL_ASR_1D);
	}
	LD3320_delay(5);
	
	LD_WriteReg(0xCD, 0x04); //��ʼ��ʱ����DSP����
	LD_WriteReg(0x17, 0x4c); //ʹDSP����
	LD3320_delay(1);
	LD_WriteReg(0xB9, 0x00); //���ʶ���ֳ�ʼ��,ÿ���һ������Ҫ�趨һ��
	LD_WriteReg(0xCF, 0x4F); //MP3��ʼ����ASR��ʼ��
	LD_WriteReg(0x6F, 0xFF); //��оƬ���г�ʼ��
}

void LD_Init_ASR(void)
{
	nLD_Mode=LD_MODE_ASR_RUN;  //Ĭ��ASRʶ������
	LD_Init_Common();

	LD_WriteReg(0xBD, 0x00);
	LD_WriteReg(0x17, 0x48);	
	LD3320_delay(5);
	LD_WriteReg(0x3C, 0x80);    
	LD_WriteReg(0x3E, 0x07);
	LD_WriteReg(0x38, 0xff);    
	LD_WriteReg(0x3A, 0x07);
	LD_WriteReg(0x40, 0);          
	LD_WriteReg(0x42, 8);
	LD_WriteReg(0x44, 0);    
	LD_WriteReg(0x46, 8); 
	LD3320_delay(1);
}
//�Ĵ������� end

//MP3����
void LD_Play_MP3(const char *path,const u8 Volume)
{
	//�ж�path�Ƿ�Ϊ��
	if(path == NULL || strlen(path) == 0)
	{
		nAsrStatus = LD_ASR_NONE;
		printf("play path is null \r\n");
		return;
	}
	//����APPָ����ƣ��ر�ASRʶ��
	nAsrStatus = LD_MP3_PLAY;
	LD_WriteReg(0x2b,0);
	LD_WriteReg(0x1C,0);//д0:ADC������
	
	bMp3Play = 1;
	while(!(f_open(&fsrc, path,FA_READ) == FR_OK));//���ļ�
	nMp3Size = fsrc.fsize;
	LD_Init_MP3();									//��LD3320��ʼ��Ϊ����MP3ģʽ
	LD_AdjustMIX2SPVolume(Volume);	//���ö�������  
	LD_Play();											//��ʼ����
}

/*
Name : LD_AdjustMIX2SPVolume
Function : ������������
						val :  2~5λ��Ч
			7 6 5 4 3 2 1 0
			x x 0 0 0 0 x x  	�������
			x x 1 1 1 1 x x		������С
*/
void LD_AdjustMIX2SPVolume(u8 val) 
{
	ucSPVol = val; //��¼��ǰ����������
	val = ((15-val)&0x0f) << 2; 
	LD_WriteReg(0x8E, val | 0xc3);//��������
	LD_WriteReg(0x87, 0x78);//ʹ���ڵ�������Ч
}

void LD_Init_MP3(void)
{
	nLD_Mode=LD_MODE_MP3;	//��ǰ����MP3ģʽ
	LD_Init_Common();//ͨ�ó�ʼ��
	
	LD_WriteReg(0xBD,0x02);//����MP3ģ��
	LD_WriteReg(0x17,0x48);//����DSP
	delay_ms(10);//��ʱ1ms
	LD_WriteReg(0x85,0x52);//��ʼ��
	LD_WriteReg(0x8F,0x00);//LineOutѡ��
	LD_WriteReg(0x81,0x00);//��������������Ϊ�����
	LD_WriteReg(0x83,0x00);//��������������Ϊ�����
	LD_WriteReg(0x8E,0xff);//�������������С
	LD_WriteReg(0x8D,0xff);//�ڲ�����
	delay_ms(1);//��ʱ1ms
	LD_WriteReg(0x87,0xff);//ģ���·���Ƴ�ʼ��
	LD_WriteReg(0x89,0xff);//ģ���·����
	delay_ms(1);//��ʱ1ms
	LD_WriteReg(0x22,0x00);//(LowerBoundary L)
	LD_WriteReg(0x23,0x00);//(LowerBoundary H)
	LD_WriteReg(0x20,0xef);//(UpperBoundary L)
	LD_WriteReg(0x21,0x07);//(UpperBoundary H)
	LD_WriteReg(0x24,0x77);//(MCU water mark L)
	LD_WriteReg(0x25,0x03);//(MCU water mark H)
	LD_WriteReg(0x26,0xbb);//(DSP water mark L)
	LD_WriteReg(0x27,0x01);//(DSP water mark H)
}

void LD_Play(void)
{
	
	nMp3Pos=0; //���ų���1
	//����������ܵ�MP3���ݳ���
	if(nMp3Pos>=nMp3Size)
		return;

	Fill_the_Fifo();//�ɲ����ж� Ȼ����� Reload���в���
	LD_WriteReg(0xBA, 0x00);//�жϸ�����Ϣ ����Ϊ00
	LD_WriteReg(0x17, 0x48);//����DSP
	LD_WriteReg(0x33, 0x01);//��ʼ����ʱд��01H
	LD_WriteReg(0x29, 0x04);//����ͬ���ж�
	LD_WriteReg(0x02, 0x01);//����FIFO_DATA�ж�
	LD_WriteReg(0x85, 0x5A);//MP3���Ÿı��ڲ�����
	//EX0=1;���ⲿ�ж�0
	EXTI_ClearITPendingBit(EXTI_Line12);
}

void Fill_the_Fifo(void)
{
  u8 ucStatus;
	int i = 0;
	ucStatus = LD_ReadReg(0x06);
	//fifo�Ƿ�����
	while ( !(ucStatus&MASK_FIFO_STATUS_AFULL))
	{
		LD_WriteReg(0x01,0xff);
		i++;
		ucStatus = LD_ReadReg(0x06);
	}
}

void LD_ReloadMp3Data(void)
{
	u8 ucStatus;
	u8 val;
	u32 i;

	UINT br;

	//Read Command
	ucStatus=LD_ReadReg(0x06);
	//���� FIFO_Status_Allfull ���������ж� �ٴε���������� ��ѭ����ȡ �����Ĳ���
	while(!(ucStatus&MASK_FIFO_STATUS_AFULL)&&nMp3Pos<nMp3Size)
	{
		//��SD����ȡMP3����
		f_read(&fsrc,&val,1,&br);
		nMp3Pos++;
		LD_WriteReg(0x01,val);//��������
		ucStatus=LD_ReadReg(0x06);
	}
	//�����˳� �ڴ���Ҫ�ж��Ƿ񲥷���� ��ֹ�����ݿ����˵��˳������
	if(nMp3Pos>=nMp3Size)
	{
		LD_WriteReg(0xBC, 0x01);
		LD_WriteReg(0x29, 0x10);
		//�ȴ�оƬ���� ���ʱ�䳬����ǿ�ƹرյȴ�MP3�������
		i=600000;
		while(!(LD_ReadReg(0xBA)&CAUSE_MP3_SONG_END)&&i>0)
		{
			i--;
			//printf("Play busy\r\n");
		}
		bMp3Play=0;					// ��������ȫ����������޸�bMp3Play�ı���
		printf("\r\nPlay Over\r\n");//�������
		LD_WriteReg(0x2B,  0);
		LD_WriteReg(0xBA, 0);	
		LD_WriteReg(0xBC,0x0);	

		LD_WriteReg(0x08,1);

		LD_WriteReg(0x08,0);
		LD_WriteReg(0x33, 0);
		MP3_END_Reset();
	}
}

void MP3_END_Reset(void)
{
	f_close(&fsrc);//�ر��ļ�
	nLD_Mode = LD_MODE_ASR_RUN;
	nAsrStatus = LD_ASR_NONE;
}





