/*
 * MyNet.c
 *
 *  Created on: 2020骞�10鏈�16鏃�
 *      Author: xbq28
 */

#include "MyNetAPI.h"

_XBQ_HandRec XBQ_HandRec;

void InputGet()
{
	//鑾峰緱AMG883鍘熷鏁版嵁
	readPixelsRaw(pixelsRaw);
	//鎵╁ぇ涓�鍊�
	for (u8 i = 0; i < 64; i++)
	{
		pixelsRaw[i] = (pixelsRaw[i] - 80) << 2;
	}
	//(浼�)褰掍竴鍖栵紝骞惰祴鍊肩粰input
	for (u8 i = 0; i < 64; i++)
	{
		XBQ_HandRec.in_data[i] = (ai_float) pixelsRaw[i] / 255;
	}
}

void GetResult()
{
	u8 max_index = 0;
	float max_value = XBQ_HandRec.out_data[0];
	for(u8 i = 0; i < AI_NETWORK_OUT_1_SIZE; i++)
	{
		if(XBQ_HandRec.out_data[i] > max_value)
		{
			max_index = i;
			max_value = XBQ_HandRec.out_data[i];
		}
	}
	XBQ_HandRec.result = max_index;
}

void HandRecResultPrint()
{
	switch (XBQ_HandRec.result)
	{
	   case 0:
	   {
		   printf("nothing");
		   break;
	   }
	   case 1:
	   {
		   printf("up");
		   break;
	   }
	   case 2:
	   {
		   printf("down");
		   break;
	   }
	   case 3:
	   {
		   printf("left");
		   break;
	   }
	   case 4:
	   {
		   printf("right");
		   break;
	   }
	   default:
	   {
		   printf("error");
		   break;
	   }
	}
	printf("\r\n");
	return;
}
