#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h"
#include "key.h"
#include "ltdc.h"
#include "lcd.h"
#include "sdram.h"
#include "usmart.h"
#include "pcf8574.h"
#include "mpu.h"
#include "malloc.h"
#include "w25qxx.h"
#include "sdmmc_sdcard.h"
#include "nand.h"    
#include "ftl.h"  
#include "ff.h"
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"
#include "wm8978.h"	 
#include "audioplay.h"
#include "rng.h"
#include "XBQreceive.h"
extern const u8 gImage_E511[];

	int size=12;              //1~12��ʾ����5.1~4.0
	int dire=0;                  //0123��ʾ�������ң�5��ʾδ֪
	int flag=0;               //0��ʾ�����ȷ��1��ʾ��һ�γ���
	int end=0;                //��ʾ�����Ƿ������1Ϊ����
	int dict=0;                 //������ɷ���
	u32 time_us = 0;
	int i=0;

int main(void)
{
	Cache_Enable();                			//��L1-Cache
	MPU_Memory_Protection();        		//������ش洢����
	HAL_Init();				        		//��ʼ��HAL��
	Stm32_Clock_Init(160,5,2,4);  		    //����ʱ��,400Mhz 
	delay_init(400);						//��ʱ��ʼ��
	uart_init(115200);						//���ڳ�ʼ��
	usmart_dev.init(200); 		    		//��ʼ��USMART	
	LED_Init();								//��ʼ��LED
	KEY_Init();								//��ʼ������
	SDRAM_Init();                   		//��ʼ��SDRAM
	LCD_Init();								//��ʼ��LCD
    W25QXX_Init();				    		//��ʼ��W25Q256
    WM8978_Init();				    		//��ʼ��WM8978
	WM8978_HPvol_Set(40,40);	   			//������������
	WM8978_SPKvol_Set(30);		    		//������������
    my_mem_init(SRAMIN);            		//��ʼ���ڲ��ڴ��
    my_mem_init(SRAMEX);            		//��ʼ���ⲿSDRAM�ڴ��
    my_mem_init(SRAMDTCM);          		//��ʼ���ڲ�DTCM�ڴ��
    exfuns_init();		            		//Ϊfatfs��ر��������ڴ�  
    f_mount(fs[0],"0:",1);          		//����SD�� 
 	f_mount(fs[1],"1:",1);          		//����SPI FLASH.   

	  while(RNG_Init())	 		    //��ʼ�������������
	{
		LCD_ShowString(30,130,200,16,16," RNG Error! ");	 
		delay_ms(200);
		LCD_ShowString(30,130,200,16,16,"RNG Trying...");	 
	}
	while(font_init()) 			   		 	//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  	 
	POINT_COLOR=RED;      

while(1)
    {		
			size=12;
			flag=0;
			end=0;
			i=0;
		LCD_ShowString(150,0,300,32,32,"Start Testing");
	   my_play(12);
		delay_ms(1000);	
		LCD_Clear(WHITE);
    while(!end)
		{
		 dict=RNG_Get_RandomRange(0,3);              //�����������
		 Display_E(size,dict);		
	 	 delay_ms(1000);
		 for(i=0;i<10;)
			{
			 if(HandResult==dire)
			 {
			  i++;
			 } else 
			 {
			  dire=HandResult;
				i=0;
			 }
			 delay_ms(100);
			}
			if(dire==0) dire=5;
			dire=dire-1;
			if(dire==5)
			{
			  LCD_ShowString(170,0,300,32,32,"Try Again");
				continue;
			}
		 if(dire==dict) 
		 { 
			 flag=0;
		   if(size>1)size--;
			 else end=1;
		 } else
		 {
		  if(flag==0) flag=1;
			else end=1;
		 }
     LCD_Clear(WHITE);		 
		 delay_ms(1000);	
		}
		 Image_display(0,0,(u8*)gImage_E511,0);
		 LCD_Clear(WHITE);
		 Display_RES(size);            //��ʾ���
		 my_play(size-1);
	   delay_ms(3000);	 
     LCD_Clear(WHITE);		
	}     		 
}

