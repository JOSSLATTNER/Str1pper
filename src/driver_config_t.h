#pragma once

#include "rmt_pulsepair_t.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>


namespace LEDCNTRL
{
	typedef struct
	{
		uint16_t gpioPin;
		uint16_t rmtChannel;
		uint8_t* pBuffer;
		uint16_t buffer_len;
		uint16_t buffer_pos;
		uint16_t buffer_half;
		uint16_t buffer_isDirty;
		rmt_pulsepair_t pulsepair[2];
		xSemaphoreHandle semaphore;
	}WS2812b_leddriver_config_t; 
}