#pragma once 

#include "stripchain_t.h"
#include "strip_t.h"

namespace LEDCNTRL
{
	#define ETS_RMT_CTRL_INUM	18
	#define ESP_RMT_CTRL_DISABLE	ESP_RMT_CTRL_DIABLE /* Typo in esp_intr.h */

	#define DIVIDER		4 /* Above 4, timings start to deviate*/
	#define DURATION	12.5 /* minimum time of a single RMT duration in nanoseconds based on clock */
	#define MAX_PULSES  32 

	class LEDDriver
	{
	public:
		virtual ~LEDDriver(){};

	public:
		virtual void InitDriver(stripchain_t* pStripchain) = 0;
		virtual void UpdateLEDS(stripchain_t* pStripChain)  = 0;
	};
}