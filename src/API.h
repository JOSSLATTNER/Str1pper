#pragma once

#include "LEDControl.h"
#include <string>
#include <sstream>

namespace LEDCNTRL
{
	class API
	{
	private:
		LEDControl* m_pControll;

	public:
		API(LEDControl* pLEDControl);
		~API();

	public:
		std::string getChains();
		std::string getChain(int id);
		
		std::string getStrips(int chainID);
		std::string getStrip(int chainID, int stripID);
		
		std::string getModuls();
		std::string getStripModulConfig(uint32_t chainID, uint32_t stripID, EModul modulID);
		static std::string getModulName(uint32_t modulID);

		
		void setChainModul(EModul ModulID, uint32_t chainID);
		void setStripModul(EModul modulID, int chainID, int stripID);
		
		template<typename T>
		void setStripConfig(int chainID, int stripID, T const & cfg)
		{
			this->m_pControll->setStripConfig<T>(chainID, stripID, cfg);
		}

		void initChain(uint32_t chainID);
		uint32_t createChain(stripchain_t chainDesc);
		uint32_t appendStrip(int chainID, strip_t stripDesc);
	};
}