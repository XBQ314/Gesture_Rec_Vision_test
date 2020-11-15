#ifndef __JPEGCODEC_H
#define __JPEGCODEC_H
#include "sys.h"
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

#define JPEG_DMA_INBUF_LEN			4096			//单个DMA IN  BUF的大小 
#define JPEG_DMA_INBUF_NB			10				//DMA IN  BUF的个数
#define JPEG_DMA_OUTBUF_NB			2				//DMA OUT BUF的个数

//JPEG数据缓冲结构体
typedef struct
{
    u8 sta;			//状态:0,无数据;1,有数据.
    u8 *buf;		//JPEG数据缓冲区
    u16 size; 		//JPEG数据长度 
}jpeg_databuf_type; 

#define JPEG_STATE_NOHEADER		0					//HEADER未读取,初始状态
#define JPEG_STATE_HEADEROK		1					//HEADER读取成功
#define JPEG_STATE_FINISHED		2					//解码完成
#define JPEG_STATE_ERROR		3					//解码错误

#define JPEG_YCBCR_COLORSPACE		JPEG_CONFR1_COLORSPACE_0
#define JPEG_CMYK_COLORSPACE		JPEG_CONFR1_COLORSPACE

//jpeg编解码控制结构体
typedef struct
{ 
	JPEG_ConfTypeDef	Conf;             			//当前JPEG文件相关参数
	jpeg_databuf_type inbuf[JPEG_DMA_INBUF_NB];		//DMA IN buf
	jpeg_databuf_type outbuf[JPEG_DMA_OUTBUF_NB];	//DMA OUT buf
	vu8 inbuf_read_ptr;								//DMA IN buf当前读取位置
	vu8 inbuf_write_ptr;							//DMA IN buf当前写入位置
	vu8 indma_pause;								//输入DMA暂停状态标识
	vu8 outbuf_read_ptr;							//DMA OUT buf当前读取位置
	vu8 outbuf_write_ptr;							//DMA OUT buf当前写入位置
	vu8 outdma_pause;								//输入DMA暂停状态标识
	vu8 state;										//解码状态;0,未识别到Header;1,识别到了Header;2,解码完成;
	u32 yuvblk_size;								//YUV输出的字节数,使得完成一次DMA2D YUV2RGB转换,刚好是图片宽度的整数倍
													//YUV420图片,每个像素占1.5个YUV字节,每次输出16行,yuvblk_size=图片宽度*16*1.5
													//YUV422图片,每个像素占2个YUV字节和RGB565一样,每次输出8行,yuvblk_size=图片宽度*8*2
													//YUV444图片,每个像素占3个YUV字节,每次输出8行,yuvblk_size=图片宽度*8*3
	
	u16 yuvblk_height;								//每个YUV块输出像素的高度,对于YUV420,为16,对于YUV422/YUV444为8	
	u16 yuvblk_curheight;							//当前输出高度,0~分辨率高度
}jpeg_codec_typedef;

//DMA回调函数
extern void (*jpeg_in_callback)(void);				//JPEG DMA输入回调函数
extern void (*jpeg_out_callback)(void);				//JPEG DMA输出 回调函数
extern void (*jpeg_eoc_callback)(void);				//JPEG 解码完成 回调函数
extern void (*jpeg_hdp_callback)(void);				//JPEG Header解码完成 回调函数


void JPEG_IN_DMA_Init(u32 meminaddr,u32 meminsize);
void JPEG_OUT_DMA_Init(u32 memoutaddr,u32 memoutsize);
u8 JPEG_Core_Init(jpeg_codec_typedef *tjpeg);
void JPEG_Core_Destroy(jpeg_codec_typedef *tjpeg);
void JPEG_Decode_Init(jpeg_codec_typedef *tjpeg);
void JPEG_IN_DMA_Start(void);
void JPEG_OUT_DMA_Start(void);
void JPEG_DMA_Stop(void);
void JPEG_IN_DMA_Resume(u32 memaddr,u32 memlen);
void JPEG_OUT_DMA_Resume(u32 memaddr,u32 memlen);
void JPEG_Get_Info(jpeg_codec_typedef *tjpeg);
u8 JPEG_Get_Quality(void);
u8 JPEG_DMA2D_YUV2RGB_Conversion(jpeg_codec_typedef *tjpeg,u32 *pdst);
#endif



