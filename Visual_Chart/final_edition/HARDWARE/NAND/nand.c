#include "nand.h"
#include "delay.h"
#include "malloc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//NAND��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/8/16
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

NAND_HandleTypeDef NAND_Handler;    //NAND FLASH���
nand_attriute nand_dev;             //nand��Ҫ�����ṹ��

//��ʼ��NAND FLASH
u8 NAND_Init(void)
{ 
    FMC_NAND_PCC_TimingTypeDef ComSpaceTiming,AttSpaceTiming;
     
    NAND_MPU_Config();  
    NAND_Handler.Instance=FMC_Bank3;
    NAND_Handler.Init.NandBank=FMC_NAND_BANK3;                          //NAND����BANK3��
    NAND_Handler.Init.Waitfeature=FMC_NAND_PCC_WAIT_FEATURE_DISABLE;    //�رյȴ�����
    NAND_Handler.Init.MemoryDataWidth=FMC_NAND_PCC_MEM_BUS_WIDTH_8;     //8λ���ݿ��
    NAND_Handler.Init.EccComputation=FMC_NAND_ECC_DISABLE;              //��ֹECC
    NAND_Handler.Init.ECCPageSize=FMC_NAND_ECC_PAGE_SIZE_512BYTE;       //ECCҳ��СΪ512�ֽ�
    NAND_Handler.Init.TCLRSetupTime=10;                                 //����TCLR(tCLR=CLE��RE����ʱ)=(TCLR+TSET+2)*THCLK,THCLK=1/200M=5ns
    NAND_Handler.Init.TARSetupTime=10;                                  //����TAR(tAR=ALE��RE����ʱ)=(TAR+TSET+1)*THCLK,THCLK=1/200M=5n��   
   
    ComSpaceTiming.SetupTime=10;         //����ʱ��
    ComSpaceTiming.WaitSetupTime=10;    //�ȴ�ʱ��
    ComSpaceTiming.HoldSetupTime=10;    //����ʱ��
    ComSpaceTiming.HiZSetupTime=10;     //����̬ʱ��
    
    AttSpaceTiming.SetupTime=10;         //����ʱ��
    AttSpaceTiming.WaitSetupTime=10;    //�ȴ�ʱ��
    AttSpaceTiming.HoldSetupTime=10;    //����ʱ��
    AttSpaceTiming.HiZSetupTime=10;     //����̬ʱ��

    HAL_NAND_Init(&NAND_Handler,&ComSpaceTiming,&AttSpaceTiming); 
    NAND_Reset();       		        //��λNAND
    delay_ms(100);
    nand_dev.id=NAND_ReadID();	        //��ȡID
    printf("NAND ID:%#x\r\n",nand_dev.id);
	NAND_ModeSet(4);			        //����ΪMODE4,����ģʽ 
    if(nand_dev.id==MT29F16G08ABABA)    //NANDΪMT29F16G08ABABA
    {
        nand_dev.page_totalsize=4320;       
        nand_dev.page_mainsize=4096;       
        nand_dev.page_sparesize=224;
        nand_dev.block_pagenum=128;
        nand_dev.plane_blocknum=2048;
        nand_dev.block_totalnum=4096;    
    }
    else if(nand_dev.id==MT29F4G08ABADA)//NANDΪMT29F4G08ABADA
    {
        nand_dev.page_totalsize=2112;
        nand_dev.page_mainsize=2048;
        nand_dev.page_sparesize=64;
        nand_dev.block_pagenum=64;
        nand_dev.plane_blocknum=2048;
        nand_dev.block_totalnum=4096; 
    }else return 1;	//���󣬷���
    return 0;
}

