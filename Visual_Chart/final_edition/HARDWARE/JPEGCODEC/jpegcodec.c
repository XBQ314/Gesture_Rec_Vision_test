#include "jpegcodec.h"
#include "usart.h"
#include "malloc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32H7开发板
//JPEG硬件编解码器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2018/8/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//JPEG规范(ISO/IEC 10918-1标准)的样本量化表
//获取JPEG图片质量时需要用到
const u8 JPEG_LUM_QuantTable[JPEG_QUANT_TABLE_SIZE] = 
{
	16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,
	14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,
	18,22,37,56,68,109,103,77,24,35,55,64,81,104,113,92,
	49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99
};
const u8 JPEG_ZIGZAG_ORDER[JPEG_QUANT_TABLE_SIZE]=
{
	0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
	12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
	35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
	58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
}; 

//JPEG硬件解码输入DMA配置
//meminaddr:JPEG输入DMA存储器地址.  
//meminsize:输入DMA数据长度,0~262143,以字节为单位
void JPEG_IN_DMA_Init(u32 meminaddr,u32 meminsize)
{ 
	u32 regval=0;
	u32 addrmask=0;
	RCC->AHB3ENR|=1<<0;				//使能MDMA时钟 
	MDMA_Channel7->CCR=0;			//输入MDMA清零
	while(MDMA_Channel7->CCR&0X01);	//等待MDMA_Channel7关闭完成
	MDMA_Channel7->CIFCR=0X1F;		//中断标志清零 
	MDMA_Channel7->CCR|=1<<2;		//CTCIE=1,使能通道传输完成中断
	MDMA_Channel7->CCR|=2<<6;		//PL[1:0]=2,高优先级
	MDMA_Channel7->CBNDTR=meminsize;//传输长度为meminsize
	MDMA_Channel7->CDAR=(u32)&JPEG->DIR;//目标地址为:JPEG->DIR
	MDMA_Channel7->CSAR=meminaddr;	//meminaddr作为源地址 	
	regval=0<<28;					//TRGM[1:0]=0,每个MDMA请求触发一次buffer传输
	regval|=1<<25;					//PKE=1,打包使能
	regval|=(32-1)<<18;				//TLEN[6:0]=31,buffer传输长度为32字节.
	regval|=4<<15;					//DBURST[2:0]=4,目标突发传输长度为16
	regval|=4<<12;					//SBURST[2:0]=4,源突发传输长度为16
	regval|=0<<8;					//SINCOS[1:0]=0,源地址变化单位为8位(字节)
	regval|=2<<6;					//DSIZE[1:0]=2,目标位宽为32位
	regval|=0<<4;					//SSIZE[1:0]=0,源位宽为8位
	regval|=0<<2;					//DINC[1:0]=0,目标地址固定
	regval|=2<<0;					//SINC[1:0]=2,源地址自增
	MDMA_Channel7->CTCR=regval;		//设置CTCR寄存器
	MDMA_Channel7->CTBR=17<<0;		//MDMA的硬件触发通道17触发inmdma,通道17=JPEG input FIFO threshold
									//详见<STM32H7xx参考手册>550页,table 91
	addrmask=meminaddr&0XFF000000;	//获取掩码
	if(addrmask==0X20000000||addrmask==0)MDMA_Channel7->CTBR|=1<<16;//使用AHBS总线访问DTCM/ITCM
  
    HAL_NVIC_SetPriority(MDMA_IRQn,1,2);    //设置中断优先级，抢占优先级1，子优先级2
    HAL_NVIC_EnableIRQ(MDMA_IRQn);          //开启MDMA中断  
}  

