#include "audioplay.h"
#include "ff.h"
#include "malloc.h"
#include "usart.h"
#include "wm8978.h"
#include "sai.h"
#include "led.h"
#include "lcd.h"
#include "delay.h"
#include "key.h"
#include "exfuns.h"  
#include "text.h"
#include "string.h"  
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
__audiodev audiodev;	  
 

//��ʼ��Ƶ����
void audio_start(void)
{
	audiodev.status=3<<0;//��ʼ����+����ͣ
	SAI_Play_Start();
} 

//�ر���Ƶ����
void audio_stop(void)
{
	audiodev.status=0;
	SAI_Play_Stop();
} 

//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 audio_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO* tfileinfo;	//��ʱ�ļ���Ϣ	 	
	tfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//�����ڴ�
    res=f_opendir(&tdir,(const TCHAR*)path); //��Ŀ¼ 
	if(res==FR_OK&&tfileinfo)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,tfileinfo);       			//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo->fname[0]==0)break;	//������/��ĩβ��,�˳�	 		 
			res=f_typetell((u8*)tfileinfo->fname);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	}  
	myfree(SRAMIN,tfileinfo);//�ͷ��ڴ�
	return rval;
}

//��ʾ��Ŀ����
//index:��ǰ����
//total:���ļ���
void audio_index_show(u16 index,u16 total)
{
	//��ʾ��ǰ��Ŀ������,������Ŀ��
	LCD_ShowxNum(60+0,230,index,3,16,0X80);		//����
	LCD_ShowChar(60+24,230,'/',16,0);
	LCD_ShowxNum(60+32,230,total,3,16,0X80); 	//����Ŀ				  	  
}
 
