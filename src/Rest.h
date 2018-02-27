#pragma once

#include <Arduino.h>
#include "WiFi.h"
#include "SPIFFS.h"
#include "AsyncWebSocket.h"
#include "ESPAsyncWebServer.h"
#include "AsyncEventSource.h"
#include <ArduinoJson.h>
#include <string>
#include <functional>
#include "API.h"

#include "LEDControl.h"


namespace LEDCNTRL
{
	class Rest
	{
		private:
			std::string WIFI_SSID = "o2-WLAN51";
			std::string WIFI_PASS = "6689171153799911";

		private:
			AsyncWebServer* pServer;
			AsyncWebSocket* pWs; // access at ws://[esp ip]/ws
			API* pAPI;

		public:
			Rest(API* a_pApi);
			~Rest();

		private:
			void initWifi();
			void initWebcontent();
			void registerHandler();
			
			void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
			void onRequest(AsyncWebServerRequest *request);

			void parseCommand(String a_rJsonString);

	};
}
