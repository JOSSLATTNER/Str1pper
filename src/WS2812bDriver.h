#pragma once 
#include <Arduino.h>
#include "LEDDriver.h"
#include "rmt_pulsepair_t.h"
#include <vector>
#include "stripchain_t.h"
#include "driver_config_t.h"
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <soc/rmt_struct.h>
#include <soc/dport_reg.h>
#include <driver/gpio.h>
#include <soc/gpio_sig_map.h>
#include <esp_intr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace LEDCNTRL
{	
	static IRAM_ATTR void handleInterrupt(void* arg);
	static intr_handle_t rmt_intr_handle = NULL;
	
	class WS2812bDriver : public LEDDriver
	{
	public:
		std::vector<stripchain_t*> m_pStripChains;
		

	public:
		WS2812bDriver( );
		~WS2812bDriver();

	public:
		void InitDriver(stripchain_t* pStripchain);
		void UpdateLEDS(stripchain_t* pStripChain);

	

	public:
		void copyToRmtBlock_half(stripchain_t* pStripChain);
	};
}