//NAND FALSH�ײ�����,�������ã�ʱ��ʹ��
//�˺����ᱻHAL_NAND_Init()����
void HAL_NAND_MspInit(NAND_HandleTypeDef *hnand)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_FMC_CLK_ENABLE();             //ʹ��FMCʱ��
    __HAL_RCC_GPIOD_CLK_ENABLE();           //ʹ��GPIODʱ��
    __HAL_RCC_GPIOE_CLK_ENABLE();           //ʹ��GPIOEʱ��
    __HAL_RCC_GPIOG_CLK_ENABLE();           //ʹ��GPIOGʱ��
    
	//��ʼ��PD6 R/B����
	GPIO_Initure.Pin=GPIO_PIN_6;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;          //����
    GPIO_Initure.Pull=GPIO_PULLUP;    			//����          
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;         //����
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);
	   
	//��ʼ��PG9 NCE3����
    GPIO_Initure.Pin=GPIO_PIN_9;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //����
    GPIO_Initure.Pull=GPIO_NOPULL;    			//����          
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;         //����
	GPIO_Initure.Alternate=GPIO_AF12_FMC;       //����ΪFMC
    HAL_GPIO_Init(GPIOG,&GPIO_Initure);  
	
    //��ʼ��PD0,1,4,5,11,12,14,15
    GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|\
                     GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_Initure.Pull=GPIO_NOPULL;              
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);

    //��ʼ��PE7,8,9,10
    GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
}

//����MPU��region
void NAND_MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_Initure;
	
	HAL_MPU_Disable();							//����MPU֮ǰ�ȹر�MPU,��������Ժ���ʹ��MPU
	
	//����RAMΪregion1����СΪ256MB��������ɶ�д
	MPU_Initure.Enable=MPU_REGION_ENABLE;			//ʹ��region
	MPU_Initure.Number=NAND_REGION_NUMBER;			//����region��NANDʹ�õ�region0
	MPU_Initure.BaseAddress=NAND_ADDRESS_START;		//region����ַ
	MPU_Initure.Size=NAND_REGION_SIZE;				//region��С
	MPU_Initure.SubRegionDisable=0X00;
	MPU_Initure.TypeExtField=MPU_TEX_LEVEL0;
	MPU_Initure.AccessPermission=MPU_REGION_FULL_ACCESS;	//��region�ɶ�д
	MPU_Initure.DisableExec=MPU_INSTRUCTION_ACCESS_ENABLE;	//�����ȡ�������е�ָ��
	MPU_Initure.IsShareable=MPU_ACCESS_NOT_SHAREABLE;
	MPU_Initure.IsCacheable=MPU_ACCESS_NOT_CACHEABLE;
	MPU_Initure.IsBufferable=MPU_ACCESS_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_Initure);
	
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);			//����MPU
}

//��ȡNAND FLASH��ID
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 NAND_ModeSet(u8 mode)
{   
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_FEATURE;//����������������
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=0X01;		//��ַΪ0X01,����mode
 	*(vu8*)NAND_ADDRESS=mode;					//P1����,����mode
	*(vu8*)NAND_ADDRESS=0;
	*(vu8*)NAND_ADDRESS=0;
	*(vu8*)NAND_ADDRESS=0; 
    if(NAND_WaitForReady()==NSTA_READY)return 0;//�ɹ�
    else return 1;								//ʧ��
}

