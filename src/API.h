#pragma once

#include "LEDControll.h"
#include <string>
#include <sstream>

namespace LEDCNTRL
{
	class API
	{
	private:
		LEDControll* m_pControll;

	public:
		API(LEDControll* pLEDControll);
		~API();

	public:
		std::string getChains();
		std::string getChain(int id);
		
		std::string getStrands(int chainID);
		std::string getStrand(int chainID, int strandID);
		
		std::string getModuls();
		std::string getStrandModulConfig(uint32_t chainID, uint32_t strandID, EModul modulID);
		static std::string getModulName(uint32_t modulID);

		
		void setChainModul(EModul ModulID, uint32_t chainID);
		void setStrandModul(EModul modulID, int chainID, int strandID);
		
		template<typename T>
		void setStrandConfig(int chainID, int strandID, T const & cfg)
		{
			this->m_pControll->setStrandConfig<T>(chainID, strandID, cfg);
		}

		template <typename T>
		void setStrandConfig(T const& cfg, int chainID, int strandID)
		{
			this->m_pControll->setStrandConfig(chainID, strandID, cfg);
		}

		void initChain(uint32_t chainID);
		uint32_t createChain(strandchain_t chainDesc);
		uint32_t appendStrand(int chainID, strand_t strandDesc);
	};
}