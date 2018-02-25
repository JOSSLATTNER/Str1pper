#pragma once 

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <functional>
#include <vector>




namespace LEDCNTRL
{
	typedef union {
		struct {
			uint8_t r, g, b, w;
		};
		uint32_t num;
	} pixelColor_t;


	inline pixelColor_t pixelFromRGB(uint8_t r, uint8_t g, uint8_t b)
	{
		pixelColor_t v;
		v.r = r;
		v.g = g;
		v.b = b;
		v.w = 0;
		return v;
	}

	inline pixelColor_t pixelFromRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
	{
		pixelColor_t v;
		v.r = r;
		v.g = g;
		v.b = b;
		v.w = w;
		return v;
	}

	enum LED_types
	{
		LED_TYPE_WS2812b_V2
	};

	enum EModul
	{
		none = 0,
		solidcolor = 1
	};

	typedef struct
	{
		pixelColor_t solidColor;
	} modul_config_solidColor;

	typedef struct
	{
		int numPixels;
		int brightLimit;
		pixelColor_t * pixels;
		EModul updateModul;
		void* pParam;
		int id;
	} strand_t;

	typedef struct
	{
		int rmtChannel;
		int gpioNum;
		int ledType;
		std::vector<strand_t> pStrands;
		pixelColor_t* pWholePixels;
		void * _stateVars;
		int totalPixel;
		int id;
		bool init;
	}strandchain_t;



	class LEDControl
	{
	public:


		// Hold Driver for LEDs
		// Webserver
		// API
	private:
		std::vector<strandchain_t> m_pStrandChains;

	public:
		LEDControl();
		~LEDControl();

	public:
		void update();

		void setStrandModul(EModul m, int ChainId, int StrandId);
		void setChainModul(EModul m, int chainID);

		template <typename T>
		void setStrandConfig(int chainID, int strandID, T const & cfg)
		{
			*static_cast<T*>(this->m_pStrandChains[chainID].pStrands[strandID].pParam) = cfg;
		};

		const void* getStrandConfig(int ChainId, int StrandId);

		void modul_Solidcolor(strand_t* pStrand);

		uint32_t createChain(strandchain_t chainDesc);
		void initChain(uint32_t chainID);
		uint32_t appendStrand(int chainID, strand_t strandDesc);


		uint32_t getChainCount();
		const strandchain_t& getChain(uint32_t chainID);
		const strand_t& getStrand(uint32_t chainID, uint32_t strandID);



	private:
		void updateLEDs(strandchain_t* pStrandChain);

	};
}