//��ȡNAND FLASH��ID
//��ͬ��NAND���в�ͬ��������Լ���ʹ�õ�NAND FALSH�����ֲ�����д����
//����ֵ:NAND FLASH��IDֵ
u32 NAND_ReadID(void)
{
    u8 deviceid[5]; 
    u32 id;  
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_READID; //���Ͷ�ȡID����
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=0X00;
	//IDһ����5���ֽ�
    deviceid[0]=*(vu8*)NAND_ADDRESS;      
    deviceid[1]=*(vu8*)NAND_ADDRESS;  
    deviceid[2]=*(vu8*)NAND_ADDRESS; 
    deviceid[3]=*(vu8*)NAND_ADDRESS; 
    deviceid[4]=*(vu8*)NAND_ADDRESS;  
    //þ���NAND FLASH��IDһ��5���ֽڣ�����Ϊ�˷�������ֻȡ4���ֽ����һ��32λ��IDֵ
    //����NAND FLASH�������ֲᣬֻҪ��þ���NAND FLASH����ôһ���ֽ�ID�ĵ�һ���ֽڶ���0X2C
    //�������ǾͿ����������0X2C��ֻȡ�������ֽڵ�IDֵ��
    id=((u32)deviceid[1])<<24|((u32)deviceid[2])<<16|((u32)deviceid[3])<<8|deviceid[4];
    return id;
}  
//��NAND״̬
//����ֵ:NAND״ֵ̬
//bit0:0,�ɹ�;1,����(���/����/READ)
//bit6:0,Busy;1,Ready
u8 NAND_ReadStatus(void)
{
    vu8 data=0; 
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_READSTA;//���Ͷ�״̬����
	data++;data++;data++;data++;data++;	//����ʱ,��ֹ-O2�Ż�,���µĴ���.
 	data=*(vu8*)NAND_ADDRESS;			//��ȡ״ֵ̬
    return data;
}
//�ȴ�NAND׼����
//����ֵ:NSTA_TIMEOUT �ȴ���ʱ��
//      NSTA_READY    �Ѿ�׼����
u8 NAND_WaitForReady(void)
{
    u8 status=0;
    vu32 time=0; 
	while(1)						//�ȴ�ready
	{
		status=NAND_ReadStatus();	//��ȡ״ֵ̬
		if(status&NSTA_READY)break;
		time++;
		if(time>=0X1FFFF)return NSTA_TIMEOUT;//��ʱ
	}  
    return NSTA_READY;//׼����
}  
//��λNAND
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 NAND_Reset(void)
{ 
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_RESET;	//��λNAND
    if(NAND_WaitForReady()==NSTA_READY)return 0;//��λ�ɹ�
    else return 1;								//��λʧ��
} 
//�ȴ�RB�ź�Ϊĳ����ƽ
//rb:0,�ȴ�RB==0
//   1,�ȴ�RB==1
//����ֵ:0,�ɹ�
//       1,��ʱ
u8 NAND_WaitRB(vu8 rb)
{
    vu16 time=0;  
	while(time<10000)
	{
		time++;
		if(NAND_RB==rb)return 0;
	}
	return 1;
}
//NAND��ʱ
void NAND_Delay(vu32 i)
{
	while(i>0)i--;
}
//��ȡNAND Flash��ָ��ҳָ���е�����(main����spare��������ʹ�ô˺���)
//PageNum:Ҫ��ȡ��ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//ColNum:Ҫ��ȡ���п�ʼ��ַ(Ҳ����ҳ�ڵ�ַ),��Χ:0~(page_totalsize-1)
//*pBuffer:ָ�����ݴ洢��
//NumByteToRead:��ȡ�ֽ���(���ܿ�ҳ��)
//����ֵ:0,�ɹ� 
//    ����,�������
u8 NAND_ReadPage(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToRead)
{
    vu16 i=0;
	u8 res=0;
	u8 eccnum=0;		//��Ҫ�����ECC������ÿNAND_ECC_SECTOR_SIZE�ֽڼ���һ��ecc
	u8 eccstart=0;		//��һ��ECCֵ�����ĵ�ַ��Χ
	u8 errsta=0;
	u8 *p;
     *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_A;
    //���͵�ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>16);
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_TRUE1;
    //�������д����ǵȴ�R/B���ű�Ϊ�͵�ƽ����ʵ��Ҫ����ʱ���õģ��ȴ�NAND����R/B���š���Ϊ������ͨ��
    //��STM32��NWAIT����(NAND��R/B����)����Ϊ��ͨIO��������ͨ����ȡNWAIT���ŵĵ�ƽ���ж�NAND�Ƿ�׼��
    //�����ġ����Ҳ����ģ��ķ������������ٶȺܿ��ʱ���п���NAND��û���ü�����R/B��������ʾNAND��æ
    //��״̬��������ǾͶ�ȡ��R/B����,���ʱ��϶������ģ���ʵ��ȷʵ�ǻ����!���Ҳ���Խ���������
    //���뻻����ʱ����,ֻ������������Ϊ��Ч������û������ʱ������
	res=NAND_WaitRB(0);			//�ȴ�RB=0 
    if(res)return NSTA_TIMEOUT;	//��ʱ�˳�
    //����2�д����������ж�NAND�Ƿ�׼���õ�
	res=NAND_WaitRB(1);			//�ȴ�RB=1 
    if(res)return NSTA_TIMEOUT;	//��ʱ�˳�
	if(NumByteToRead%NAND_ECC_SECTOR_SIZE)//����NAND_ECC_SECTOR_SIZE����������������ECCУ��
	{ 
		//��ȡNAND FLASH�е�ֵ
		for(i=0;i<NumByteToRead;i++)
		{
			*(vu8*)pBuffer++ = *(vu8*)NAND_ADDRESS;
		}
	}else
	{
		eccnum=NumByteToRead/NAND_ECC_SECTOR_SIZE;			//�õ�ecc�������
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE;
		p=pBuffer;
		for(res=0;res<eccnum;res++)
		{
			FMC_Bank3->PCR|=1<<6;						//ʹ��ECCУ�� 
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//��ȡNAND_ECC_SECTOR_SIZE������
			{
				*(vu8*)pBuffer++ = *(vu8*)NAND_ADDRESS;
			}		
			while(!(FMC_Bank3->SR&(1<<6)));				//�ȴ�FIFO��	
			nand_dev.ecc_hdbuf[res+eccstart]=FMC_Bank3->ECCR;//��ȡӲ��������ECCֵ
			FMC_Bank3->PCR&=~(1<<6);						//��ֹECCУ��
		} 
		i=nand_dev.page_mainsize+0X10+eccstart*4;			//��spare����0X10λ�ÿ�ʼ��ȡ֮ǰ�洢��eccֵ
		NAND_Delay(30);//�ȴ�tADL 
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=0X05;				//�����ָ��
		//���͵�ַ
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)i;
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(i>>8);
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=0XE0;				//��ʼ������
		NAND_Delay(30);//�ȴ�tADL 
		pBuffer=(u8*)&nand_dev.ecc_rdbuf[eccstart];
		for(i=0;i<4*eccnum;i++)								//��ȡ�����ECCֵ
		{
			*(vu8*)pBuffer++= *(vu8*)NAND_ADDRESS;
		}			
		for(i=0;i<eccnum;i++)								//����ECC
		{
			if(nand_dev.ecc_rdbuf[i+eccstart]!=nand_dev.ecc_hdbuf[i+eccstart])//�����,��ҪУ��
			{
				printf("err hd,rd:0x%x,0x%x\r\n",nand_dev.ecc_hdbuf[i+eccstart],nand_dev.ecc_rdbuf[i+eccstart]); 
 				printf("eccnum,eccstart:%d,%d\r\n",eccnum,eccstart);	
				printf("PageNum,ColNum:%d,%d\r\n",PageNum,ColNum);	
				res=NAND_ECC_Correction(p+NAND_ECC_SECTOR_SIZE*i,nand_dev.ecc_rdbuf[i+eccstart],nand_dev.ecc_hdbuf[i+eccstart]);//ECCУ��
				if(res)errsta=NSTA_ECC2BITERR;				//���2BIT������ECC����
				else errsta=NSTA_ECC1BITERR;				//���1BIT ECC����
			} 
		} 		
	}
    if(NAND_WaitForReady()!=NSTA_READY)errsta=NSTA_ERROR;	//ʧ��
    return errsta;	//�ɹ�   
} 
//��ȡNAND Flash��ָ��ҳָ���е�����(main����spare��������ʹ�ô˺���),���Ա�(FTL����ʱ��Ҫ)
//PageNum:Ҫ��ȡ��ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//ColNum:Ҫ��ȡ���п�ʼ��ַ(Ҳ����ҳ�ڵ�ַ),��Χ:0~(page_totalsize-1)
//CmpVal:Ҫ�Աȵ�ֵ,��u32Ϊ��λ
//NumByteToRead:��ȡ����(��4�ֽ�Ϊ��λ,���ܿ�ҳ��)
//NumByteEqual:�ӳ�ʼλ�ó�����CmpValֵ��ͬ�����ݸ���
//����ֵ:0,�ɹ�
//    ����,�������
u8 NAND_ReadPageComp(u32 PageNum,u16 ColNum,u32 CmpVal,u16 NumByteToRead,u16 *NumByteEqual)
{
    u16 i=0;
	u8 res=0;
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_A;
    //���͵�ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>16);
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_TRUE1;
    //�������д����ǵȴ�R/B���ű�Ϊ�͵�ƽ����ʵ��Ҫ����ʱ���õģ��ȴ�NAND����R/B���š���Ϊ������ͨ��
    //��STM32��NWAIT����(NAND��R/B����)����Ϊ��ͨIO��������ͨ����ȡNWAIT���ŵĵ�ƽ���ж�NAND�Ƿ�׼��
    //�����ġ����Ҳ����ģ��ķ������������ٶȺܿ��ʱ���п���NAND��û���ü�����R/B��������ʾNAND��æ
    //��״̬��������ǾͶ�ȡ��R/B����,���ʱ��϶������ģ���ʵ��ȷʵ�ǻ����!���Ҳ���Խ���������
    //���뻻����ʱ����,ֻ������������Ϊ��Ч������û������ʱ������
	res=NAND_WaitRB(0);			//�ȴ�RB=0 
	if(res)return NSTA_TIMEOUT;	//��ʱ�˳�
    //����2�д����������ж�NAND�Ƿ�׼���õ�
	res=NAND_WaitRB(1);			//�ȴ�RB=1 
    if(res)return NSTA_TIMEOUT;	//��ʱ�˳�  
    for(i=0;i<NumByteToRead;i++)//��ȡ����,ÿ�ζ�4�ֽ�
    {
		if(*(vu32*)NAND_ADDRESS!=CmpVal)break;	//������κ�һ��ֵ,��CmpVal�����,���˳�.
    }
	*NumByteEqual=i;					//��CmpValֵ��ͬ�ĸ���
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//ʧ��
    return 0;	//�ɹ�   
} 
//��NANDһҳ��д��ָ�����ֽڵ�����(main����spare��������ʹ�ô˺���)
//PageNum:Ҫд���ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//ColNum:Ҫд����п�ʼ��ַ(Ҳ����ҳ�ڵ�ַ),��Χ:0~(page_totalsize-1)
//pBbuffer:ָ�����ݴ洢��
//NumByteToWrite:Ҫд����ֽ�������ֵ���ܳ�����ҳʣ���ֽ���������
//����ֵ:0,�ɹ� 
//    ����,�������
u8 NAND_WritePage(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToWrite)
{
    vu16 i=0;  
	u8 res=0;
	u8 eccnum=0;		//��Ҫ�����ECC������ÿNAND_ECC_SECTOR_SIZE�ֽڼ���һ��ecc
	u8 eccstart=0;		//��һ��ECCֵ�����ĵ�ַ��Χ
	
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE0;
    //���͵�ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>16);
	NAND_Delay(30);//�ȴ�tADL 
	if(NumByteToWrite%NAND_ECC_SECTOR_SIZE)//����NAND_ECC_SECTOR_SIZE����������������ECCУ��
	{  
		for(i=0;i<NumByteToWrite;i++)		//д������
		{
			*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
		}
	}else
	{
		eccnum=NumByteToWrite/NAND_ECC_SECTOR_SIZE;			//�õ�ecc�������
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE; 
 		for(res=0;res<eccnum;res++)
		{
			FMC_Bank3->PCR|=1<<6;						//ʹ��ECCУ�� 
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//д��NAND_ECC_SECTOR_SIZE������
			{
				*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
			}		
			while(!(FMC_Bank3->SR&(1<<6)));				//�ȴ�FIFO��	
			nand_dev.ecc_hdbuf[res+eccstart]=FMC_Bank3->ECCR;	//��ȡӲ��������ECCֵ
  			FMC_Bank3->PCR&=~(1<<6);						//��ֹECCУ��
		}  
		i=nand_dev.page_mainsize+0X10+eccstart*4;			//����д��ECC��spare����ַ
		NAND_Delay(30);//�ȴ� 
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=0X85;				//���дָ��
		//���͵�ַ
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)i;
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(i>>8);
		NAND_Delay(30);//�ȴ�tADL 
		pBuffer=(u8*)&nand_dev.ecc_hdbuf[eccstart];
		for(i=0;i<eccnum;i++)					//д��ECC
		{ 
			for(res=0;res<4;res++)				 
			{
				*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
			}
		} 		
	}
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE_TURE1; 
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//ʧ��
    return 0;//�ɹ�   
}
//��NANDһҳ�е�ָ����ַ��ʼ,д��ָ�����ȵĺ㶨����
//PageNum:Ҫд���ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//ColNum:Ҫд����п�ʼ��ַ(Ҳ����ҳ�ڵ�ַ),��Χ:0~(page_totalsize-1)
//cval:Ҫд���ָ������
//NumByteToWrite:Ҫд�������(��4�ֽ�Ϊ��λ)
//����ֵ:0,�ɹ� 
//    ����,�������
u8 NAND_WritePageConst(u32 PageNum,u16 ColNum,u32 cval,u16 NumByteToWrite)
{
    u16 i=0;  
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE0;
    //���͵�ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>16);
	NAND_Delay(30);//�ȴ�tADL 
	for(i=0;i<NumByteToWrite;i++)		//д������,ÿ��д4�ֽ�
	{
		*(vu32*)NAND_ADDRESS=cval;
	} 
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE_TURE1; 
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//ʧ��
    return 0;//�ɹ�   
}
//��һҳ���ݿ�������һҳ,��д��������
//ע��:Դҳ��Ŀ��ҳҪ��ͬһ��Plane�ڣ�
//Source_PageNo:Դҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//Dest_PageNo:Ŀ��ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)  
//����ֵ:0,�ɹ�
//    ����,�������
u8 NAND_CopyPageWithoutWrite(u32 Source_PageNum,u32 Dest_PageNum)
{
	u8 res=0;
    u16 source_block=0,dest_block=0;  
    //�ж�Դҳ��Ŀ��ҳ�Ƿ���ͬһ��plane��
    source_block=Source_PageNum/nand_dev.block_pagenum;
    dest_block=Dest_PageNum/nand_dev.block_pagenum;
    if((source_block%2)!=(dest_block%2))return NSTA_ERROR;	//����ͬһ��plane�� 
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD0;	//��������0X00
    //����Դҳ��ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)Source_PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Source_PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Source_PageNum>>16);
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD1;//��������0X35 
    //�������д����ǵȴ�R/B���ű�Ϊ�͵�ƽ����ʵ��Ҫ����ʱ���õģ��ȴ�NAND����R/B���š���Ϊ������ͨ��
    //��STM32��NWAIT����(NAND��R/B����)����Ϊ��ͨIO��������ͨ����ȡNWAIT���ŵĵ�ƽ���ж�NAND�Ƿ�׼��
    //�����ġ����Ҳ����ģ��ķ������������ٶȺܿ��ʱ���п���NAND��û���ü�����R/B��������ʾNAND��æ
    //��״̬��������ǾͶ�ȡ��R/B����,���ʱ��϶������ģ���ʵ��ȷʵ�ǻ����!���Ҳ���Խ���������
    //���뻻����ʱ����,ֻ������������Ϊ��Ч������û������ʱ������
	res=NAND_WaitRB(0);			//�ȴ�RB=0 
	if(res)return NSTA_TIMEOUT;	//��ʱ�˳�
    //����2�д����������ж�NAND�Ƿ�׼���õ�
	res=NAND_WaitRB(1);			//�ȴ�RB=1 
    if(res)return NSTA_TIMEOUT;	//��ʱ�˳� 
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD2;  //��������0X85
    //����Ŀ��ҳ��ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)Dest_PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Dest_PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Dest_PageNum>>16);
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD3;	//��������0X10 
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;	//NANDδ׼���� 
    return 0;//�ɹ�   
}

