/*
 * XBQreceive.h
 *
 *  Created on: Oct 21, 2020
 *      Author: xbq28
 */

#ifndef SRC_XBQRECEIVE_H_
#define SRC_XBQRECEIVE_H_

#include "usart.h"
#include "led.h"
extern int HandResult;
extern uint8_t rec_buf[3];
extern uint8_t ch;

void HandRecByteGet(uint8_t bytedata);
void HandRecDataAnal(uint8_t * rec_buf);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle);
#endif /* SRC_XBQRECEIVE_H_ */
