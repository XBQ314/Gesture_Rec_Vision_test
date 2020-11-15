/*
 * MyNet.h
 *
 *  Created on: 2020年10月16日
 *      Author: xbq28
 */

#ifndef MYNETAPI_H_
#define MYNETAPI_H_

#include "network.h"
#include "sys.h"
#include "usart.h"
#include "ai_platform.h"
#include "network.h"
#include "Adafruit_AMG88xx.h"

typedef struct
{
	ai_float in_data[AI_NETWORK_IN_1_SIZE];
	ai_float out_data[AI_NETWORK_OUT_1_SIZE];
	ai_u8 result;
}_XBQ_HandRec;

extern _XBQ_HandRec XBQ_HandRec;

void InputGet();
void GetResult();
void HandRecResultPrint();
#endif /* MYNET_H_ */
