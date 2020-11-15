/*
 * XBQreceive.c
 *
 *  Created on: Oct 21, 2020
 *      Author: xbq28
 */
#include "XBQreceive.h"

int HandResult;
uint8_t rec_buf[3];
uint8_t ch;

void HandRecByteGet(uint8_t bytedata)
{
	static uint8_t rec_sta;

	rec_buf[rec_sta] = bytedata;

	if(rec_sta == 0)
	{
		if(bytedata==0xaa)
		{
			rec_sta++;
		}
		else
		{
			rec_sta=0;
		}
	}
	else if(rec_sta == 1)
	{
		rec_sta++;
	}
	else if(rec_sta == 2)
	{
		if(bytedata==0x55)
		{
			HandRecDataAnal(rec_buf);
			rec_sta = 0;
		}
		else
		{
			rec_sta=0;
		}
	}
}

void HandRecDataAnal(uint8_t * rec_buf)
{
	if(*(rec_buf + 0)==0xaa && *(rec_buf + 2)==0x55)
	{
		HandResult =*(rec_buf + 1);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	HAL_UART_Transmit(&UART3_Handler, &ch, 1, HAL_MAX_DELAY);
	HandRecByteGet(ch);
	if(HandResult == 1)
	{
	}
	if(HandResult == 2)
	{
	}
	if(HandResult == 3)
	{
	}
}
