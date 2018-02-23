#include "LEDControll.h"



	#if DEBUG_ESP32_DIGITAL_LED_LIB
	extern char * digitalLeds_debugBuffer;
	extern int digitalLeds_debugBufferSz;
	#endif
namespace LEDCNTRL
{
	static DRAM_ATTR const uint16_t MAX_PULSES = 32;  // A channel has a 64 "pulse" buffer - we use half per pass
	static DRAM_ATTR const uint16_t DIVIDER    =  4;  // 8 still seems to work, but timings become marginal
	static DRAM_ATTR const double   RMT_DURATION_NS = 12.5;  // Minimum time of a single RMT duration based on clock ns

	// LUT for mapping bits in RMT.int_<op>.ch<n>_tx_thr_event
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

static strandchain_t * localStrands;
static int localStrandCnt = 0;

static intr_handle_t rmt_intr_handle = nullptr;

// Forward declarations of local functions
static void copyToRmtBlock_half(strand_t * pStrand);
static void handleInterrupt(void *arg);



	static IRAM_ATTR void copyToRmtBlock_half(strandchain_t * pStrand)
{
  // This fills half an RMT block
  // When wraparound is happening, we want to keep the inactive half of the RMT block filled

  digitalLeds_stateData * pState = static_cast<digitalLeds_stateData*>(pStrand->_stateVars);
  ledParams_t ledParams = ledParamsAll[pStrand->ledType];

  uint16_t i, j, offset, len, byteval;

  offset = pState->buf_half * MAX_PULSES;
  pState->buf_half = !pState->buf_half;

  len = pState->buf_len - pState->buf_pos;
  if (len > (MAX_PULSES / 8))
    len = (MAX_PULSES / 8);

  if (!len) {
    if (!pState->buf_isDirty) {
      return;
    }
    // Clear the channel's data block and return
    for (i = 0; i < MAX_PULSES; i++) {
      RMTMEM.chan[pStrand->rmtChannel].data32[i + offset].val = 0;
    }
    pState->buf_isDirty = 0;
    return;
  }
  pState->buf_isDirty = 1;

  for (i = 0; i < len; i++) {
    byteval = pState->buf_data[i + pState->buf_pos];

    #if DEBUG_ESP32_DIGITAL_LED_LIB
      snprintf(digitalLeds_debugBuffer, digitalLeds_debugBufferSz,
               "%s%d(", digitalLeds_debugBuffer, byteval);
    #endif

    // Shift bits out, MSB first, setting RMTMEM.chan[n].data32[x] to
    // the rmtPulsePair value corresponding to the buffered bit value
    for (j = 0; j < 8; j++, byteval <<= 1) {
      int bitval = (byteval >> 7) & 0x01;
      int data32_idx = i * 8 + offset + j;
      RMTMEM.chan[pStrand->rmtChannel].data32[data32_idx].val = pState->pulsePairMap[bitval].val;
      #if DEBUG_ESP32_DIGITAL_LED_LIB
        snprintf(digitalLeds_debugBuffer, digitalLeds_debugBufferSz,
                 "%s%d", digitalLeds_debugBuffer, bitval);
      #endif
    }
    #if DEBUG_ESP32_DIGITAL_LED_LIB
      snprintf(digitalLeds_debugBuffer, digitalLeds_debugBufferSz,
               "%s) ", digitalLeds_debugBuffer);
    #endif

    // Handle the reset bit by stretching duration1 for the final bit in the stream
    if (i + pState->buf_pos == pState->buf_len - 1) {
      RMTMEM.chan[pStrand->rmtChannel].data32[i * 8 + offset + 7].duration1 =
        ledParams.TRS / (RMT_DURATION_NS * DIVIDER);
      #if DEBUG_ESP32_DIGITAL_LED_LIB
        snprintf(digitalLeds_debugBuffer, digitalLeds_debugBufferSz,
                 "%sRESET ", digitalLeds_debugBuffer);
      #endif
    }
  }

  // Clear the remainder of the channel's data not set above
  for (i *= 8; i < MAX_PULSES; i++) {
    RMTMEM.chan[pStrand->rmtChannel].data32[i + offset].val = 0;
  }
  
  pState->buf_pos += len;

  #if DEBUG_ESP32_DIGITAL_LED_LIB
    snprintf(digitalLeds_debugBuffer, digitalLeds_debugBufferSz,
             "%s ", digitalLeds_debugBuffer);
  #endif

  return;
}

static IRAM_ATTR void handleInterrupt(void *arg)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  #if DEBUG_ESP32_DIGITAL_LED_LIB
    snprintf(digitalLeds_debugBuffer, digitalLeds_debugBufferSz,
             "%sRMT.int_st.val = %08x\n", digitalLeds_debugBuffer, RMT.int_st.val);
  #endif

