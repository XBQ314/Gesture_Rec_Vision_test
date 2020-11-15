#include "sai.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//SAI��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/8/17
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

SAI_HandleTypeDef SAI1A_Handler;        //SAI1 Block A���
DMA_HandleTypeDef SAI1_TXDMA_Handler;   //DMA���;��

//SAI Block A��ʼ��,I2S,�����ֱ�׼
//mode:����ģʽ,��������:SAI_MODEMASTER_TX/SAI_MODEMASTER_RX/SAI_MODESLAVE_TX/SAI_MODESLAVE_RX
//cpol:������ʱ�ӵ�����/�½���ѡͨ���������ã�SAI_CLOCKSTROBING_FALLINGEDGE/SAI_CLOCKSTROBING_RISINGEDGE
//datalen:���ݴ�С,�������ã�SAI_DATASIZE_8/10/16/20/24/32
void SAIA_Init(u32 mode,u32 cpol,u32 datalen)
{
    HAL_SAI_DeInit(&SAI1A_Handler);                          //�����ǰ������
    SAI1A_Handler.Instance=SAI1_Block_A;                     //SAI1 Bock A
    SAI1A_Handler.Init.AudioMode=mode;                       //����SAI1����ģʽ
    SAI1A_Handler.Init.Synchro=SAI_ASYNCHRONOUS;             //��Ƶģ���첽
    SAI1A_Handler.Init.OutputDrive=SAI_OUTPUTDRIVE_ENABLE;   //����������Ƶģ�����
    SAI1A_Handler.Init.NoDivider=SAI_MASTERDIVIDER_ENABLE;   //ʹ����ʱ�ӷ�Ƶ��(MCKDIV)
    SAI1A_Handler.Init.FIFOThreshold=SAI_FIFOTHRESHOLD_1QF;  //����FIFO��ֵ,1/4 FIFO
    SAI1A_Handler.Init.MonoStereoMode=SAI_STEREOMODE;        //������ģʽ
    SAI1A_Handler.Init.Protocol=SAI_FREE_PROTOCOL;           //����SAI1Э��Ϊ:����Э��(֧��I2S/LSB/MSB/TDM/PCM/DSP��Э��)
    SAI1A_Handler.Init.DataSize=datalen;                     //�������ݴ�С
    SAI1A_Handler.Init.FirstBit=SAI_FIRSTBIT_MSB;            //����MSBλ����
    SAI1A_Handler.Init.ClockStrobing=cpol;                   //������ʱ�ӵ�����/�½���ѡͨ
    
    //֡����
    SAI1A_Handler.FrameInit.FrameLength=64;                  //����֡����Ϊ64,��ͨ��32��SCK,��ͨ��32��SCK.
    SAI1A_Handler.FrameInit.ActiveFrameLength=32;            //����֡ͬ����Ч��ƽ����,��I2Sģʽ��=1/2֡��.
    SAI1A_Handler.FrameInit.FSDefinition=SAI_FS_CHANNEL_IDENTIFICATION;//FS�ź�ΪSOF�ź�+ͨ��ʶ���ź�
    SAI1A_Handler.FrameInit.FSPolarity=SAI_FS_ACTIVE_LOW;    //FS�͵�ƽ��Ч(�½���)
    SAI1A_Handler.FrameInit.FSOffset=SAI_FS_BEFOREFIRSTBIT;  //��slot0�ĵ�һλ��ǰһλʹ��FS,��ƥ������ֱ�׼	

    //SLOT����
    SAI1A_Handler.SlotInit.FirstBitOffset=0;                 //slotƫ��(FBOFF)Ϊ0
    SAI1A_Handler.SlotInit.SlotSize=SAI_SLOTSIZE_32B;        //slot��СΪ32λ
    SAI1A_Handler.SlotInit.SlotNumber=2;                     //slot��Ϊ2��    
    SAI1A_Handler.SlotInit.SlotActive=SAI_SLOTACTIVE_0|SAI_SLOTACTIVE_1;//ʹ��slot0��slot1
    
    HAL_SAI_Init(&SAI1A_Handler);                            //��ʼ��SAI
    __HAL_SAI_ENABLE(&SAI1A_Handler);                        //ʹ��SAI 
}

