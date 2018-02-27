#pragma once 

#include "pixelColor_t.h"
#include "Modules.h"

namespace LEDCNTRL
{
	typedef struct
	{
		int numPixels;
		float brightLimit;
		pixelColor_t * pixels;
		EModul updateModul;
		void* pParam;
		int id;
	} strip_t;
}