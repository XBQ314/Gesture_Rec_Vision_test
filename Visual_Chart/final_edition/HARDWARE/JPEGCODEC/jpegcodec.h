#ifndef __JPEGCODEC_H
#define __JPEGCODEC_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//JPEGӲ��������� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2018/8/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define JPEG_DMA_INBUF_LEN			4096			//����DMA IN  BUF�Ĵ�С 
#define JPEG_DMA_INBUF_NB			10				//DMA IN  BUF�ĸ���
#define JPEG_DMA_OUTBUF_NB			2				//DMA OUT BUF�ĸ���

//JPEG���ݻ���ṹ��
typedef struct
{
    u8 sta;			//״̬:0,������;1,������.
    u8 *buf;		//JPEG���ݻ�����
    u16 size; 		//JPEG���ݳ��� 
}jpeg_databuf_type; 

#define JPEG_STATE_NOHEADER		0					//HEADERδ��ȡ,��ʼ״̬
#define JPEG_STATE_HEADEROK		1					//HEADER��ȡ�ɹ�
#define JPEG_STATE_FINISHED		2					//�������
#define JPEG_STATE_ERROR		3					//�������

#define JPEG_YCBCR_COLORSPACE		JPEG_CONFR1_COLORSPACE_0
#define JPEG_CMYK_COLORSPACE		JPEG_CONFR1_COLORSPACE

//jpeg�������ƽṹ��
typedef struct
{ 
	JPEG_ConfTypeDef	Conf;             			//��ǰJPEG�ļ���ز���
	jpeg_databuf_type inbuf[JPEG_DMA_INBUF_NB];		//DMA IN buf
	jpeg_databuf_type outbuf[JPEG_DMA_OUTBUF_NB];	//DMA OUT buf
	vu8 inbuf_read_ptr;								//DMA IN buf��ǰ��ȡλ��
	vu8 inbuf_write_ptr;							//DMA IN buf��ǰд��λ��
	vu8 indma_pause;								//����DMA��ͣ״̬��ʶ
	vu8 outbuf_read_ptr;							//DMA OUT buf��ǰ��ȡλ��
	vu8 outbuf_write_ptr;							//DMA OUT buf��ǰд��λ��
	vu8 outdma_pause;								//����DMA��ͣ״̬��ʶ
	vu8 state;										//����״̬;0,δʶ��Header;1,ʶ����Header;2,�������;
	u32 yuvblk_size;								//YUV������ֽ���,ʹ�����һ��DMA2D YUV2RGBת��,�պ���ͼƬ��ȵ�������
													//YUV420ͼƬ,ÿ������ռ1.5��YUV�ֽ�,ÿ�����16��,yuvblk_size=ͼƬ���*16*1.5
													//YUV422ͼƬ,ÿ������ռ2��YUV�ֽں�RGB565һ��,ÿ�����8��,yuvblk_size=ͼƬ���*8*2
													//YUV444ͼƬ,ÿ������ռ3��YUV�ֽ�,ÿ�����8��,yuvblk_size=ͼƬ���*8*3
	
	u16 yuvblk_height;								//ÿ��YUV��������صĸ߶�,����YUV420,Ϊ16,����YUV422/YUV444Ϊ8	
	u16 yuvblk_curheight;							//��ǰ����߶�,0~�ֱ��ʸ߶�
}jpeg_codec_typedef;

//DMA�ص�����
extern void (*jpeg_in_callback)(void);				//JPEG DMA����ص�����
extern void (*jpeg_out_callback)(void);				//JPEG DMA��� �ص�����
extern void (*jpeg_eoc_callback)(void);				//JPEG ������� �ص�����
extern void (*jpeg_hdp_callback)(void);				//JPEG Header������� �ص�����


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



