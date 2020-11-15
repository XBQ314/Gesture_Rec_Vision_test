#ifndef __HJPGD_H
#define __HJPGD_H
#include "sys.h"
#include "jpegcodec.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32H7开发板
//图片解码 驱动代码-jpeg硬件解码部分	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2018/8/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//********************************************************************************
//升级说明
//无
//////////////////////////////////////////////////////////////////////////////////
	

extern jpeg_codec_typedef hjpgd;  

 

void jpeg_dma_in_callback(void);
void jpeg_dma_out_callback(void);
void jpeg_endofcovert_callback(void);
void jpeg_hdrover_callback(void);
u8 hjpgd_decode(u8* pname);

#endif




























