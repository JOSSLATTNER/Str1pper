#pragma once 

namespace LEDCNTRL
{
	typedef struct
	{
		int rmtChannel;
		int gpioNum;
		int ledType;
		std::vector<strip_t> pStrips;
		pixelColor_t* pWholePixels;
		void * _stateVars;
		int totalPixel;
		int id;
		bool init;
	}stripchain_t;
}
