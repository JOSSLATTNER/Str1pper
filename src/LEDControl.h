#pragma once 

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <functional>
#include <vector>
#include <map>

#include "strip_t.h"
#include "stripchain_t.h"
#include "pixelColor_t.h"
#include "Modules.h"
#include "LEDTypes.h"
#include "LEDDriver.h"
#include "driver_config_t.h"
#include "WS2812bDriver.h"

namespace LEDCNTRL
{



	class LEDControl
	{
	public:


		// Hold Driver for LEDs
		// Webserver
		// API
	private:
		std::vector<stripchain_t> m_pStripChains;
		std::map<LED_types,LEDDriver*> m_pDriverMap;

	public:
		LEDControl();
		~LEDControl();

	public:
		void update();



		void setChainModul(EModul m, uint32_t chainID);

		void setStripModul(EModul m, uint32_t ChainId, uint32_t StripId);

		template <typename T>
		void setStripConfig(uint32_t chainID, uint32_t stripID, T const & cfg)
		{
			*static_cast<T*>(this->m_pStripChains[chainID].pStrips[stripID].pParam) = cfg;
		};
		
		void* getStripConfig(uint32_t ChainId, uint32_t StripId);


		uint32_t createChain(stripchain_t chainDesc);
		void initChain(uint32_t chainID);
		uint32_t appendStrip(uint32_t chainID, strip_t stripDesc);

		

		uint32_t getChainCount();
		const stripchain_t& getChain(uint32_t chainID);
		const strip_t& getStrip(uint32_t chainID, uint32_t stripID);
		
		void modul_Solidcolor(strip_t* pStrip);
		void runDriver();

	private:
		void updateLEDs(stripchain_t* pStripChain);
		void initDriver(uint32_t chainID);

	};
}