#include "mpu.h"
#include "led.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//MPU��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

//����ĳ�������MPU����
//baseaddr:MPU��������Ļ�ַ(�׵�ַ)
//size:MPU��������Ĵ�С(������32�ı���,��λΪ�ֽ�),�����õ�ֵ�ο�:CORTEX_MPU_Region_Size
//rnum:MPU���������,��Χ:0~7,���֧��8����������,�����õ�ֵ�ο���CORTEX_MPU_Region_Number
//ap:����Ȩ��,���ʹ�ϵ����:�����õ�ֵ�ο���CORTEX_MPU_Region_Permission_Attributes
//MPU_REGION_NO_ACCESS,�޷��ʣ���Ȩ&�û������ɷ��ʣ�
//MPU_REGION_PRIV_RW,��֧����Ȩ��д����
//MPU_REGION_PRIV_RW_URO,��ֹ�û�д���ʣ���Ȩ�ɶ�д���ʣ�
//MPU_REGION_FULL_ACCESS,ȫ���ʣ���Ȩ&�û����ɷ��ʣ�
//MPU_REGION_PRIV_RO,��֧����Ȩ������
//MPU_REGION_PRIV_RO_URO,ֻ������Ȩ&�û���������д��
//���:STM32F7����ֲ�.pdf,4.6��,Table 89.
//sen:�Ƿ�������;MPU_ACCESS_NOT_SHAREABLE,������;MPU_ACCESS_SHAREABLE,����
//cen:�Ƿ�����cache;MPU_ACCESS_NOT_CACHEABLE,������;MPU_ACCESS_CACHEABLE,����
//ben:�Ƿ�������;MPU_ACCESS_NOT_BUFFERABLE,������;MPU_ACCESS_BUFFERABLE,����
//����ֵ;0,�ɹ�.
//    ����,����.
u8 MPU_Set_Protection(u32 baseaddr,u32 size,u32 rnum,u8 ap,u8 sen,u8 cen,u8 ben)
{
	MPU_Region_InitTypeDef MPU_Initure;
	
	HAL_MPU_Disable();								        //����MPU֮ǰ�ȹر�MPU,��������Ժ���ʹ��MPU

	MPU_Initure.Enable=MPU_REGION_ENABLE;			        //ʹ�ܸñ������� 
	MPU_Initure.Number=rnum;			                    //���ñ�������
	MPU_Initure.BaseAddress=baseaddr;	                    //���û�ַ
	MPU_Initure.Size=size;				                    //���ñ��������С
	MPU_Initure.SubRegionDisable=0X00;                      //��ֹ������
	MPU_Initure.TypeExtField=MPU_TEX_LEVEL0;                //����������չ��Ϊlevel0
	MPU_Initure.AccessPermission=(u8)ap;		            //���÷���Ȩ��,
	MPU_Initure.DisableExec=MPU_INSTRUCTION_ACCESS_ENABLE;	//����ָ�����(�����ȡָ��)
	MPU_Initure.IsShareable=sen;                            //�Ƿ���?
    MPU_Initure.IsCacheable=cen;                            //�Ƿ�cache?     
	MPU_Initure.IsBufferable=ben;                           //�Ƿ񻺳�?
	HAL_MPU_ConfigRegion(&MPU_Initure);                     //����MPU
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);			        //����MPU
    return 0;
}

//������Ҫ�����Ĵ洢��
//����Բ��ִ洢�������MPU����,������ܵ��³��������쳣
//����MCU������ʾ,����ͷ�ɼ����ݳ���ȵ�����...
void MPU_Memory_Protection(void)
{
    //��������D1 SRAM 512KB
    MPU_Set_Protection( 0x24000000,                 //����ַ
                        MPU_REGION_SIZE_512KB,      //����
                        MPU_REGION_NUMBER1,         //NUMER1
                        MPU_REGION_FULL_ACCESS,     //ȫ����
                        MPU_ACCESS_SHAREABLE,       //������
                        MPU_ACCESS_CACHEABLE,       //����cache
                        MPU_ACCESS_NOT_BUFFERABLE); //��ֹ����
    
    //����SDRAM����,��32M�ֽ�  
    MPU_Set_Protection( 0XC0000000,                 //����ַ
                        MPU_REGION_SIZE_32MB,       //����
                        MPU_REGION_NUMBER2,         //NUMER2
                        MPU_REGION_FULL_ACCESS,     //ȫ����
                        MPU_ACCESS_NOT_SHAREABLE,   //��ֹ����
                        MPU_ACCESS_CACHEABLE,       //����cache
                        MPU_ACCESS_BUFFERABLE);     //������
}


