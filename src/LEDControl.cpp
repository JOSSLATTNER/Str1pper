#include "LEDControl.h"

namespace LEDCNTRL
{
	//typedef std::function<void(void*)> ModuleLoop_t;
	//typedef void(*ModuleLoop_t)(void* p);
	LEDControl::LEDControl()
	{
		this->m_pStripChains.clear();
	}

	LEDControl::~LEDControl()
	{
	}

	void LEDControl::update()
	{
		/*Serial.println("Updateing LEDs!");*/

		for (uint32_t i = 0; i < this->m_pStripChains.size(); ++i)
		{
			for (uint32_t s = 0; s < this->m_pStripChains[i].pStrips.size(); ++s)
			{
				//this->m_pStripChains[i].pStrips[s].updateModul(&this->m_pStripChains[i].pStrips[s]);

				if (this->m_pStripChains[i].pStrips[s].pParam == nullptr)
					continue;

				switch (this->m_pStripChains[i].pStrips[s].updateModul)
				{
				case EModul::solidcolor:
					this->modul_Solidcolor(&this->m_pStripChains[i].pStrips[s]);
					break;
				}
			}
			updateLEDs(&this->m_pStripChains[i]);
		}
	}


	void LEDControl::setStripModul(EModul m, uint32_t ChainId, uint32_t StripId)
	{
		Serial.print("Set strip modul for chain[");
		Serial.print(ChainId);
		Serial.print("]");
		Serial.print(" strip[");
		Serial.print(StripId);
		Serial.println("]");

		strip_t* pStrip = &this->m_pStripChains[ChainId].pStrips[StripId];

		pStrip->updateModul = m;

		if (pStrip->pParam != nullptr)
			free(pStrip->pParam);

		size_t size = 0;
		switch (m)
		{
		case EModul::solidcolor:
			size = sizeof(modul_config_solidColor);
			break;
		}
		pStrip->pParam = malloc(size);
		memset(pStrip->pParam, 0, size);
	}

	void LEDControl::setChainModul(EModul m, uint32_t chainID)
	{
		Serial.print("Set Chainmodul for[");
		Serial.print(chainID);
		Serial.println("]");
		for (int i = 1; i < this->m_pStripChains[chainID].pStrips.size(); ++i)
		{
			this->m_pStripChains[chainID].pStrips[i].updateModul = EModul::none;
			free(this->m_pStripChains[chainID].pStrips[i].pParam);
		}

		this->setStripModul(m, chainID, 0);
	}

	void* LEDControl::getStripConfig(uint32_t ChainId, uint32_t stripId)
	{
		return this->m_pStripChains[ChainId].pStrips[stripId].pParam;
	}

	void LEDControl::modul_Solidcolor(strip_t* pStrip)
	{
		/*Serial.print("Run modul Solidcolor for Strip[");
		Serial.print(pStrip->id);
		Serial.println("]");*/
		
		modul_config_solidColor* cfg = static_cast<modul_config_solidColor*>(pStrip->pParam);

		if (cfg == nullptr)
			return;

		for (uint32_t i = 0; i < pStrip->numPixels; ++i)
		{
			pStrip->pixels[i] = cfg->solidColor;
		}
	}

	uint32_t LEDControl::createChain(stripchain_t chainDesc)
	{
		chainDesc.id = this->m_pStripChains.size();
		chainDesc.pWholePixels = nullptr;
		chainDesc.totalPixel = 0;

		this->m_pStripChains.push_back(chainDesc);

		this->appendStrip(chainDesc.id, {});

		Serial.print("Created Chain[");
		Serial.print(chainDesc.id);
		Serial.println("]");
		
		return chainDesc.id;
	}

	void LEDControl::initChain(uint32_t chainID)
	{
		Serial.print("Init Chain[");
		Serial.print(chainID);
		Serial.println("]");
		
		stripchain_t* pStrip = &this->m_pStripChains[chainID];

		m_pStripChains[chainID].pWholePixels = static_cast<pixelColor_t*>(malloc(m_pStripChains[chainID].totalPixel * sizeof(pixelColor_t)));
		memset(m_pStripChains[chainID].pWholePixels, 1, m_pStripChains[chainID].totalPixel * sizeof(pixelColor_t));

		// Itterate throug strips and init those
		pixelColor_t* se = m_pStripChains[chainID].pWholePixels;

		m_pStripChains[chainID].pStrips[0].pixels = se;
		m_pStripChains[chainID].pStrips[0].numPixels = m_pStripChains[chainID].totalPixel;

		for (uint32_t s = 1; s < m_pStripChains[chainID].pStrips.size(); ++s)
		{
			m_pStripChains[chainID].pStrips[s].pixels = se;
			memset(m_pStripChains[chainID].pStrips[s].pixels, 0, sizeof(pixelColor_t)*m_pStripChains[chainID].pStrips[s].numPixels);

			se += m_pStripChains[chainID].pStrips[s].numPixels;
			// if LEDCNT IS < 1 THROW ERRO
		}

		initDriver(chainID);

		m_pStripChains[chainID].init = true;
	}

