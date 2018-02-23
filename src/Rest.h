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

class Rest
{
	private:
		std::string WIFI_SSID = "o2-WLAN51";
		std::string WIFI_PASS = "6689171153799911";

		AsyncWebServer* server;
		AsyncWebSocket* ws;
		AsyncEventSource* events;

	public:
		Rest();
		~Rest();

	private:
		void initWifi();
		void initWebsocket();
		void initWebcontent();
		void registerEvents();
		void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
		void parseCommand(String a_rJsonString);

};