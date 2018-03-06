#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "soc/i2s_struct.h"

#include "LEDDriver.h"

namespace LEDCNTRL
{
	class I2C_shiftoutDriver : LEDDriver
	{
	public:
		I2C_shiftoutDriver();

		~I2C_shiftoutDriver();
		
	public:
		void InitDriver(stripchain_t* pStripchain);
		void UpdateLEDS(stripchain_t* pStripChain);
	};
}