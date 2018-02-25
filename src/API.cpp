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

		const stripchain_t& chain = this->m_pControll->getChain(chainID);

		ss << "{";
		ss << "\"chainid\":\""  << chain.id <<   "\",";
		ss << "\"gpiopin\":\""  << chain.gpioNum <<   "\",";
		ss << "\"ledtype\":\""  << chain.ledType <<   "\",";
		ss << "\"rmtchannel\":\""  << chain.rmtChannel <<   "\",";
		ss << "\"totalpixel\":\""  << chain.totalPixel <<   "\",";
		ss << "\"isinit\":\""  << chain.init <<   "\",";
		ss << "\"strip\":" + getStrips(chainID) ;
		ss << "}";

		return ss.str();
	}

	std::string  API::getStrips(int chainID)
	{
		std::stringstream ss;
		ss << "{";

		ss << "\"stripcount\":\""  << this->m_pControll->getChain(chainID).pStrips.size() <<   "\",";
		ss << "\"strip\":";
		ss << "[";

		for (uint32_t i = 0; i < this->m_pControll->getChain(chainID).pStrips.size(); ++i)
		{
			ss << getStrip(chainID, i);

			if(i<this->m_pControll->getChain(chainID).pStrips.size()-1)
				ss << ",";
		}
		ss << "]";
		ss << "}";
		
		return ss.str();
	}

	std::string  API::getStrip(int chainID, int stripID)
	{
		std::stringstream ss;

		ss << "{";

		const strip_t& strip = this->m_pControll->getStrip(chainID,stripID);

		ss << "\"stripid\":\""  << strip.id <<   "\",";
		ss << "\"brightlimit\":\""  << strip.brightLimit <<   "\",";
		ss << "\"pixelcount\":\""  << strip.numPixels <<   "\",";
		ss << "\"activemodul\":\"" + getModulName(strip.updateModul) <<  "\"";
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

	std::string API::getStripModulConfig(uint32_t chainID, uint32_t stripID, EModul modulID)
	{
		std::stringstream ss;
		const modul_config_solidColor* pcfg = nullptr;
		switch (modulID)
		{
			case EModul::solidcolor:
				pcfg = static_cast<const modul_config_solidColor*>(this->m_pControll->getStripConfig(chainID, stripID));
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

	void API::setStripModul( EModul modulID, int chainID, int stripID)
	{
		this->m_pControll->setStripModul(modulID, chainID, stripID);
	}


	void API::initChain(uint32_t chainID)
	{
		this->m_pControll->initChain(chainID);
	}

	uint32_t API::createChain(stripchain_t chainDesc)
	{
		return this->m_pControll->createChain(chainDesc);
	}

	uint32_t API::appendStrip(int chainID, strip_t stripDesc)
	{
		return this->m_pControll->appendStrip(chainID, stripDesc);
	}

}