//��ʾ����ʱ��,������ ��Ϣ  
//totsec;��Ƶ�ļ���ʱ�䳤��
//cursec:��ǰ����ʱ��
//bitrate:������(λ��)
void audio_msg_show(u32 totsec,u32 cursec,u32 bitrate)
{	
	static u16 playtime=0XFFFF;//����ʱ����	      
	if(playtime!=cursec)					//��Ҫ������ʾʱ��
	{
		playtime=cursec;
		//��ʾ����ʱ��			 
		LCD_ShowxNum(60,210,playtime/60,2,16,0X80);		//����
		LCD_ShowChar(60+16,210,':',16,0);
		LCD_ShowxNum(60+24,210,playtime%60,2,16,0X80);	//����		
 		LCD_ShowChar(60+40,210,'/',16,0); 	    	 
		//��ʾ��ʱ��    	   
 		LCD_ShowxNum(60+48,210,totsec/60,2,16,0X80);	//����
		LCD_ShowChar(60+64,210,':',16,0);
		LCD_ShowxNum(60+72,210,totsec%60,2,16,0X80);	//����	  		    
		//��ʾλ��			   
   		LCD_ShowxNum(60+110,210,bitrate/1000,4,16,0X80);//��ʾλ��	 
		LCD_ShowString(60+110+32,210,200,16,16,"Kbps");	 
	} 		 
}
//��������
void audio_play(int i)
{
	u8 res;
 	DIR wavdir;	 		//Ŀ¼
	FILINFO *wavfileinfo;//�ļ���Ϣ 
	u8 *pname;			//��·�����ļ���
	u16 totwavnum; 		//�����ļ�����
	u16 curindex;		//��ǰ����
	u8 key;				//��ֵ		  
 	u32 temp;
	u32 *wavoffsettbl;	//����offset������
    
	WM8978_ADDA_Cfg(1,0);	//����DAC
	WM8978_Input_Cfg(0,0,0);//�ر�����ͨ��
	WM8978_Output_Cfg(1,0);	//����DAC���   
	
 	while(f_opendir(&wavdir,"0:/MUSIC"))//�������ļ���
 	{	    
		Show_Str(60,190,240,16,"MUSIC�ļ��д���!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,206,WHITE);//�����ʾ	     
		delay_ms(200);				  
	} 									  
	totwavnum=audio_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
  	while(totwavnum==NULL)//�����ļ�����Ϊ0		
 	{	    
		Show_Str(60,190,240,16,"û�������ļ�!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,146,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}										   
	wavfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));	//�����ڴ�
  	pname=mymalloc(SRAMIN,FF_MAX_LFN*2+1);					//Ϊ��·�����ļ��������ڴ�
 	wavoffsettbl=mymalloc(SRAMIN,4*totwavnum);				//����4*totwavnum���ֽڵ��ڴ�,���ڴ�������ļ�off block����
 	while(!wavfileinfo||!pname||!wavoffsettbl)//�ڴ�������
 	{	    
		Show_Str(60,190,240,16,"�ڴ����ʧ��!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,146,WHITE);//�����ʾ	     
		delay_ms(200);				  
	} 	 
 	//��¼����
    res=f_opendir(&wavdir,"0:/MUSIC"); //��Ŀ¼
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{						
			temp=wavdir.dptr;								//��¼��ǰindex 
	        res=f_readdir(&wavdir,wavfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||wavfileinfo->fname[0]==0)break;	//������/��ĩβ��,�˳� 		 
			res=f_typetell((u8*)wavfileinfo->fname);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				wavoffsettbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
   	curindex=i;											//��0��ʼ��ʾ
   	res=f_opendir(&wavdir,(const TCHAR*)"0:/MUSIC"); 	//��Ŀ¼
	while(res==FR_OK)//�򿪳ɹ�
	{	
		dir_sdi(&wavdir,wavoffsettbl[curindex]);				//�ı䵱ǰĿ¼����	    
		strcpy((char*)pname,"0:/MUSIC/");						//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)wavfileinfo->fname);	//���ļ������ں���
		key=audio_play_song(pname); 			 		//���������Ƶ�ļ�
 
	} 											  
	myfree(SRAMIN,wavfileinfo);			//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�			    
	myfree(SRAMIN,wavoffsettbl);		//�ͷ��ڴ�	 
} 
//����ĳ����Ƶ�ļ�
u8 audio_play_song(u8* fname)
{
		WM8978_ADDA_Cfg(1,0);	//����DAC
	WM8978_Input_Cfg(0,0,0);//�ر�����ͨ��
	WM8978_Output_Cfg(1,0);	//����DAC���   
	u8 res;  
	res=f_typetell(fname); 
	switch(res)
	{
		case T_WAV:
			res=wav_play_song(fname);
			break;
		default://�����ļ�,�Զ���ת����һ��
			printf("can't play:%s\r\n",fname);
			res=KEY0_PRES;
			break;
	}
	return res;
}

void my_play(int i)
{
		switch(i){
			case 0:audio_play_song((u8*)"0:/MUSIC/0.WAV");break;
			case 1:audio_play_song((u8*)"0:/MUSIC/1.WAV");break;
			case 2:audio_play_song((u8*)"0:/MUSIC/2.WAV");break;
			case 3:audio_play_song((u8*)"0:/MUSIC/3.WAV");break;
			case 4:audio_play_song((u8*)"0:/MUSIC/4.WAV");break;
			case 5:audio_play_song((u8*)"0:/MUSIC/5.WAV");break;
			case 6:audio_play_song((u8*)"0:/MUSIC/6.WAV");break;
			case 7:audio_play_song((u8*)"0:/MUSIC/7.WAV");break;
			case 8:audio_play_song((u8*)"0:/MUSIC/8.WAV");break;
			case 9:audio_play_song((u8*)"0:/MUSIC/9.WAV");break;
			case 10:audio_play_song((u8*)"0:/MUSIC/10.WAV");break;
			case 11:audio_play_song((u8*)"0:/MUSIC/11.WAV");break;
			case 12:audio_play_song((u8*)"0:/MUSIC/12.WAV");break;
			case 13:audio_play_song((u8*)"0:/MUSIC/13.WAV");break;		
		}
			
}

























