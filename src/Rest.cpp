#include "Rest.h"

namespace LEDCNTRL
{

  Rest::Rest(API* a_pApi):
  pAPI(a_pApi)
  {
    pServer = new AsyncWebServer(80);
    pWs = new AsyncWebSocket("/stripcontroll");

    initWifi();
    initWebcontent();
    registerHandler();

    pServer->begin();
  }


  Rest::~Rest()
  {
    //delete(server);
    //delete(ws);
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


  void Rest::initWebcontent()
  {
    pServer->serveStatic("/", SPIFFS, "/");
  }


  void Rest::registerHandler()
  {
    Serial.println("Register Events!");
    pServer->on("/index", HTTP_ANY, [](AsyncWebServerRequest *request)
    {
      request->send(SPIFFS, "/index.htm");
    });


   using namespace std::placeholders;
    std::function<void(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)> onEventFnc = std::bind(&Rest::onEvent, this, _1,_2,_3,_4,_5,_6 );
    std::function<void (AsyncWebServerRequest *request)> onRequestFnc = std::bind(&Rest::onRequest, this, _1 );

    pWs->onEvent(onEventFnc);
    pServer->addHandler(pWs);

    pServer->onNotFound(onRequestFnc);

      /*server.on("/getChains", HTTP_POST, [](AsyncWebServerRequest *request)
      {
        Serial.println("Color Event!");
        AsyncWebParameter* r = request->getParam("r",true);
        AsyncWebParameter* g = request->getParam("g",true);
        AsyncWebParameter* b = request->getParam("b",true);
    
    //    setColor(r->value().toInt(),g->value().toInt(),b->value().toInt(), 128);
        request->send(200, "text/plain","It works!" );
      });*/
     //
    //  server.on("/red", HTTP_GET, [](AsyncWebServerRequest *request){
    //      setColor(255,0,0,255);
    //      request->send(200, "text/plain", "Red");
    //  });
  }

  void Rest::onRequest(AsyncWebServerRequest *request)
  {
    
    Serial.print("Unknown request! : ");
    Serial.println(request->url());
    //Handle Unknown Request
    request->send(404);
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
         printf("ws[%s][%u] %s-message[%llu]: \n", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
         if(info->opcode == WS_TEXT)
         {
           data[len] = 0;
           String json = String((char*)data);

           parseCommand(json);
         }
       }

     }
  }


  void Rest::parseCommand(String a_rJsonString)
  {
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& jobj = jsonBuffer.parseObject(a_rJsonString);
    Serial.println(a_rJsonString);

      if(!jobj.success())
      {
        Serial.println("parseObject() failed");
        return;
      }

      else if(jobj["command"] == "SET_COLOR")
      {

          modul_config_solidColor cfg{0};

          float r = jobj["data"]["rgb"]["r"];
          float g = jobj["data"]["rgb"]["g"];
          float b = jobj["data"]["rgb"]["b"];


          cfg.solidColor.r = r * 255;
          cfg.solidColor.g = g * 255;
          cfg.solidColor.b = b * 255;


         pAPI->setStripConfig<modul_config_solidColor>(0,1,cfg);
      }
  }
}
