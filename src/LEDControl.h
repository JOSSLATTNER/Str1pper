#pragma once 

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <functional>
#include <vector>

#include "strip_t.h"
#include "stripchain_t.h"
#include "pixelColor_t.h"
#include "Modules.h"
#include "LEDTypes.h"

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

	public:
		LEDControl();
		~LEDControl();

	public:
		void update();



		void setChainModul(EModul m, int chainID);

		void setStripModul(EModul m, int ChainId, int StripId);

		template <typename T>
		void setStripConfig(int chainID, int stripID, T const & cfg)
		{
			*static_cast<T*>(this->m_pStripChains[chainID].pStrips[stripID].pParam) = cfg;
		};
		
		void* getStripConfig(int ChainId, int StripId);


		uint32_t createChain(stripchain_t chainDesc);
		void initChain(uint32_t chainID);
		uint32_t appendStrip(int chainID, strip_t stripDesc);


		uint32_t getChainCount();
		const stripchain_t& getChain(uint32_t chainID);
		const strip_t& getStrip(uint32_t chainID, uint32_t stripID);
		
		void modul_Solidcolor(strip_t* pStrip);


	private:
		void updateLEDs(stripchain_t* pStripChain);

	};
}