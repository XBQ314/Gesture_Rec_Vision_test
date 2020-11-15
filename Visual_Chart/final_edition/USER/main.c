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

	int size=12;              //1~12表示视力5.1~4.0
	int dire=0;                  //0123表示上下左右，5表示未知
	int flag=0;               //0表示结果正确，1表示第一次出错
	int end=0;                //表示测试是否结束，1为结束
	int dict=0;                 //随机生成方向
	u32 time_us = 0;
	int i=0;

int main(void)
{
	Cache_Enable();                			//打开L1-Cache
	MPU_Memory_Protection();        		//保护相关存储区域
	HAL_Init();				        		//初始化HAL库
	Stm32_Clock_Init(160,5,2,4);  		    //设置时钟,400Mhz 
	delay_init(400);						//延时初始化
	uart_init(115200);						//串口初始化
	usmart_dev.init(200); 		    		//初始化USMART	
	LED_Init();								//初始化LED
	KEY_Init();								//初始化按键
	SDRAM_Init();                   		//初始化SDRAM
	LCD_Init();								//初始化LCD
    W25QXX_Init();				    		//初始化W25Q256
    WM8978_Init();				    		//初始化WM8978
	WM8978_HPvol_Set(40,40);	   			//耳机音量设置
	WM8978_SPKvol_Set(30);		    		//喇叭音量设置
    my_mem_init(SRAMIN);            		//初始化内部内存池
    my_mem_init(SRAMEX);            		//初始化外部SDRAM内存池
    my_mem_init(SRAMDTCM);          		//初始化内部DTCM内存池
    exfuns_init();		            		//为fatfs相关变量申请内存  
    f_mount(fs[0],"0:",1);          		//挂载SD卡 
 	f_mount(fs[1],"1:",1);          		//挂载SPI FLASH.   

	  while(RNG_Init())	 		    //初始化随机数发生器
	{
		LCD_ShowString(30,130,200,16,16," RNG Error! ");	 
		delay_ms(200);
		LCD_ShowString(30,130,200,16,16,"RNG Trying...");	 
	}
	while(font_init()) 			   		 	//检查字库
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//清除显示	     
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
		 dict=RNG_Get_RandomRange(0,3);              //随机产生方向
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
		 Display_RES(size);            //显示结果
		 my_play(size-1);
	   delay_ms(3000);	 
     LCD_Clear(WHITE);		
	}     		 
}