//��һҳ���ݿ�������һҳ,���ҿ���д������
//ע��:Դҳ��Ŀ��ҳҪ��ͬһ��Plane�ڣ�
//Source_PageNo:Դҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//Dest_PageNo:Ŀ��ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)  
//ColNo:ҳ���е�ַ,��Χ:0~(page_totalsize-1)
//pBuffer:Ҫд�������
//NumByteToWrite:Ҫд������ݸ���
//����ֵ:0,�ɹ� 
//    ����,�������
u8 NAND_CopyPageWithWrite(u32 Source_PageNum,u32 Dest_PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToWrite)
{
	u8 res=0;
    vu16 i=0;
	u16 source_block=0,dest_block=0;  
	u8 eccnum=0;		//��Ҫ�����ECC������ÿNAND_ECC_SECTOR_SIZE�ֽڼ���һ��ecc
	u8 eccstart=0;		//��һ��ECCֵ�����ĵ�ַ��Χ
    //�ж�Դҳ��Ŀ��ҳ�Ƿ���ͬһ��plane��
    source_block=Source_PageNum/nand_dev.block_pagenum;
    dest_block=Dest_PageNum/nand_dev.block_pagenum;
    if((source_block%2)!=(dest_block%2))return NSTA_ERROR;//����ͬһ��plane��
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD0;  //��������0X00
    //����Դҳ��ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)Source_PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Source_PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Source_PageNum>>16);
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD1;  //��������0X35
    
    //�������д����ǵȴ�R/B���ű�Ϊ�͵�ƽ����ʵ��Ҫ����ʱ���õģ��ȴ�NAND����R/B���š���Ϊ������ͨ��
    //��STM32��NWAIT����(NAND��R/B����)����Ϊ��ͨIO��������ͨ����ȡNWAIT���ŵĵ�ƽ���ж�NAND�Ƿ�׼��
    //�����ġ����Ҳ����ģ��ķ������������ٶȺܿ��ʱ���п���NAND��û���ü�����R/B��������ʾNAND��æ
    //��״̬��������ǾͶ�ȡ��R/B����,���ʱ��϶������ģ���ʵ��ȷʵ�ǻ����!���Ҳ���Խ���������
    //���뻻����ʱ����,ֻ������������Ϊ��Ч������û������ʱ������
	res=NAND_WaitRB(0);			//�ȴ�RB=0 
	if(res)return NSTA_TIMEOUT;	//��ʱ�˳�
    //����2�д����������ж�NAND�Ƿ�׼���õ�
	res=NAND_WaitRB(1);			//�ȴ�RB=1 
    if(res)return NSTA_TIMEOUT;	//��ʱ�˳� 
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD2;  //��������0X85
    //����Ŀ��ҳ��ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)Dest_PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Dest_PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Dest_PageNum>>16); 
    //����ҳ���е�ַ
	NAND_Delay(30);//�ȴ�tADL 
	if(NumByteToWrite%NAND_ECC_SECTOR_SIZE)//����NAND_ECC_SECTOR_SIZE����������������ECCУ��
	{  
		for(i=0;i<NumByteToWrite;i++)		//д������
		{
			*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
		}
	}else
	{
		eccnum=NumByteToWrite/NAND_ECC_SECTOR_SIZE;			//�õ�ecc�������
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE; 
 		for(res=0;res<eccnum;res++)
		{
			FMC_Bank3->PCR|=1<<6;							//ʹ��ECCУ�� 
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//д��NAND_ECC_SECTOR_SIZE������
			{
				*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
			}		
			while(!(FMC_Bank3->SR&(1<<6)));					//�ȴ�FIFO��	
			nand_dev.ecc_hdbuf[res+eccstart]=FMC_Bank3->ECCR;	//��ȡӲ��������ECCֵ
 			FMC_Bank3->PCR&=~(1<<6);						//��ֹECCУ��
		}  
		i=nand_dev.page_mainsize+0X10+eccstart*4;			//����д��ECC��spare����ַ
		NAND_Delay(30);//�ȴ� 
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=0X85;				//���дָ��
		//���͵�ַ
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)i;
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(i>>8);
		NAND_Delay(30);//�ȴ�tADL 
		pBuffer=(u8*)&nand_dev.ecc_hdbuf[eccstart];
		for(i=0;i<eccnum;i++)					//д��ECC
		{ 
			for(res=0;res<4;res++)				 
			{
				*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
			}
		} 		
	}
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD3;	//��������0X10 
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;	//ʧ��
    return 0;	//�ɹ�   
} 
//��ȡspare���е�����
//PageNum:Ҫд���ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//ColNum:Ҫд���spare����ַ(spare�����ĸ���ַ),��Χ:0~(page_sparesize-1) 
//pBuffer:�������ݻ����� 
//NumByteToRead:Ҫ��ȡ���ֽ���(������page_sparesize)
//����ֵ:0,�ɹ�
//    ����,�������
u8 NAND_ReadSpare(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToRead)
{
    u8 temp=0;
    u8 remainbyte=0;
    remainbyte=nand_dev.page_sparesize-ColNum;
    if(NumByteToRead>remainbyte) NumByteToRead=remainbyte;  //ȷ��Ҫд����ֽ���������spareʣ��Ĵ�С
    temp=NAND_ReadPage(PageNum,ColNum+nand_dev.page_mainsize,pBuffer,NumByteToRead);//��ȡ����
    return temp;
} 
//��spare����д����
//PageNum:Ҫд���ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//ColNum:Ҫд���spare����ַ(spare�����ĸ���ַ),��Χ:0~(page_sparesize-1)  
//pBuffer:Ҫд��������׵�ַ 
//NumByteToWrite:Ҫд����ֽ���(������page_sparesize)
//����ֵ:0,�ɹ�
//    ����,ʧ��
u8 NAND_WriteSpare(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToWrite)
{
    u8 temp=0;
    u8 remainbyte=0;
    remainbyte=nand_dev.page_sparesize-ColNum;
    if(NumByteToWrite>remainbyte) NumByteToWrite=remainbyte;  //ȷ��Ҫ��ȡ���ֽ���������spareʣ��Ĵ�С
    temp=NAND_WritePage(PageNum,ColNum+nand_dev.page_mainsize,pBuffer,NumByteToWrite);//��ȡ
    return temp;
} 
//����һ����
//BlockNum:Ҫ������BLOCK���,��Χ:0-(block_totalnum-1)
//����ֵ:0,�����ɹ�
//    ����,����ʧ��
u8 NAND_EraseBlock(u32 BlockNum)
{
	if(nand_dev.id==MT29F16G08ABABA)BlockNum<<=7;  	//�����ַת��Ϊҳ��ַ
    else if(nand_dev.id==MT29F4G08ABADA)BlockNum<<=6;
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_ERASE0;
    //���Ϳ��ַ
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)BlockNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(BlockNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(BlockNum>>16);
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_ERASE1;
	if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//ʧ��
    return 0;	//�ɹ�   
} 
//ȫƬ����NAND FLASH
void NAND_EraseChip(void)
{
    u8 status;
    u16 i=0;
    for(i=0;i<nand_dev.block_totalnum;i++) //ѭ���������еĿ�
    {
        status=NAND_EraseBlock(i);
        if(status)printf("Erase %d block fail!!��������Ϊ%d\r\n",i,status);//����ʧ��
    }
}

