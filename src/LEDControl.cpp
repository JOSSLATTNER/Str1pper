#include "LEDControl.h"

namespace LEDCNTRL
{
	//typedef std::function<void(void*)> ModuleLoop_t;
	//typedef void(*ModuleLoop_t)(void* p);
	LEDControl::LEDControl()
	{
		this->m_pStrandChains.clear();
	}

	LEDControl::~LEDControl()
	{
	}

	void LEDControl::update()
	{
		Serial.println("Updateing LEDs!");

		for (int i = 0; i < this->m_pStrandChains.size(); ++i)
		{
			for (int s = 0; s < this->m_pStrandChains[i].pStrands.size(); ++s)
			{
				//this->m_pStrandChains[i].pStrands[s].updateModul(&this->m_pStrandChains[i].pStrands[s]);

				if (this->m_pStrandChains[i].pStrands[s].pParam == nullptr)
					continue;

				switch (this->m_pStrandChains[i].pStrands[s].updateModul)
				{
				case EModul::solidcolor:
					this->modul_Solidcolor(&this->m_pStrandChains[i].pStrands[s]);
					break;
				}
			}
			updateLEDs(&this->m_pStrandChains[i]);
		}
	}


	void LEDControl::setStrandModul(EModul m, int ChainId, int StrandId)
	{
		Serial.print("Set strand modul for chain[");
		Serial.print(ChainId);
		Serial.print("]");
		Serial.print(" strand[");
		Serial.print(StrandId);
		Serial.println("]");

		strand_t* pStrand = &this->m_pStrandChains[ChainId].pStrands[StrandId];

		pStrand->updateModul = m;

		if (pStrand->pParam != nullptr)
			free(pStrand->pParam);

		size_t size = 0;
		switch (m)
		{
		case EModul::solidcolor:
			size = sizeof(modul_config_solidColor);
			break;
		}
		pStrand->pParam = malloc(size);
		memset(pStrand->pParam, 0, size);
	}

	void LEDControl::setChainModul(EModul m, int chainID)
	{
		Serial.print("Set Chainmodul for[");
		Serial.print(chainID);
		Serial.println("]");
		for (int i = 1; i < this->m_pStrandChains[chainID].pStrands.size(); ++i)
		{
			this->m_pStrandChains[chainID].pStrands[i].updateModul = EModul::none;
			free(this->m_pStrandChains[chainID].pStrands[i].pParam);
		}

		this->setStrandModul(m, chainID, 0);
	}

	const void* LEDControl::getStrandConfig(int ChainId, int StrandId)
	{
		return this->m_pStrandChains[ChainId].pStrands[StrandId].pParam;
	}

	void LEDControl::modul_Solidcolor(strand_t* pStrand)
	{
		Serial.print("Run modul Solidcolor for Strand[");
		Serial.print(pStrand->id);
		Serial.println("]");
		
		modul_config_solidColor* cfg = static_cast<modul_config_solidColor*>(pStrand->pParam);

		if (cfg == nullptr)
			return;

		for (int i = 0; i < pStrand->numPixels; ++i)
		{
			pStrand->pixels[i] = cfg->solidColor;
		}
	}

	uint32_t LEDControl::createChain(strandchain_t chainDesc)
	{


		chainDesc.id = this->m_pStrandChains.size();
		chainDesc.pWholePixels = nullptr;
		chainDesc.totalPixel = 0;

		this->m_pStrandChains.push_back(chainDesc);

		this->appendStrand(chainDesc.id, {});

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
		
		strandchain_t* pStrand = &this->m_pStrandChains[chainID];

		m_pStrandChains[chainID].pWholePixels = static_cast<pixelColor_t*>(malloc(m_pStrandChains[chainID].totalPixel * sizeof(pixelColor_t)));
		memset(m_pStrandChains[chainID].pWholePixels, 1, m_pStrandChains[chainID].totalPixel * sizeof(pixelColor_t));

		// Itterate throug strands and init those
		pixelColor_t* se = m_pStrandChains[chainID].pWholePixels;

		m_pStrandChains[chainID].pStrands[0].pixels = se;
		m_pStrandChains[chainID].pStrands[0].numPixels = m_pStrandChains[chainID].totalPixel;

		for (int s = 1; s < m_pStrandChains[chainID].pStrands.size(); ++s)
		{
			m_pStrandChains[chainID].pStrands[s].pixels = se;
			memset(m_pStrandChains[chainID].pStrands[s].pixels, 0, sizeof(pixelColor_t)*m_pStrandChains[chainID].pStrands[s].numPixels);

			se += m_pStrandChains[chainID].pStrands[s].numPixels;
			// if LEDCNT IS < 1 THROW ERRO
		}

		m_pStrandChains[chainID].init = true;
	}

	uint32_t LEDControl::appendStrand(int chainID, strand_t strandDesc)
	{
		strandDesc.id = this->m_pStrandChains[chainID].pStrands.size();

		this->m_pStrandChains[chainID].totalPixel += strandDesc.numPixels;

		this->m_pStrandChains[chainID].pStrands.push_back(strandDesc);

		Serial.print("Created strand[");
		Serial.print(strandDesc.id);
		Serial.print("] on chain[");
		Serial.print(chainID);
		Serial.println("]");

		return strandDesc.id;
	}

	uint32_t LEDControl::getChainCount()
	{
		return this->m_pStrandChains.size();
	}

	const strandchain_t& LEDControl::getChain(uint32_t chainID)
	{
		return this->m_pStrandChains[chainID];
	}

	const strand_t&  LEDControl::getStrand(uint32_t chainID, uint32_t strandID)
	{
		return this->m_pStrandChains[chainID].pStrands[strandID];
	}


	void LEDControl::updateLEDs(strandchain_t* pStrandChain)
	{

		Serial.print("Updateing Chain[");
		Serial.print(pStrandChain->id);
		Serial.println("]");

		
			for (uint16_t i = 0; i < pStrandChain->totalPixel; i++) 
			{
				Serial.print(i);
				Serial.print("/");
				Serial.println(pStrandChain->totalPixel);
				Serial.print("R[");
				Serial.print(pStrandChain->pWholePixels[i].r);
				Serial.print("] G[");
				Serial.print(pStrandChain->pWholePixels[i].g);
				Serial.print("] B[");
				Serial.print(pStrandChain->pWholePixels[i].b);
				Serial.println("]");
					

			}
		

		Serial.print("Finish Updateing Chain[");
		Serial.print(pStrandChain->id);
		Serial.println("]");
	}

		
}
