#pragma once 

namespace LEDCNTRL
{
	enum EModul
	{
		none = 0,
		solidcolor = 1
	};

	typedef struct
	{
		pixelColor_t solidColor;
	} modul_config_solidColor;
}