	uint32_t LEDControl::appendStrip(uint32_t chainID, strip_t stripDesc)
	{
		stripDesc.id = this->m_pStripChains[chainID].pStrips.size();

		this->m_pStripChains[chainID].totalPixel += stripDesc.numPixels;

		this->m_pStripChains[chainID].pStrips.push_back(stripDesc);

		Serial.print("Created strip[");
		Serial.print(stripDesc.id);
		Serial.print("] on chain[");
		Serial.print(chainID);
		Serial.println("]");

		return stripDesc.id;
	}

	uint32_t LEDControl::getChainCount()
	{
		return this->m_pStripChains.size();
	}

	const stripchain_t& LEDControl::getChain(uint32_t chainID)
	{
		return this->m_pStripChains[chainID];
	}

	const strip_t&  LEDControl::getStrip(uint32_t chainID, uint32_t stripID)
	{
		return this->m_pStripChains[chainID].pStrips[stripID];
	}


	void LEDControl::updateLEDs(stripchain_t* pStripChain)
	{

		/*Serial.print("Updateing Chain[");
		Serial.print(pStripChain->id);
		Serial.println("]");*/

		WS2812b_leddriver_config_t* cfg = static_cast<WS2812b_leddriver_config_t*>(pStripChain->driver_config);

			for (uint16_t i = 0; i < pStripChain->totalPixel; i++) 
			{

				cfg->pBuffer[0 + i * 3] = pStripChain->pWholePixels[i].g;
				cfg->pBuffer[1 + i * 3] = pStripChain->pWholePixels[i].r;
				cfg->pBuffer[2 + i * 3] = pStripChain->pWholePixels[i].b;
				
				this->m_pDriverMap[pStripChain->ledType]->UpdateLEDS(pStripChain);
				/*Serial.print(i);
				Serial.print("/");
				Serial.println(pStripChain->totalPixel);
				Serial.print("R[");
				Serial.print(pStripChain->pWholePixels[i].r);
				Serial.print("] G[");
				Serial.print(pStripChain->pWholePixels[i].g);
				Serial.print("] B[");
				Serial.print(pStripChain->pWholePixels[i].b);
				Serial.println("]");*/
					

			}
		

		/*Serial.print("Finish Updateing Chain[");
		Serial.print(pStripChain->id);
		Serial.println("]");*/
	}

	void LEDControl::initDriver(uint32_t chainID)
	{
		stripchain_t* pStripChain = &this->m_pStripChains[chainID];

		switch(pStripChain->ledType)
		{
			case LED_types::LED_TYPE_WS2812b_V2 :
				pStripChain->driver_config = static_cast<WS2812b_leddriver_config_t*>(malloc(sizeof(WS2812b_leddriver_config_t)));
				WS2812b_leddriver_config_t* cfg = static_cast<WS2812b_leddriver_config_t*>(pStripChain->driver_config);


				cfg->gpioPin = pStripChain->gpioPin;
				cfg->rmtChannel = pStripChain->rmtChannel;
				cfg->buffer_len = pStripChain->totalPixel;
				cfg->pBuffer = static_cast<uint8_t*>(malloc(cfg->buffer_len));
				cfg->buffer_pos = 0;
				cfg->buffer_half = 0;
				cfg->buffer_isDirty = 0;
				

				cfg->pulsepair[0].level0 = 1;
				cfg->pulsepair[0].level1 = 0;
				cfg->pulsepair[0].duration0 = 400 / (DIVIDER * DURATION);
				cfg->pulsepair[0].duration1 = 850 / (DIVIDER * DURATION);
				
				cfg->pulsepair[1].level0 = 1;
				cfg->pulsepair[1].level1 = 0;
				cfg->pulsepair[1].duration0 = 850 / (DIVIDER * DURATION);
				cfg->pulsepair[1].duration1 = 400 / (DIVIDER * DURATION);
				
				if(this->m_pDriverMap[pStripChain->ledType] == nullptr)
					this->m_pDriverMap[pStripChain->ledType] = static_cast<LEDDriver*>(new WS2812bDriver());

				this->m_pDriverMap[pStripChain->ledType]->InitDriver(pStripChain);
				break;
		}
	}

}