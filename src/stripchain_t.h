#pragma once 
#include "LEDTypes.h"
#include "pixelColor_t.h"
#include "strip_t.h"
#include <vector>
namespace LEDCNTRL
{
	typedef struct
	{
		uint8_t rmtChannel;
		gpio_num_t gpioPin;
		LED_types ledType;
		std::vector<strip_t> pStrips;
		pixelColor_t* pWholePixels;
		void* driver_config;
		int totalPixel;
		int id;
		bool init;
	}stripchain_t;
}
