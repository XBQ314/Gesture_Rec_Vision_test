#ifndef __SPI_H
#define __SPI_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//SPI��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

extern SPI_HandleTypeDef SPI2_Handler;  //SPI���

void SPI2_Init(void);
void SPI2_SetSpeed(u32 SPI_BaudRatePrescaler);
u8 SPI2_ReadWriteByte(u8 TxData);
#endif
