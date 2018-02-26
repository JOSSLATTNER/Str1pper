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
#include "WS2812bDriver.h"

namespace LEDCNTRL
{

	static DRAM_ATTR const uint32_t tx_thr_event_offsets [] = {
  	static_cast<uint32_t>(1) << (24 + 0),
  	static_cast<uint32_t>(1) << (24 + 1),
  	static_cast<uint32_t>(1) << (24 + 2),
	static_cast<uint32_t>(1) << (24 + 3),
	static_cast<uint32_t>(1) << (24 + 4),
	static_cast<uint32_t>(1) << (24 + 5),
	static_cast<uint32_t>(1) << (24 + 6),
	static_cast<uint32_t>(1) << (24 + 7),
	};

// LUT for mapping bits in RMT.int_<op>.ch<n>_tx_end
	static DRAM_ATTR const uint32_t tx_end_offsets [] = {
	static_cast<uint32_t>(1) << (0 + 0) * 3,
	static_cast<uint32_t>(1) << (0 + 1) * 3,
	static_cast<uint32_t>(1) << (0 + 2) * 3,
	static_cast<uint32_t>(1) << (0 + 3) * 3,
	static_cast<uint32_t>(1) << (0 + 4) * 3,
	static_cast<uint32_t>(1) << (0 + 5) * 3,
	static_cast<uint32_t>(1) << (0 + 6) * 3,
	static_cast<uint32_t>(1) << (0 + 7) * 3,
	};

	static IRAM_ATTR void handleInterrupt(void* arg)
	{
		WS2812bDriver* pDriverInst = static_cast<WS2812bDriver*>(arg);

		portBASE_TYPE taskAwoke = 0;

		for(uint32_t i = 0; i < pDriverInst->m_pStripChains.size(); i++)
		{
			WS2812b_leddriver_config_t* cfg = static_cast<WS2812b_leddriver_config_t*>(pDriverInst->m_pStripChains[i]->driver_config);


			if(RMT.int_st.val & tx_thr_event_offsets[cfg->rmtChannel])
			{
				pDriverInst->copyToRmtBlock_half(pDriverInst->m_pStripChains[i]);
				RMT.int_clr.val |= tx_thr_event_offsets[cfg->rmtChannel];
			}
			else if(RMT.int_st.val & tx_end_offsets[cfg->rmtChannel] && cfg->semaphore)
			{
				xSemaphoreGiveFromISR(cfg->semaphore, &taskAwoke);
				RMT.int_clr.val |= tx_end_offsets[cfg->rmtChannel];

				if(taskAwoke == pdTRUE)
					portYIELD_FROM_ISR();
			}
		}
	}

	WS2812bDriver::WS2812bDriver()
	{
		esp_intr_alloc(ETS_RMT_INTR_SOURCE,0,handleInterrupt,this,&rmt_intr_handle);
	}

	WS2812bDriver::~WS2812bDriver()
	{
	}

	void WS2812bDriver::InitDriver(stripchain_t* pStripChain)
	{
		this->m_pStripChains.push_back(pStripChain);
		
		DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_RMT_CLK_EN);
  		DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_RMT_RST);

  		rmt_set_pin(static_cast<rmt_channel_t>(pStripChain->rmtChannel), RMT_MODE_TX,static_cast<gpio_num_t>(pStripChain->gpioPin));
	
  		RMT.apb_conf.fifo_mask = 1;  // Enable memory access, instead of FIFO mode
  		RMT.apb_conf.mem_tx_wrap_en = 1;  // Wrap around when hitting end of buffer
  		RMT.conf_ch[pStripChain->rmtChannel].conf0.div_cnt = DIVIDER;
    	RMT.conf_ch[pStripChain->rmtChannel].conf0.mem_size = 1;
    	RMT.conf_ch[pStripChain->rmtChannel].conf0.carrier_en = 0;
    	RMT.conf_ch[pStripChain->rmtChannel].conf0.carrier_out_lv = 1;
    	RMT.conf_ch[pStripChain->rmtChannel].conf0.mem_pd = 0;
  
    	RMT.conf_ch[pStripChain->rmtChannel].conf1.rx_en = 0;
    	RMT.conf_ch[pStripChain->rmtChannel].conf1.mem_owner = 0;
   	 	RMT.conf_ch[pStripChain->rmtChannel].conf1.tx_conti_mode = 0;  //loop back mode
    	RMT.conf_ch[pStripChain->rmtChannel].conf1.ref_always_on = 1;  // use apb clock: 80M
    	RMT.conf_ch[pStripChain->rmtChannel].conf1.idle_out_en = 1;
    	RMT.conf_ch[pStripChain->rmtChannel].conf1.idle_out_lv = 0;

    	RMT.tx_lim_ch[pStripChain->rmtChannel].limit = MAX_PULSES;

    	RMT.int_ena.val |= tx_thr_event_offsets[pStripChain->rmtChannel];  // RMT.int_ena.ch<n>_tx_thr_event = 1;
    	RMT.int_ena.val |= tx_end_offsets[pStripChain->rmtChannel];  // RMT.int_ena.ch<n>_tx_end = 1;
	}

	void WS2812bDriver::UpdateLEDS(stripchain_t* pStripChain)
	{
		WS2812b_leddriver_config_t* cfg = static_cast<WS2812b_leddriver_config_t*>(pStripChain->driver_config);

		cfg->buffer_pos = 0;
		cfg->buffer_half = 0;

		copyToRmtBlock_half(pStripChain);

		if(cfg->buffer_pos < cfg->buffer_len)
			copyToRmtBlock_half(pStripChain);

		cfg->semaphore = xSemaphoreCreateBinary();

		RMT.conf_ch[cfg->rmtChannel].conf1.mem_rd_rst = 1;
		RMT.conf_ch[cfg->rmtChannel].conf1.tx_start = 1;
		
		xSemaphoreTake(cfg->semaphore, portMAX_DELAY);
		vSemaphoreDelete(cfg->semaphore);
		cfg->semaphore = nullptr;
	}

	void WS2812bDriver::copyToRmtBlock_half(stripchain_t* pStripChain)
	{
		uint16_t i, j, offset, len, bit;

		WS2812b_leddriver_config_t* cfg = static_cast<WS2812b_leddriver_config_t*>(pStripChain->driver_config);

		offset = cfg->buffer_half * MAX_PULSES;
  		cfg->buffer_half = !cfg->buffer_half;

  		len = cfg->buffer_len - cfg->buffer_pos;

  		if (len > (MAX_PULSES / 8))
    		len = (MAX_PULSES / 8);

    	if (!len) 
    	{
			for (i = 0; i < MAX_PULSES; i++)
		      RMTMEM.chan[cfg->rmtChannel].data32[i + offset].val = 0;
		    return;
		}

		for (i = 0; i < len; i++) 
		{
		  	bit = cfg->pBuffer[i + cfg->buffer_pos];
		  
		  	for (j = 0; j < 8; j++, bit <<= 1) 
		  	{
		    	RMTMEM.chan[cfg->rmtChannel].data32[j + i * 8 + offset].val =	cfg->pulsepair[(bit >> 7) & 0x01].val;
		  	}

			if (i + cfg->buffer_pos == cfg->buffer_len - 1)
		      RMTMEM.chan[cfg->rmtChannel].data32[7 + i * 8 + offset].duration1 = 50000 / (DIVIDER * DURATION);
		}

		for (i *= 8; i < MAX_PULSES; i++)
    		RMTMEM.chan[cfg->rmtChannel].data32[i + offset].val = 0;

    	cfg->buffer_pos += len;
    	return;
	}
}