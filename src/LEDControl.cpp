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

		for (int i = 0; i < this->m_pStripChains.size(); ++i)
		{
			for (int s = 0; s < this->m_pStripChains[i].pStrips.size(); ++s)
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


	void LEDControl::setStripModul(EModul m, int ChainId, int StripId)
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

	void LEDControl::setChainModul(EModul m, int chainID)
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

	void* LEDControl::getStripConfig(int ChainId, int stripId)
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

		for (int i = 0; i < pStrip->numPixels; ++i)
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

		for (int s = 1; s < m_pStripChains[chainID].pStrips.size(); ++s)
		{
			m_pStripChains[chainID].pStrips[s].pixels = se;
			memset(m_pStripChains[chainID].pStrips[s].pixels, 0, sizeof(pixelColor_t)*m_pStripChains[chainID].pStrips[s].numPixels);

			se += m_pStripChains[chainID].pStrips[s].numPixels;
			// if LEDCNT IS < 1 THROW ERRO
		}

		m_pStripChains[chainID].init = true;
	}

	uint32_t LEDControl::appendStrip(int chainID, strip_t stripDesc)
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

		
			for (uint16_t i = 0; i < pStripChain->totalPixel; i++) 
			{
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

		
}