//JPEG硬件解码输出DMA配置 
//memoutaddr:JPEG输出DMA存储器地址. 
//memoutsize:输出DMA数据长度,0~262143,以字节为单位 
void JPEG_OUT_DMA_Init(u32 memoutaddr,u32 memoutsize)
{ 
	u32 regval=0;
	u32 addrmask=0;
	//RCC->AHB3ENR|=1<<0;			//使能MDMA时钟
	MDMA_Channel6->CCR=0;			//输出MDMA清零
	while(MDMA_Channel6->CCR&0X01);	//等待MDMA_Channel6关闭完成
	MDMA_Channel6->CIFCR=0X1F;		//中断标志清零 
	MDMA_Channel6->CCR|=3<<6;		//PL[1:0]=2,最高优先级
	MDMA_Channel6->CCR|=1<<2;		//CTCIE=1,使能通道传输完成中断
	MDMA_Channel6->CBNDTR=memoutsize;	//传输长度为meminsize
	MDMA_Channel6->CDAR=memoutaddr;		//目标地址为:memoutaddr
	MDMA_Channel6->CSAR=(u32)&JPEG->DOR;//JPEG->DOR作为源地址 	
	regval=0<<28;					//TRGM[1:0]=0,每个MDMA请求触发一次buffer传输
	regval|=1<<25;					//PKE=1,打包使能
	regval|=(32-1)<<18;				//TLEN[6:0]=31,buffer传输长度为32字节.
	regval|=4<<15;					//DBURST[2:0]=4,目标突发传输长度为16
	regval|=4<<12;					//SBURST[2:0]=4,源突发传输长度为16
	regval|=0<<10;					//DINCOS[1:0]=0,目标地址变化单位为8位(字节)
	regval|=0<<6;					//DSIZE[1:0]=0,目标位宽为8位
	regval|=2<<4;					//SSIZE[1:0]=2,源位宽为32位
	regval|=2<<2;					//DINC[1:0]=2,目标地址自增
	regval|=0<<0;					//SINC[1:0]=0,源地址固定
	MDMA_Channel6->CTCR=regval;		//设置CTCR寄存器
	MDMA_Channel6->CTBR=19<<0;		//MDMA的硬件触发通道17触发inmdma,通道17=JPEG input FIFO threshold
									//详见<STM32H7xx参考手册>550页,table 91
	addrmask=memoutaddr&0XFF000000;	//获取掩码
	if(addrmask==0X20000000||addrmask==0)MDMA_Channel6->CTBR|=1<<17;//使用AHBS总线访问DTCM/ITCM
    
    HAL_NVIC_SetPriority(MDMA_IRQn,1,2);    //设置中断优先级，抢占优先级1，子优先级2
    HAL_NVIC_EnableIRQ(MDMA_IRQn);          //开启MDMA中断  
}  

void (*jpeg_in_callback)(void);		//JPEG DMA输入回调函数
void (*jpeg_out_callback)(void);	//JPEG DMA输出 回调函数
void (*jpeg_eoc_callback)(void);	//JPEG 解码完成 回调函数
void (*jpeg_hdp_callback)(void);	//JPEG Header解码完成 回调函数

//MDMA中断服务函数
//处理硬件JPEG解码时输入/输出数据流
void MDMA_IRQHandler(void)
{        
	if(MDMA_Channel7->CISR&(1<<1))	//CTCIF,通道7传输完成(输入)
 	if(MDMA_Channel7->CISR&(1<<1))	//CTCIF,通道7传输完成(输入)
	{
		MDMA_Channel7->CIFCR|=1<<1;	//清除通道传输完成中断  
		JPEG->CR&=~(0X7E);			//关闭JPEG中断,防止被打断.
      	jpeg_in_callback();			//执行输入回调函数,继续读取数据
		JPEG->CR|=3<<5;				//使能EOC和HPD中断.				
	}
	if(MDMA_Channel6->CISR&(1<<1))	//CTCIF,通道6传输完成(输出)
	{
		MDMA_Channel6->CIFCR|=1<<1;	//清除通道传输完成中断  
		JPEG->CR&=~(0X7E);			//关闭JPEG中断,防止被打断.
      	jpeg_out_callback();		//执行输出回调函数,将数据转换成RGB
		JPEG->CR|=3<<5;				//使能EOC和HPD中断.				
	} 	 											 
}   
//JPEG解码中断服务函数
void JPEG_IRQHandler(void)
{
	if(JPEG->SR&(1<<6))				//JPEG Header解码完成
	{ 
		jpeg_hdp_callback();
		JPEG->CR&=~(1<<6);			//禁止Jpeg Header解码完成中断
		JPEG->CFR|=1<<6;			//清除HPDF位(header解码完成位)
	}
	if(JPEG->SR&(1<<5))				//JPEG解码完成   
	{
		JPEG_DMA_Stop();
		jpeg_eoc_callback();
		JPEG->CFR|=1<<5;			//清除EOC位(解码完成位)
		MDMA_Channel6->CCR&=~(1<<0);//关闭MDMA通道6 
		MDMA_Channel7->CCR&=~(1<<0);//关闭MDMA通道7
	}
}

