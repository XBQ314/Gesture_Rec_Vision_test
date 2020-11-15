#ifndef __AUDIOPLAY_H
#define __AUDIOPLAY_H
#include "sys.h"
#include "ff.h"
#include "wavplay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//���ֲ����� Ӧ�ô���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/18
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
 

//���ֲ��ſ�����
typedef __packed struct
{  
	//2��SAI�����BUF
	u8 *saibuf1;
	u8 *saibuf2; 
	u8 *tbuf;				//��ʱ����,����24bit�����ʱ����Ҫ�õ�
	FIL *file;				//��Ƶ�ļ�ָ��
	
	u8 status;				//bit0:0,��ͣ����;1,��������
							//bit1:0,��������;1,�������� 
}__audiodev; 
extern __audiodev audiodev;	//���ֲ��ſ�����


void wav_sai_dma_callback(void);

void audio_start(void);
void audio_stop(void);
u16 audio_get_tnum(u8 *path);
void audio_index_show(u16 index,u16 total);
void audio_msg_show(u32 totsec,u32 cursec,u32 bitrate);
void audio_play(int i);
u8 audio_play_song(u8* fname);
void my_play(int i);
 
#endif