//SAI�ײ��������������ã�ʱ��ʹ��
//�˺����ᱻHAL_SAI_Init()����
//hsdram:SAI���
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
    GPIO_InitTypeDef GPIO_Initure;
	
    __HAL_RCC_SAI1_CLK_ENABLE();                //ʹ��SAI1ʱ��
    __HAL_RCC_GPIOE_CLK_ENABLE();               //ʹ��GPIOEʱ��
	
    //��ʼ��PE2,3,4,5,6
    GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;  
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //���츴��
    GPIO_Initure.Pull=GPIO_PULLUP;              //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;         //����
    GPIO_Initure.Alternate=GPIO_AF6_SAI1;       //����ΪSAI   
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);         //��ʼ��
}

//SAI Block A����������
//�����ʼ��㹫ʽ:
//MCKDIV!=0: Fs=SAI_CK_x/[512*MCKDIV]
//MCKDIV==0: Fs=SAI_CK_x/256
//SAI_CK_x=(HSE/pllm)*PLLSAIN/PLLSAIQ/(PLLSAIDivQ+1)
//һ��HSE=25Mhz
//pllm:��Stm32_Clock_Init���õ�ʱ��ȷ����һ����25
//PLLSAIN:һ����50~432 
//PLLSAIQ:2~15 
//PLLSAIDivQ:1~32
//MCKDIV:0~15 
//SAI A��Ƶϵ����@pllm=25,HSE=25Mhz,��vco����Ƶ��Ϊ1Mhz 
const u16 SAI_PSC_TBL[][5]=
{
	{800 ,344,7,1,12},	//8Khz������
	{1102,429,2,19,2},	//11.025Khz������ 
	{1600,344,7, 1,6},	//16Khz������
	{2205,429,2,19,1},	//22.05Khz������
	{3200,344,7, 1,3},	//32Khz������
	{4410,429,2,19,0},	//44.1Khz������
	{4800,344,7, 1,2},	//48Khz������
	{8820,271,2, 3,1},	//88.2Khz������
	{9600,344,7, 1,1},	//96Khz������
	{17640,271,6,3,0},	//176.4Khz������ 
	{19200,295,6,1,0},	//192Khz������
};

//����SAI��DMA����,HAL��û���ṩ�˺���
//���������Ҫ�Լ������Ĵ�����дһ��
void SAIA_DMA_Enable(void)
{
    u32 tempreg=0;
    tempreg=SAI1_Block_A->CR1;          //�ȶ�����ǰ������			
	tempreg|=1<<17;					    //ʹ��DMA
	SAI1_Block_A->CR1=tempreg;		    //д��CR1�Ĵ�����
}


//����SAIA�Ĳ�����(@MCKEN)
//samplerate:������,��λ:Hz
//����ֵ:0,���óɹ�;1,�޷�����.
u8 SAIA_SampleRate_Set(u32 samplerate)
{   
    u8 i=0;   
    RCC_PeriphCLKInitTypeDef RCCSAI1_Sture; 
	
	for(i=0;i<(sizeof(SAI_PSC_TBL)/10);i++)//�����Ĳ������Ƿ����֧��
	{
		if((samplerate/10)==SAI_PSC_TBL[i][0])break;
	}
    if(i==(sizeof(SAI_PSC_TBL)/10))return 1;//�ѱ���Ҳ�Ҳ���
	
	RCCSAI1_Sture.PeriphClockSelection=RCC_PERIPHCLK_SAI1;
	RCCSAI1_Sture.Sai1ClockSelection=RCC_SAI1CLKSOURCE_PLL2;
	RCCSAI1_Sture.PLL2.PLL2M=25;
	RCCSAI1_Sture.PLL2.PLL2N=(u32)SAI_PSC_TBL[i][1];
	RCCSAI1_Sture.PLL2.PLL2P=(u32)SAI_PSC_TBL[i][2];
	HAL_RCCEx_PeriphCLKConfig(&RCCSAI1_Sture);
  
    __HAL_SAI_DISABLE(&SAI1A_Handler);                          //�ر�SAI
    SAI1A_Handler.Init.AudioFrequency=samplerate;               //���ò���Ƶ��  
    HAL_SAI_Init(&SAI1A_Handler);                               //��ʼ��SAI
    SAIA_DMA_Enable();                                          //����SAI��DMA����
    __HAL_SAI_ENABLE(&SAI1A_Handler);                           //����SAI
    return 0;
} 

