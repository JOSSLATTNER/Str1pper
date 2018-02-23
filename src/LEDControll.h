#pragma once 

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <functional>
#include <vector>

#ifdef __cplusplus
	extern "C" {
	#endif

	#if defined(ARDUINO)
	  #include "esp32-hal.h"
	  #include "esp_intr.h"
	  #include "driver/gpio.h"
	  #include "driver/rmt.h"
	  #include "driver/periph_ctrl.h"
	  #include "freertos/semphr.h"
	  #include "soc/rmt_struct.h"
	#elif defined(ESP_PLATFORM)
	  #include <esp_intr.h>
	  #include <driver/gpio.h>
	  #include <driver/rmt.h>
	  #include <freertos/FreeRTOS.h>
	  #include <freertos/semphr.h>
	  #include <soc/dport_reg.h>
	  #include <soc/gpio_sig_map.h>
	  #include <soc/rmt_struct.h>
	  #include <stdio.h>
	  #include <string.h>  // memset, memcpy, etc. live here!
	#endif

	#ifdef __cplusplus
	}
	#endif


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

	typedef struct
	{
		int bytesPerPixel;
		uint32_t T0H;
		uint32_t T1H;
		uint32_t T0L;
		uint32_t T1L;
		uint32_t TRS;
	} ledParams_t;

	enum led_types
	{
		LED_WS2812_V1,
		LED_WS2812B_V1,
		LED_WS2812B_V2,
		LED_WS2812B_V3,
		LED_WS2813_V1,
		LED_WS2813_V2,
		LED_WS2813_V3,
		LED_SK6812_V1,
		LED_SK6812W_V1,
	};

	typedef union {
		struct {
			uint32_t duration0 : 15;
			uint32_t level0 : 1;
			uint32_t duration1 : 15;
			uint32_t level1 : 1;
		};
		uint32_t val;
	} rmtPulsePair;

	typedef struct {
		uint8_t * buf_data;
		uint16_t buf_pos, buf_len, buf_half, buf_isDirty;
		xSemaphoreHandle sem;
		rmtPulsePair pulsePairMap[2];
	} digitalLeds_stateData;


	const ledParams_t ledParamsAll[] = {  // Still must match order of `led_types`
	  [LED_WS2812_V1]  = { .bytesPerPixel = 3, .T0H = 350, .T1H = 700, .T0L = 800, .T1L = 600, .TRS =  50000},
	  [LED_WS2812B_V1] = { .bytesPerPixel = 3, .T0H = 350, .T1H = 900, .T0L = 900, .T1L = 350, .TRS =  50000}, // Older datasheet
	  [LED_WS2812B_V2] = { .bytesPerPixel = 3, .T0H = 400, .T1H = 850, .T0L = 850, .T1L = 400, .TRS =  50000}, // 2016 datasheet
	  [LED_WS2812B_V3] = { .bytesPerPixel = 3, .T0H = 450, .T1H = 850, .T0L = 850, .T1L = 450, .TRS =  50000}, // cplcpu test
	  [LED_WS2813_V1]  = { .bytesPerPixel = 3, .T0H = 350, .T1H = 800, .T0L = 350, .T1L = 350, .TRS = 300000}, // Older datasheet
	  [LED_WS2813_V2]  = { .bytesPerPixel = 3, .T0H = 270, .T1H = 800, .T0L = 800, .T1L = 270, .TRS = 300000}, // 2016 datasheet
	  [LED_WS2813_V3]  = { .bytesPerPixel = 3, .T0H = 270, .T1H = 630, .T0L = 630, .T1L = 270, .TRS = 300000}, // 2017-05 WS datasheet
	  [LED_SK6812_V1]  = { .bytesPerPixel = 3, .T0H = 300, .T1H = 600, .T0L = 900, .T1L = 600, .TRS =  80000},
	  [LED_SK6812W_V1] = { .bytesPerPixel = 4, .T0H = 300, .T1H = 600, .T0L = 900, .T1L = 600, .TRS =  80000},
	};


	class LEDControll
	{
	public:


		// Hold Driver for LEDs
		// Webserver
		// API
	private:
		std::vector<strandchain_t> m_pStrandChains;

	public:
		LEDControll();
		~LEDControll();

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