  for (int i = 0; i < localStrandCnt; i++) {
    strandchain_t * pStrand = &localStrands[i];
    digitalLeds_stateData * pState = static_cast<digitalLeds_stateData*>(pStrand->_stateVars);

    if (RMT.int_st.val & tx_thr_event_offsets[pStrand->rmtChannel])
    {  // tests RMT.int_st.ch<n>_tx_thr_event
      copyToRmtBlock_half(pStrand);
      RMT.int_clr.val |= tx_thr_event_offsets[pStrand->rmtChannel];  // set RMT.int_clr.ch<n>_tx_thr_event
    }
    else if (RMT.int_st.val & tx_end_offsets[pStrand->rmtChannel] && pState->sem)
    {  // tests RMT.int_st.ch<n>_tx_end and semaphore
      xSemaphoreGiveFromISR(pState->sem, &xHigherPriorityTaskWoken);
      RMT.int_clr.val |= tx_end_offsets[pStrand->rmtChannel];  // set RMT.int_clr.ch<n>_tx_end 
      if (xHigherPriorityTaskWoken == pdTRUE)
      {
          portYIELD_FROM_ISR();
      }
    }
  }

  return;
}
}


namespace LEDCNTRL
{
	//typedef std::function<void(void*)> ModuleLoop_t;
	//typedef void(*ModuleLoop_t)(void* p);
	LEDControll::LEDControll()
	{
		this->m_pStrandChains.clear();
	}

	LEDControll::~LEDControll()
	{
	}

	void LEDControll::update()
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


	void LEDControll::setStrandModul(EModul m, int ChainId, int StrandId)
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

	void LEDControll::setChainModul(EModul m, int chainID)
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

	const void* LEDControll::getStrandConfig(int ChainId, int StrandId)
	{
		return this->m_pStrandChains[ChainId].pStrands[StrandId].pParam;
	}

	void LEDControll::modul_Solidcolor(strand_t* pStrand)
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

	uint32_t LEDControll::createChain(strandchain_t chainDesc)
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

