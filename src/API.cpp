#include "API.h"

namespace LEDCNTRL
{

	API::API(LEDControl * pLEDControl):
		m_pControll(pLEDControl)
	{
	}

	API::~API()
	{
	}

	std::string  API::getChains()
	{
		uint32_t size = this->m_pControll->getChainCount();

		std::stringstream ss;
		ss << "[";

		for (uint32_t i = 0; i < size; ++i)
		{
			ss << this->getChain(i);
			if (i<this->m_pControll->getChainCount() - 1)
				ss << ",";
		}

		ss << "]";
		return ss.str();
	}

	std::string  API::getChain(int chainID)
	{
		std::stringstream ss;

		const strandchain_t& chain = this->m_pControll->getChain(chainID);

		ss << "{";
		ss << "\"chainid\":\""  << chain.id <<   "\",";
		ss << "\"gpiopin\":\""  << chain.gpioNum <<   "\",";
		ss << "\"ledtype\":\""  << chain.ledType <<   "\",";
		ss << "\"rmtchannel\":\""  << chain.rmtChannel <<   "\",";
		ss << "\"totalpixel\":\""  << chain.totalPixel <<   "\",";
		ss << "\"isinit\":\""  << chain.init <<   "\",";
		ss << "\"strand\":" + getStrands(chainID) ;
		ss << "}";

		return ss.str();
	}

	std::string  API::getStrands(int chainID)
	{
		std::stringstream ss;
		ss << "{";

		ss << "\"strandcount\":\""  << this->m_pControll->getChain(chainID).pStrands.size() <<   "\",";
		ss << "\"strands\":";
		ss << "[";

		for (uint32_t i = 0; i < this->m_pControll->getChain(chainID).pStrands.size(); ++i)
		{
			ss << getStrand(chainID, i);

			if(i<this->m_pControll->getChain(chainID).pStrands.size()-1)
				ss << ",";
		}
		ss << "]";
		ss << "}";
		
		return ss.str();
	}

	std::string  API::getStrand(int chainID, int strandID)
	{
		std::stringstream ss;

		ss << "{";

		const strand_t& strand = this->m_pControll->getStrand(chainID,strandID);

		ss << "\"strandid\":\""  << strand.id <<   "\",";
		ss << "\"brightlimit\":\""  << strand.brightLimit <<   "\",";
		ss << "\"pixelcount\":\""  << strand.numPixels <<   "\",";
		ss << "\"activemodul\":\"" + getModulName(strand.updateModul) <<  "\"";
		ss << "}";

		return ss.str();
	}

	std::string  API::getModuls()
	{
		std::stringstream ss;

		ss << "[";

		ss << "\"solidcolor\":\"" << EModul::solidcolor << "\"";
		ss << "]";

		return ss.str();
	}

	std::string API::getStrandModulConfig(uint32_t chainID, uint32_t strandID, EModul modulID)
	{
		std::stringstream ss;
		const modul_config_solidColor* pcfg = nullptr;
		switch (modulID)
		{
			case EModul::solidcolor:
				pcfg = static_cast<const modul_config_solidColor*>(this->m_pControll->getStrandConfig(chainID, strandID));
				ss << "{";
				ss << "\"solidColor\": ["  << pcfg->solidColor.r <<   "," << pcfg->solidColor.g <<   ","  << pcfg->solidColor.b <<   "," << "]";
				ss << "}";
				break;

			case EModul::none:
				ss << "{";
				ss << "\"None\": None";
				ss << "}";
				break;
			default:
				break;
		}

		return	ss.str();
	}

	std::string API::getModulName(uint32_t modulID)
	{
		switch (modulID)
		{
		case EModul::solidcolor:
			return "solidcolor";
		case EModul::none:
			return "None";
		}

		return std::string();
	}

	void API::setChainModul( EModul ModulID, uint32_t chainID)
	{
		this->m_pControll->setChainModul(ModulID, chainID);
	}

	void API::setStrandModul( EModul modulID, int chainID, int strandID)
	{
		this->m_pControll->setStrandModul(modulID, chainID, strandID);
	}


	void API::initChain(uint32_t chainID)
	{
		this->m_pControll->initChain(chainID);
	}

	uint32_t API::createChain(strandchain_t chainDesc)
	{
		return this->m_pControll->createChain(chainDesc);
	}

	uint32_t API::appendStrand(int chainID, strand_t strandDesc)
	{
		return this->m_pControll->appendStrand(chainID, strandDesc);
	}

}