//��ȡECC������λ/ż��λ
//oe:0,ż��λ
//   1,����λ
//eccval:�����eccֵ
//����ֵ:������eccֵ(���16λ)
u16 NAND_ECC_Get_OE(u8 oe,u32 eccval)
{
	u8 i;
	u16 ecctemp=0;
	for(i=0;i<24;i++)
	{
		if((i%2)==oe)
		{
			if((eccval>>i)&0X01)ecctemp+=1<<(i>>1); 
		}
	}
	return ecctemp;
} 
//ECCУ������
//eccrd:��ȡ����,ԭ�������ECCֵ
//ecccl:��ȡ����ʱ,Ӳ�������ECCֻ
//����ֵ:0,����������
//    ����,ECC����(�д���2��bit�Ĵ���,�޷��ָ�)
u8 NAND_ECC_Correction(u8* data_buf,u32 eccrd,u32 ecccl)
{
	u16 eccrdo,eccrde,eccclo,ecccle;
	u16 eccchk=0;
	u16 errorpos=0; 
	u32 bytepos=0;  
	eccrdo=NAND_ECC_Get_OE(1,eccrd);	//��ȡeccrd������λ
	eccrde=NAND_ECC_Get_OE(0,eccrd);	//��ȡeccrd��ż��λ
	eccclo=NAND_ECC_Get_OE(1,ecccl);	//��ȡecccl������λ
	ecccle=NAND_ECC_Get_OE(0,ecccl); 	//��ȡecccl��ż��λ
	eccchk=eccrdo^eccrde^eccclo^ecccle;
	if(eccchk==0XFFF)	//ȫ1,˵��ֻ��1bit ECC����
	{
		errorpos=eccrdo^eccclo; 
		printf("errorpos:%d\r\n",errorpos); 
		bytepos=errorpos/8; 
		data_buf[bytepos]^=1<<(errorpos%8);
	}else				//����ȫ1,˵��������2bit ECC����,�޷��޸�
	{
		printf("2bit ecc error or more\r\n");
		return 1;
	} 
	return 0;
}
