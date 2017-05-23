
#include <stdlib.h>
#include <cmath>
#include <functional>
#include <string>

#include "CScriptVariable.h"
#include "CIScriptVariable.h"
#include "CModul.h"

#include <ESP8266WiFi.h>
#include <Hash.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>


extern "C" {
#include "user_interface.h"
}

enum LEDModul
{
  NONE,
  PLASMA,
  BLINK,
  SOLIDCOLOR
};


const char* ssid = "o2-WLAN51";
const char* password = "6689171153799911";
const bool apMode = false;
const char WiFiAPPSK[] = "";

#define LEDPIN 0
int NUMPIXELS = 30;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);
AsyncWebSocket ws("/stripcontroll");

LEDModul currentModul = LEDModul::NONE;
double timer;

Str1pper::CIScriptVariable* fsv;
Str1pper::CModul* m;

float* modulTime;

void setColor(int a_r, int a_g, int a_b, int a_a)
{
  Serial.print(F("Change Color: r:"));
  Serial.print(a_r);
  Serial.print(" g:");
  Serial.print(a_g);
  Serial.print(" b:");
  Serial.print(a_b);
  Serial.println("");

  for(int i = 0 ; i < NUMPIXELS; ++i)
  {
    pixels.setPixelColor(i,pixels.Color(a_r,a_g,a_b));
  }
  pixels.show();
}


void plasma()
{
  for(int i = 0 ; i < NUMPIXELS; ++i)
  {
      float steps = (float)NUMPIXELS / 1.0;
      float currentStep = i / steps;
    float cx = currentStep + 0.5 * sin(timer/5.0);
    float cy = currentStep+0.5 * cos(timer/3.0);

    pixels.setPixelColor(i,pixels.Color(cx*255,cy*255,0));
  }
}

void laufsinus()
{
  //Serial.print("Plama!");
  for(int i = 0 ; i < NUMPIXELS; ++i)
  {
    float steps = (float)NUMPIXELS / 3.0;
    float currentStep = i / steps;

    float s = (sin(currentStep*6.28-1.57+timer)*0.5+0.5)*255;
      pixels.setPixelColor(i,pixels.Color(s,0,0));
  }
//  pixels.show();
}

void setupWifi()
{
  if (apMode)
  {
    WiFi.mode(WIFI_AP);

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                   String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = "ESP8266 Thing " + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i = 0; i < AP_NameString.length(); i++)
      AP_NameChar[i] = AP_NameString.charAt(i);

    WiFi.softAP(AP_NameChar, WiFiAPPSK);

    Serial.printf("Connect to Wi-Fi access point: %s\n", AP_NameChar);
    Serial.println("and open http://192.168.4.1 in your browser");
  }
  else
  {
    WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s\n", ssid);
    if (String(WiFi.SSID()) != String(ssid)) {
      WiFi.begin(ssid, password);
    }

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.print("Connected! Open http://");
    Serial.print(WiFi.localIP());
    Serial.println(" in your browser");
  }
}

void registerEvents()
{
   server.on("/color", HTTP_POST, [](AsyncWebServerRequest *request){
     AsyncWebParameter* r = request->getParam("r",true);
     AsyncWebParameter* g = request->getParam("g",true);
     AsyncWebParameter* b = request->getParam("b",true);
     String a = "128";

     setColor(r->value().toInt(),g->value().toInt(),b->value().toInt(),a.toInt());
    request->send(200, "text/plain", "");
   });

   server.on("/red", HTTP_GET, [](AsyncWebServerRequest *request){
       setColor(255,0,0,255);
       request->send(200, "text/plain", "Red");
     });
     server.on("/plasma", HTTP_GET, [](AsyncWebServerRequest *request){
        //plasma();
        currentModul = LEDModul::PLASMA;
         request->send(200, "text/plain", "Plasma!!!");
       });
  /*   server.on("/SetLedAmount", HTTP_GET, [](AsyncWebServerRequest *request){
       char buffer[10];

         AsyncWebParameter* r = request->getParam("amount",true);
         NUMPIXELS = r->value().toInt();

         request->send(200, "text/plain", itoa(NUMPIXELS,buffer,10) );
       });*/
}

void parseCommand(String& a_rJsonString)
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& jsonObject = jsonBuffer.parseObject(a_rJsonString);


  if(!jsonObject.success())
  {
    Serial.println("parseObject() failed");
    return;
  }



  if(jsonObject["command"] == "solidcolor")
  {
    float r = jsonObject["data"][0];
    float g = jsonObject["data"][1];
    float b = jsonObject["data"][2];

    setColor(r*255,g*255,b*255,0);
  }
  if(jsonObject["command"] == "plasma")
  {
    //plasma();
  }
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
     //client connected
     os_printf("ws[%s][%u] connect\n", server->url(), client->id());
     client->printf("Hello Client %u :)", client->id());
     client->ping();
   } else if(type == WS_EVT_DISCONNECT){
     //client disconnected
     os_printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
   } else if(type == WS_EVT_ERROR){
     //error was received from the other end
     os_printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
   } else if(type == WS_EVT_PONG){
     //pong message was received (in response to a ping request maybe)
     os_printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
   }
   else if(type == WS_EVT_DATA)
   {
     //data packet
     AwsFrameInfo * info = (AwsFrameInfo*)arg;
     if(info->final && info->index == 0 && info->len == len){
       //the whole message is in a single frame and we got all of it's data
       os_printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
       if(info->opcode == WS_TEXT)
       {
         data[len] = 0;
         String json = String((char*)data);


         Serial.println(json);
         parseCommand(json);
       }
       else
       {
         for(size_t i=0; i < info->len; i++)
         {
           os_printf("%02x ", data[i]);
         }
         os_printf("\n");
       }
       if(info->opcode == WS_TEXT)
         client->text("I got your text message");
       else
         client->binary("I got your binary message");
     }
   }
   delay(1);
}


void setupWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setupWebCommuncation()
{
  setupWebSocket();
  server.serveStatic("/", SPIFFS, "/");
  server.serveStatic("/js", SPIFFS, "/js");
  registerEvents();
  server.begin();
  Serial.println("HTTP server started");
}

void setupLeds()
{
  pixels.begin();
  pixels.show();
  Serial.println("Leds started");
}

void setupFS()
{
  SPIFFS.begin();
   {
     Dir dir = SPIFFS.openDir("/");
     while (dir.next()) {
       String fileName = dir.fileName();
       size_t fileSize = dir.fileSize();
       Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
     }
     Serial.printf("\n");
   }
}

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
  Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
  Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
  Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
  Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
  Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
  Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
  Serial.println();

//  fsv = new Str1pper::CScriptVariable<float>(42.0f);
  m = new Str1pper::CModul();

  modulTime = m->GetVariablePtr<float>(std::string("Timer"));

  setupFS();
  setupLeds();
  setupWifi();
  setupWebCommuncation();
}

void loop(void)
{
  switch(currentModul)
  {
    case LEDModul::PLASMA:
      plasma();
    break;
  }

timer += 0.016;
//delay(16);
//*modulTime = timer;
m->SetVariable("Timer", (float)timer);

 pixels.show();



//float& asd = *(static_cast<float*>(fsv->GetValuePtr()));

//asd += 1.0f;
 Serial.println(*modulTime);
 delay(50);
}