//SAIA TX DMA����
//����Ϊ˫����ģʽ,������DMA��������ж�
//buf0:M0AR��ַ.
//buf1:M1AR��ַ.
//num:ÿ�δ���������
//width:λ��(�洢��������,ͬʱ����),0,8λ;1,16λ;2,32λ;
void SAIA_TX_DMA_Init(u8* buf0,u8 *buf1,u16 num,u8 width)
{ 
    u32 memwidth=0,perwidth=0;      //����ʹ洢��λ��
    switch(width)
    {
        case 0:         //8λ
            memwidth=DMA_MDATAALIGN_BYTE;
            perwidth=DMA_PDATAALIGN_BYTE;
            break;
        case 1:         //16λ
            memwidth=DMA_MDATAALIGN_HALFWORD;
            perwidth=DMA_PDATAALIGN_HALFWORD;
            break;
        case 2:         //32λ
            memwidth=DMA_MDATAALIGN_WORD;
            perwidth=DMA_PDATAALIGN_WORD;
            break;
            
    }
    __HAL_RCC_DMA1_CLK_ENABLE();                                    //ʹ��DMA1ʱ��
    __HAL_LINKDMA(&SAI1A_Handler,hdmatx,SAI1_TXDMA_Handler);        //��DMA��SAI��ϵ����
    SAI1_TXDMA_Handler.Instance=DMA1_Stream5;                       //DMA1������5 
	SAI1_TXDMA_Handler.Init.Request=DMA_REQUEST_SAI1_A;				//SAI1 Bock A
    SAI1_TXDMA_Handler.Init.Direction=DMA_MEMORY_TO_PERIPH;         //�洢��������ģʽ
    SAI1_TXDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;             //���������ģʽ
    SAI1_TXDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;                 //�洢������ģʽ
    SAI1_TXDMA_Handler.Init.PeriphDataAlignment=perwidth;           //�������ݳ���:16/32λ
    SAI1_TXDMA_Handler.Init.MemDataAlignment=memwidth;              //�洢�����ݳ���:16/32λ
    SAI1_TXDMA_Handler.Init.Mode=DMA_CIRCULAR;                      //ʹ��ѭ��ģʽ 
    SAI1_TXDMA_Handler.Init.Priority=DMA_PRIORITY_HIGH;             //�����ȼ�
    SAI1_TXDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;          //��ʹ��FIFO
    SAI1_TXDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;             //�洢������ͻ������
    SAI1_TXDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;          //����ͻ�����δ��� 
    HAL_DMA_DeInit(&SAI1_TXDMA_Handler);                            //�������ǰ������
    HAL_DMA_Init(&SAI1_TXDMA_Handler);	                            //��ʼ��DMA

    HAL_DMAEx_MultiBufferStart(&SAI1_TXDMA_Handler,(u32)buf0,(u32)&SAI1_Block_A->DR,(u32)buf1,num);//����˫����
    __HAL_DMA_DISABLE(&SAI1_TXDMA_Handler);                         //�ȹر�DMA 
    delay_us(10);                                                   //10us��ʱ����ֹ-O2�Ż������� 	
    __HAL_DMA_ENABLE_IT(&SAI1_TXDMA_Handler,DMA_IT_TC);             //������������ж�
    __HAL_DMA_CLEAR_FLAG(&SAI1_TXDMA_Handler,DMA_FLAG_TCIF1_5);     //���DMA��������жϱ�־λ
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn,0,0);                    //DMA�ж����ȼ�
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}
  
//SAI DMA�ص�����ָ��
void (*sai_tx_callback)(void);	//TX�ص����� 
//DMA1_Stream5�жϷ�����
void DMA1_Stream5_IRQHandler(void)
{   
    if(__HAL_DMA_GET_FLAG(&SAI1_TXDMA_Handler,DMA_FLAG_TCIF1_5)!=RESET) //DMA�������
    {
        __HAL_DMA_CLEAR_FLAG(&SAI1_TXDMA_Handler,DMA_FLAG_TCIF1_5);     //���DMA��������жϱ�־λ
        if(sai_tx_callback!=NULL)sai_tx_callback();	//ִ�лص�����,��ȡ���ݵȲ����������洦��  
    }  											 
}  
//SAI��ʼ����
void SAI_Play_Start(void)
{   	
    __HAL_DMA_ENABLE(&SAI1_TXDMA_Handler);//����DMA TX����  			
}
//�ر�I2S����
void SAI_Play_Stop(void)
{   
    __HAL_DMA_DISABLE(&SAI1_TXDMA_Handler);  //��������  	 	 
} 