//初始化硬件JPEG内核
//tjpeg:jpeg编解码控制结构体
//返回值:0,成功;
//    其他,失败
u8 JPEG_Core_Init(jpeg_codec_typedef *tjpeg)
{
	u8 i;
	RCC->AHB3ENR|=1<<5;				//使能硬件jpeg时钟 	
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)
	{
		tjpeg->inbuf[i].buf=mymalloc(SRAMDTCM,JPEG_DMA_INBUF_LEN);
		if(tjpeg->inbuf[i].buf==NULL)
		{
			JPEG_Core_Destroy(tjpeg);
			return 1;
		}   
	} 
	JPEG->CR=0;						//先清零
	JPEG->CR|=1<<0;					//使能硬件JPEG
	JPEG->CONFR0&=~(1<<0);			//停止JPEG编解码进程
	JPEG->CR|=1<<13;				//清空输入fifo
	JPEG->CR|=1<<14;				//清空输出fifo
	JPEG->CFR=3<<5;					//清空标志 
	//MY_NVIC_Init(1,3,JPEG_IRQn,2);	//抢占1，子优先级3，组2  
    HAL_NVIC_SetPriority(JPEG_IRQn,1,3);    //设置中断优先级，抢占优先级1，子优先级3
    HAL_NVIC_EnableIRQ(JPEG_IRQn);          //开启JPEG中断 
	JPEG->CONFR1|=1<<8;				//使能header处理
	return 0;
}

//关闭硬件JPEG内核,并释放内存
//tjpeg:jpeg编解码控制结构体
void JPEG_Core_Destroy(jpeg_codec_typedef *tjpeg)
{
	u8 i; 
	JPEG_DMA_Stop();//停止DMA传输
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)myfree(SRAMDTCM,tjpeg->inbuf[i].buf);	//释放内存
	for(i=0;i<JPEG_DMA_OUTBUF_NB;i++)myfree(SRAMIN,tjpeg->outbuf[i].buf);	//释放内存
}

