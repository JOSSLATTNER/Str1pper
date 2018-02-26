#pragma once 

#include "pixelColor_t.h"
#include "Modules.h"

namespace LEDCNTRL
{
	typedef struct
	{
		int numPixels;
		int brightLimit;
		pixelColor_t * pixels;
		EModul updateModul;
		void* pParam;
		int id;
	} strip_t;
}