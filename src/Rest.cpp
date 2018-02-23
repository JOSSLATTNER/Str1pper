#include "Rest.h"

Rest::Rest()
{
	server = new AsyncWebServer(80);
	ws = new AsyncWebSocket("/ws");

	initWifi();
	initWebsocket();
	registerEvents();
	server->begin();
}


Rest::~Rest()
{
	delete(server);
	delete(ws);
}

void Rest::initWifi()
{
	Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID.c_str());


    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    Serial.println("Connecting...");

    while (WiFi.status() != WL_CONNECTED) 
    {
      if (WiFi.status() == WL_CONNECT_FAILED) 
      {
        Serial.println("Failed to connect to WIFI. Please verify credentials: ");
        Serial.println();
        Serial.print("SSID: ");
        Serial.println(WIFI_SSID.c_str());
        Serial.print("Password: ");
        Serial.println(WIFI_PASS.c_str());
        Serial.println();
      }

      delay(5000);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Hello World, I'm connected to the internets!!");

}

void Rest::initWebsocket()
{
	using namespace std::placeholders;
	


	std::function<void(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)> f = std::bind(&Rest::onEvent, this, _1,_2,_3,_4,_5,_6 );
	this->ws->onEvent(f);
	server->addHandler(ws);
	Serial.println("Init Websocket!");
}

void Rest::initWebcontent()
{
	server->serveStatic("/", SPIFFS, "/");
	server->serveStatic("/js", SPIFFS, "/js");
  	server->serveStatic("/image", SPIFFS, "/image");
  	server->serveStatic("/css", SPIFFS, "/css");
}


void Rest::registerEvents()
{
	Serial.println("Register Events!");
    server->on("/getChains", HTTP_POST, [](AsyncWebServerRequest *request)
    {
    	Serial.println("Color Event!");
      AsyncWebParameter* r = request->getParam("r",true);
      AsyncWebParameter* g = request->getParam("g",true);
      AsyncWebParameter* b = request->getParam("b",true);
  
  //    setColor(r->value().toInt(),g->value().toInt(),b->value().toInt(), 128);
      request->send(200, "text/plain","It works!" );
   	});
   //
  //  server.on("/red", HTTP_GET, [](AsyncWebServerRequest *request){
  //      setColor(255,0,0,255);
  //      request->send(200, "text/plain", "Red");
  //  });
}


void Rest::onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	if(type == WS_EVT_CONNECT)
  {
     //client connected
     printf("ws[%s][%u] connect\n", server->url(), client->id());
   }
   else if(type == WS_EVT_DISCONNECT)
   {
     //client disconnected
     printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
   }
   else if(type == WS_EVT_ERROR)
   {
     //error was received from the other end
     printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
   }
   else if(type == WS_EVT_DATA)
   {
     //data packet
     AwsFrameInfo * info = (AwsFrameInfo*)arg;
     if(info->final && info->index == 0 && info->len == len)
     {
       //the whole message is in a single frame and we got all of it's data
       printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
       if(info->opcode == WS_TEXT)
       {
         data[len] = 0;
         String json = String((char*)data);

         parseCommand(json);
       }
     }

   }
   delay(1);
}

void Rest::parseCommand(String a_rJsonString)
{
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& jobj = jsonBuffer.parseObject(a_rJsonString);
  Serial.println(a_rJsonString);

  /*if(!jobj.success())
  {
    Serial.println("parseObject() failed");
    return;
  }

  if(jobj["command"] == "SET_MODULE")
  {
      ModuleIndex = jobj["data"]["module"];
      Serial.print("ModulIndex:");
      Serial.print(ModuleIndex);
  }
  else if(jobj["command"] == "SET_COLOR")
  {
      float r =  jobj["data"]["rgb"]["r"];
      float g =  jobj["data"]["rgb"]["g"];
      float b =  jobj["data"]["rgb"]["b"];

      _Color = { r, g, b };
  }*/
}