//初始化硬件JPEG解码器
//tjpeg:jpeg编解码控制结构体
void JPEG_Decode_Init(jpeg_codec_typedef *tjpeg)
{ 
	u8 i;
	tjpeg->inbuf_read_ptr=0;
	tjpeg->inbuf_write_ptr=0;
	tjpeg->indma_pause=0;
	tjpeg->outbuf_read_ptr=0;
	tjpeg->outbuf_write_ptr=0;	
	tjpeg->outdma_pause=0;		
	tjpeg->state=JPEG_STATE_NOHEADER;	//图片解码结束标志
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)
	{
		tjpeg->inbuf[i].sta=0;
		tjpeg->inbuf[i].size=0;
	}
	for(i=0;i<JPEG_DMA_OUTBUF_NB;i++)
	{
		tjpeg->outbuf[i].sta=0;
		tjpeg->outbuf[i].size=0;
	}		
	MDMA_Channel6->CCR=0;		//MDMA通道6禁止
	MDMA_Channel7->CCR=0;		//MDMA通道7禁止
	MDMA_Channel6->CIFCR=0X1F;	//中断标志清零 
	MDMA_Channel7->CIFCR=0X1F;	//中断标志清零
	
	JPEG->CONFR1|=1<<3;			//硬件JPEG解码模式
	JPEG->CONFR0&=~(1<<0);		//停止JPEG编解码进程 
	JPEG->CR&=~(0X3F<<1);		//关闭所有中断 
	JPEG->CR|=1<<13;			//清空输入fifo
	JPEG->CR|=1<<14;			//清空输出fifo
	JPEG->CR|=1<<6;				//使能Jpeg Header解码完成中断
	JPEG->CR|=1<<5;				//使能解码完成中断
	JPEG->CFR=3<<5;				//清空标志   
	JPEG->CONFR0|=1<<0;			//使能JPEG编解码进程 
}
//启动JPEG IN DMA,开始解码JPEG
void JPEG_IN_DMA_Start(void)
{ 
	MDMA_Channel7->CCR|=1<<0;	//使能MDMA通道7的传输	
}
//启动JPEG OUT DMA,开始输出YUV数据
void JPEG_OUT_DMA_Start(void)
{
	MDMA_Channel6->CCR|=1<<0;	//使能MDMA通道6的传输 	
}
//停止JPEG DMA解码过程
void JPEG_DMA_Stop(void)
{ 
	JPEG->CONFR0&=~(1<<0);		//停止JPEG编解码进程 
	JPEG->CR&=~(0X3F<<1);		//关闭所有中断  
	JPEG->CFR=3<<5;				//清空标志  	
}  
//恢复DMA IN过程
//memaddr:存储区首地址
//memlen:要传输数据长度(以字节为单位)
void JPEG_IN_DMA_Resume(u32 memaddr,u32 memlen)
{  
	if(memlen%4)memlen+=4-memlen%4;	//扩展到4的倍数  
	MDMA_Channel7->CIFCR=0X1F;		//中断标志清零   
	MDMA_Channel7->CBNDTR=memlen;	//传输长度为memlen 
	MDMA_Channel7->CSAR=memaddr;	//memaddr作为源地址 
	MDMA_Channel7->CCR|=1<<0;		//使能MDMA通道7的传输 
} 
//恢复DMA OUT过程
//memaddr:存储区首地址
//memlen:要传输数据长度(以字节为单位)
void JPEG_OUT_DMA_Resume(u32 memaddr,u32 memlen)
{  
	if(memlen%4)memlen+=4-memlen%4;	//扩展到4的倍数  
	MDMA_Channel6->CIFCR=0X1F;		//中断标志清零   
	MDMA_Channel6->CBNDTR=memlen;	//传输长度为memlen 
	MDMA_Channel6->CDAR=memaddr;	//memaddr作为源地址 
	MDMA_Channel6->CCR|=1<<0;		//使能MDMA通道6的传输
}
//获取图像信息
//tjpeg:jpeg解码结构体
void JPEG_Get_Info(jpeg_codec_typedef *tjpeg)
{ 
	u32 yblockNb,cBblockNb,cRblockNb; 
	switch(JPEG->CONFR1&0X03)
	{
		case 0://grayscale,1 color component
			tjpeg->Conf.ColorSpace=JPEG_GRAYSCALE_COLORSPACE;
			break;
		case 2://YUV/RGB,3 color component
			tjpeg->Conf.ColorSpace=JPEG_YCBCR_COLORSPACE;
			break;	
		case 3://CMYK,4 color component
			tjpeg->Conf.ColorSpace=JPEG_CMYK_COLORSPACE;
			break;			
	}
	tjpeg->Conf.ImageHeight=(JPEG->CONFR1&0XFFFF0000)>>16;	//获得图像高度
	tjpeg->Conf.ImageWidth=(JPEG->CONFR3&0XFFFF0000)>>16;	//获得图像宽度
	if((tjpeg->Conf.ColorSpace==JPEG_YCBCR_COLORSPACE)||(tjpeg->Conf.ColorSpace==JPEG_CMYK_COLORSPACE))
	{
		yblockNb  =(JPEG->CONFR4&(0XF<<4))>>4;
		cBblockNb =(JPEG->CONFR5&(0XF<<4))>>4;
		cRblockNb =(JPEG->CONFR6&(0XF<<4))>>4;
		if((yblockNb==1)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling=JPEG_422_SUBSAMPLING; //16x8 block
		else if((yblockNb==0)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING;
		else if((yblockNb==3)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling = JPEG_420_SUBSAMPLING;
		else tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING; 
	}else tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING;		//默认用4:4:4 
	tjpeg->Conf.ImageQuality=0;	//图像质量参数在整个图片的最末尾,刚开始的时候,是无法获取的,所以直接设置为0
}
//得到jpeg图像质量
//在解码完成后,可以调用并获得正确的结果.
//返回值:图像质量,0~100.
u8 JPEG_Get_Quality(void)
{
	u32 quality=0;
	u32 quantRow,quantVal,scale,i,j;
	u32 *tableAddress=(u32*)JPEG->QMEM0; 
	i=0;
	while(i<JPEG_QUANT_TABLE_SIZE)
	{
		quantRow=*tableAddress;
		for(j=0;j<4;j++)
		{
			quantVal=(quantRow>>(8*j))&0xFF;
			if(quantVal==1)quality+=100;	//100% 
			else
			{
				scale=(quantVal*100)/((u32)JPEG_LUM_QuantTable[JPEG_ZIGZAG_ORDER[i+j]]);
				if(scale<=100)quality+=(200-scale)/2;  
				else quality+=5000/scale;      
			}      
		} 
		i+=4;
		tableAddress++;    
	} 
	return (quality/((u32)64));   
}

//利用DMA2D,将JPEG解码的YUV数据转换成RGB数据,全硬件完成,速度非常快.
//tjpeg:jpeg解码结构体
//pdst:输出数组首地址
//返回值:0,ok
//       1,超时已出
u8 JPEG_DMA2D_YUV2RGB_Conversion(jpeg_codec_typedef *tjpeg,u32 *pdst)
{ 
	u32 regval=0;
	u32 cm=0;						//采样方式
	u32 destination=0,timeout=0; 

	if(tjpeg->Conf.ChromaSubsampling==JPEG_420_SUBSAMPLING)cm=DMA2D_CSS_420;	//YUV420转RGB
	if(tjpeg->Conf.ChromaSubsampling==JPEG_422_SUBSAMPLING)cm=DMA2D_CSS_422;	//YUV422转RGB
	else if(tjpeg->Conf.ChromaSubsampling==JPEG_444_SUBSAMPLING)cm=DMA2D_NO_CSS;//YUV444转RGB	
	destination=(u32)pdst+(tjpeg->yuvblk_curheight*tjpeg->Conf.ImageWidth)*2;	//计算目标地址的首地址 
	RCC->AHB3ENR|=1<<4;				//使能DMA2D时钟	
	RCC->AHB3RSTR|=1<<4;			//复位DMA2D
	RCC->AHB3RSTR&=~(1<<4);			//结束复位
	DMA2D->CR&=~(1<<0);				//先停止DMA2D
	DMA2D->CR=1<<16;				//MODE[1:0]=01,存储器到存储器,带PFC模式
	DMA2D->OPFCCR=2<<0;				//CM[2:0]=010,输出为RGB565格式
	DMA2D->OOR=0;					//设置行偏移为0 
	DMA2D->IFCR|=1<<1;				//清除传输完成标志  	
	regval=11<<0;					//CM[3:0]=1011,输入数据为YCbCr格式
	regval|=cm<<18;					//CSS[1:0]=cm,Chroma Sub-Sampling:0,4:4:4;1,4:2:2;2,4:2:0 
	DMA2D->FGPFCCR=regval;			//设置FGPCCR寄存器
	DMA2D->FGOR=0;					//前景层行偏移为0
	DMA2D->NLR=tjpeg->yuvblk_height|(tjpeg->Conf.ImageWidth<<16);	//设定行数寄存器 
	DMA2D->OMAR=destination;		//输出存储器地址 
	DMA2D->FGMAR=(u32)tjpeg->outbuf[tjpeg->outbuf_read_ptr].buf;	//源地址
	DMA2D->CR|=1<<0;				//启动DMA2D
	while((DMA2D->ISR&(1<<1))==0)	//等待传输完成
	{
		timeout++;
		if(timeout>0X1FFFFF)break;	//超时退出
	} 
	tjpeg->yuvblk_curheight+=tjpeg->yuvblk_height;	//偏移到下一个内存地址 
    //YUV2RGB转码结束后，在复位一次DMA2D
    RCC->AHB3RSTR|=1<<4;    //复位DMA2D
    RCC->AHB3RSTR&=~(1<<4); //结束复位
	if(timeout>0X1FFFFF)return 1;  	
	return 0;
}


 


























