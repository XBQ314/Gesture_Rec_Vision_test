#ifndef __NANDTESTER_H
#define __NANDTESTER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//NAND FLASH USMART���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	  

u8 test_writepage(u32 pagenum,u16 colnum,u16 writebytes);
u8 test_readpage(u32 pagenum,u16 colnum,u16 readbytes);
u8 test_copypageandwrite(u32 spnum,u32 dpnum,u16 colnum,u16 writebytes);
u8 test_readspare(u32 pagenum,u16 colnum,u16 readbytes);
void test_readallblockinfo(u32 sblock);
u8 test_ftlwritesectors(u32 secx,u16 secsize,u16 seccnt);
u8 test_ftlreadsectors(u32 secx,u16 secsize,u16 seccnt);

#endif