	void LEDControll::initChain(uint32_t chainID)
	{
		Serial.print("Init Chain[");
		Serial.print(chainID);
		Serial.println("]");
		
		ledParams_t ledParams = ledParamsAll[this->m_pStrandChains[chainID].ledType]; 
		strandchain_t* pStrand = &this->m_pStrandChains[chainID];

		m_pStrandChains[chainID].pWholePixels = static_cast<pixelColor_t*>(malloc(m_pStrandChains[chainID].totalPixel * sizeof(pixelColor_t)));
		memset(m_pStrandChains[chainID].pWholePixels, 1, m_pStrandChains[chainID].totalPixel * sizeof(pixelColor_t));
		// Alloc RMT Buffer
		m_pStrandChains[chainID]._stateVars = static_cast<digitalLeds_stateData*>(malloc(sizeof(digitalLeds_stateData)));
		memset(m_pStrandChains[chainID]._stateVars, 0, sizeof(digitalLeds_stateData));

		digitalLeds_stateData * pState = static_cast<digitalLeds_stateData*>(m_pStrandChains[chainID]._stateVars);
		pState->buf_len = (m_pStrandChains[chainID].totalPixel * 3);
		pState->buf_data = static_cast<uint8_t*>(malloc(pState->buf_len));

		rmt_set_pin(
	      static_cast<rmt_channel_t>(pStrand->rmtChannel),
	      RMT_MODE_TX,
	      static_cast<gpio_num_t>(pStrand->gpioNum));
	  
	    RMT.conf_ch[pStrand->rmtChannel].conf0.div_cnt = DIVIDER;
	    RMT.conf_ch[pStrand->rmtChannel].conf0.mem_size = 1;
	    RMT.conf_ch[pStrand->rmtChannel].conf0.carrier_en = 0;
	    RMT.conf_ch[pStrand->rmtChannel].conf0.carrier_out_lv = 1;
	    RMT.conf_ch[pStrand->rmtChannel].conf0.mem_pd = 0;
	  
	    RMT.conf_ch[pStrand->rmtChannel].conf1.rx_en = 0;
	    RMT.conf_ch[pStrand->rmtChannel].conf1.mem_owner = 0;
	    RMT.conf_ch[pStrand->rmtChannel].conf1.tx_conti_mode = 0;  //loop back mode
	    RMT.conf_ch[pStrand->rmtChannel].conf1.ref_always_on = 1;  // use apb clock: 80M
	    RMT.conf_ch[pStrand->rmtChannel].conf1.idle_out_en = 1;
	    RMT.conf_ch[pStrand->rmtChannel].conf1.idle_out_lv = 0;
	  
	    RMT.tx_lim_ch[pStrand->rmtChannel].limit = MAX_PULSES;

		pState->pulsePairMap[0].level0 = 1;
		pState->pulsePairMap[0].level1 = 0;
		pState->pulsePairMap[0].duration0 = ledParams.T0H / (RMT_DURATION_NS * DIVIDER);
		pState->pulsePairMap[0].duration1 = ledParams.T0L / (RMT_DURATION_NS * DIVIDER);

		//RMT config for transmitting a '0' bit val to this LED strand
		pState->pulsePairMap[1].level0 = 1;
		pState->pulsePairMap[1].level1 = 0;
		pState->pulsePairMap[1].duration0 = ledParams.T1H / (RMT_DURATION_NS * DIVIDER);
		pState->pulsePairMap[1].duration1 = ledParams.T1L / (RMT_DURATION_NS * DIVIDER);

		RMT.int_ena.val |= tx_thr_event_offsets[pStrand->rmtChannel];  // RMT.int_ena.ch<n>_tx_thr_event = 1;
   		RMT.int_ena.val |= tx_end_offsets[pStrand->rmtChannel];  // RMT.int_ena.ch<n>_tx_end = 1;
  		esp_intr_alloc(ETS_RMT_INTR_SOURCE, 0, handleInterrupt, nullptr, &rmt_intr_handle);
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

	uint32_t LEDControll::appendStrand(int chainID, strand_t strandDesc)
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

	uint32_t LEDControll::getChainCount()
	{
		return this->m_pStrandChains.size();
	}

	const strandchain_t& LEDControll::getChain(uint32_t chainID)
	{
		return this->m_pStrandChains[chainID];
	}

	const strand_t&  LEDControll::getStrand(uint32_t chainID, uint32_t strandID)
	{
		return this->m_pStrandChains[chainID].pStrands[strandID];
	}


	void LEDControll::updateLEDs(strandchain_t* pStrandChain)
	{

		Serial.print("Updateing Chain[");
		Serial.print(pStrandChain->id);
		Serial.println("]");

		digitalLeds_stateData * pState = static_cast<digitalLeds_stateData*>(pStrandChain->_stateVars);
		ledParams_t ledParams = ledParamsAll[pStrandChain->ledType]; 

		// Pack pixels into transmission buffer
		if (ledParams.bytesPerPixel == 3) 
		{
			Serial.println("We have 3 colors!");
			for (uint16_t i = 0; i < pStrandChain->totalPixel; i++) 
			{
				Serial.print(i);
				Serial.print("/");
				Serial.println(pStrandChain->totalPixel);

				// Color order is translated from RGB to GRB
				pState->buf_data[0 + i * 3] = pStrandChain->pWholePixels[i].g;
				pState->buf_data[1 + i * 3] = pStrandChain->pWholePixels[i].r;
				pState->buf_data[2 + i * 3] = pStrandChain->pWholePixels[i].b;
			}
		}
		else if (ledParams.bytesPerPixel == 4) {
		for (uint16_t i = 0; i < pStrandChain->totalPixel; i++) {
		// Color order is translated from RGBW to GRBW
		pState->buf_data[0 + i * 4] = pStrandChain->pWholePixels[i].g;
		pState->buf_data[1 + i * 4] = pStrandChain->pWholePixels[i].r;
		pState->buf_data[2 + i * 4] = pStrandChain->pWholePixels[i].b;
		pState->buf_data[3 + i * 4] = pStrandChain->pWholePixels[i].w;
		}
		}
		else {
		return;
		}

		pState->buf_pos = 0;
		pState->buf_half = 0;

		Serial.println("Copy first half");
		copyToRmtBlock_half(pStrandChain);

		if (pState->buf_pos < pState->buf_len) {
		// Fill the other half of the buffer block
		#if DEBUG_ESP32_DIGITAL_LED_LIB
		snprintf(digitalLeds_debugBuffer, digitalLeds_debugBufferSz,
		"%s# ", digitalLeds_debugBuffer);
		#endif
		Serial.println("Copy second half");
		copyToRmtBlock_half(pStrandChain);
		}

		Serial.println("Create semaphore");
		pState->sem = xSemaphoreCreateBinary();

		RMT.conf_ch[pStrandChain->rmtChannel].conf1.mem_rd_rst = 1;
		RMT.conf_ch[pStrandChain->rmtChannel].conf1.tx_start = 1;

		Serial.println("TakeSemaohore");
		xSemaphoreTake(pState->sem, portMAX_DELAY);
		vSemaphoreDelete(pState->sem);
		pState->sem = nullptr;
		
		Serial.println("TokeSemaohore");

		Serial.print("Finish Updateing Chain[");
		Serial.print(pStrandChain->id);
		Serial.println("]");
	}

		
}
